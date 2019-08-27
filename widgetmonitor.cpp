#include "widgetmonitor.h"

void WidgetMonitor::requestForMeasureRanges()
{
    CmdHolder cmd(monitorNumber);
    cmd.setStartReg(CmdHolder::MonitorRegs + 26);
    cmd.setRegCount(12);
    emit sendRequest(cmd);
}

WidgetMonitor::WidgetMonitor(int num, QWidget *parent) : QFrame(parent), monitorNumber(num)
{
    createWidget();
    setMonitorViewMode(false);//выставление графического режима по-умолчанию

    monHistoryWindow = 0;//архив монитора
    regValueChangeWindow = 0;//регистры монитора

    //создание запроса о состоянии счетчиков
    cmdMonStatus.setDeviceAdress(monitorNumber);
    cmdMonStatus.setStartReg(CmdHolder::MonitorRegs);
    cmdMonStatus.setRegCount(17);

    connection = ConnectionManager::GetConnection();//соединение с мониторами
    connect(this, SIGNAL(sendRequest(CmdHolder)), connection, SLOT(SendRequest(CmdHolder)));

    //запуск отправки запросов монитору
    cmdSendTimer = new QTimer(this);
    //считывание текущего интервала обновления
    QSettings conSettings;
    cmdSendTimer->setInterval(conSettings.value("/updateinterval", 1000).toInt());
    connect(cmdSendTimer, SIGNAL(timeout()), SLOT(sendCommands()));
    cmdSendTimer->start();

    //таймер сброса последней ошибки
    errorTimer = new QTimer(this);
    errorTimer->setSingleShot(true);//выполняется единожды
    errorTimer->setInterval(10000);//время сброса ошибки
    connect(errorTimer, SIGNAL(timeout()), SLOT(ResetErrorIndicator()));
}

WidgetMonitor::~WidgetMonitor()
{
    //закрытие открытых окон
    if(monHistoryWindow)
    {
        monHistoryWindow->close();
        delete monHistoryWindow;
    }

    if(regValueChangeWindow)
    {
        regValueChangeWindow->close();
        delete regValueChangeWindow;
    }
}

void WidgetMonitor::setMonitoringStatus(bool enabled)
{
    static QPalette color;

    if(enabled)
    {
        color.setColor(QPalette::Text,Qt::darkGreen);

        labelMonitoringStatus->setPalette(color);
        labelMonitoringStatus->setToolTip("Мониторинг включен");
        labelMonitoringStatus->setStatusTip("Мониторинг включен");
    }else
    {
        color.setColor(QPalette::Text,Qt::darkRed);

        labelMonitoringStatus->setPalette(color);
        labelMonitoringStatus->setToolTip("Мониторинг выключен");
        labelMonitoringStatus->setStatusTip("Мониторинг выключен");
    }
}

void WidgetMonitor::switchMonitoring()
{
    if(cmdSendTimer->isActive())
    {
        cmdSendTimer->stop();

        setMonitoringStatus(false);
    }else
    {
        cmdSendTimer->start();

        setMonitoringStatus(true);
    }
}

void WidgetMonitor::changeRegValue()
{
    if(!regValueChangeWindow)//если окно не было создано
    {
        regValueChangeWindow = new RegisterValueChange(monitorNumber, this);
    }
    connect(this, SIGNAL(registerValueRecieved(CmdHolder &)), regValueChangeWindow, SLOT(registerValueRecieved(CmdHolder &)));//подключение сигнала о полученном значении регистра
    //показ окна в месте по умолчанию и вывод на первый план
    regValueChangeWindow->showNormal();
    regValueChangeWindow->activateWindow();
}

void WidgetMonitor::ProcessResponse(CmdHolder &ch)
{
    //получено состояние монитора
    if((ch.getStartReg() == (CmdHolder::MonitorRegs)) && (ch.getRegCount() == 17))
    {
        if(ch.getFunc() & 0x80)//ошибка
        {
            QDataStream in(ch.getData());
            quint8 err;
            in >> err;
            setErrorIndicator(err);
            //добавление пустой точки на графики
            if(!gbGGraph->isHidden()) graphicG->AddValues(0, 0, 0, 0);
            if(!gbNGraph->isHidden()) graphicN->AddValues(0, 0, 0, 0);
            return;
        }
        //значения для графиков
        float fonN, fonG;
        quint32 valueN, valueG, time;
        quint16 levelN, levelG;

        //выставление состояния монитора
        QDataStream in(ch.getData());
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);//чтобы считывалось 4 байта для float
        quint16 val16;
        quint32 val32;
        float valFloat;

        //время монитора
        in >> val32;
        CmdHolder::SwapUInt32(val32);//смена местами 16-битных слов
        time = val32;
        //состояние входных сигналов
        in >> val16;
        changeStatus(val16);

        //состояние тревоги гамма канала
        in >> val16;
        levelG = val16;
        changeWarningLevel(labelLevelG, val16);
        //счет
        in >> val32;
        CmdHolder::SwapUInt32(val32);
        valueG = val32;
        progressG->setValue(val32);
        //фон
        in >> valFloat;
        CmdHolder::SwapFloat(&valFloat);
        fonG = valFloat;
        labelFonG->setText("Фон: " + QString::number(valFloat, 'f', 2));
        if(!gbGGraph->isHidden()) graphicG->AddValues(time, valueG, fonG, levelG);

        //состояние тревоги нейтронного канала
        in.device()->seek(20);
        in >> val16;
        levelN = val16;
        changeWarningLevel(labelLevelN, val16);
        //счет
        in >> val32;
        CmdHolder::SwapUInt32(val32);
        valueN = val32;
        progressN->setValue(val32);
        //фон
        in >> valFloat;
        CmdHolder::SwapFloat(&valFloat);
        fonN = valFloat;
        labelFonN->setText("Фон: " + QString::number(valFloat, 'f', 2));
        if(!gbNGraph->isHidden()) graphicN->AddValues(time, valueN, fonN, levelN);

        //запуск метода внешней библиотеки при тревоге для дополнительных действий
//        if((levelG & 0x0003) || (levelN & 0x0003))
//        {
//            YantarMonitorAlertAction act;
//            act.Action(time, fonG, fonN, valueG, valueN, levelG, levelN);
//        }
    }
    //получена запись из архива монитора
    else if((ch.getStartReg() == (CmdHolder::MonitorRegs + 51)) && (ch.getRegCount() == 10))
    {
        emit archiveEventRecieved(ch);
    }
    //ответ на запрос чтения регистра
    else if((ch.getRegCount() == 1) || (ch.getRegCount() == 2))
    {
        emit registerValueRecieved(ch);
    }
    //ответ на запрос диапазонов отображения счета
    else if((ch.getStartReg() == (CmdHolder::MonitorRegs + 26)) && (ch.getRegCount() == 12))
    {
        if(ch.getFunc() & 0x80)//ошибка
        {
            QDataStream in(ch.getData());
            quint8 err;
            in >> err;
            setErrorIndicator(err);
            return;
        }

        //выставление диапазонов
        QDataStream in(ch.getData());
        in.device()->seek(2);
        quint16 maxVal;
        in >> maxVal;
        progressG->setMaximum(maxVal);

        in.device()->seek(20);
        in >> maxVal;
        progressN->setMaximum(maxVal);
    }
}

void WidgetMonitor::ChangeTimerInterval(int interval)
{
    cmdSendTimer->setInterval(interval);
}

void WidgetMonitor::openMonitorHistory()
{
    if(!monHistoryWindow)//если окно не было создано
    {
        monHistoryWindow = new MonitorHistory(monitorNumber);
        connect(this, SIGNAL(archiveEventRecieved(CmdHolder &)), monHistoryWindow, SLOT(readEventFromArchive(CmdHolder &)));//подключение сигнала о полученном событии архива
    }

    monHistoryWindow->showNormal();//показ окна или возвращение его к изначальному размеру
    monHistoryWindow->activateWindow();//вывод окна на первый план
}

void WidgetMonitor::setMonitorViewMode(bool textMode)
{
    if(textMode)
    {//выставление текстового режима
        requestForMeasureRanges();

        labelTextStatus->show();
        gbG->show();
        gbN->show();

        gbGraphStatus->hide();
        gbGGraph->hide();
        gbNGraph->hide();

        graphicG->Clear();
        graphicN->Clear();
    }else
    {//выставление графического режима
        labelTextStatus->hide();
        gbG->hide();
        gbN->hide();

        gbGraphStatus->show();
        gbGGraph->show();
        gbNGraph->show();
    }
}

void WidgetMonitor::changeGraphSize()
{
    if(gbGGraph->isHidden()) return;//если графики скрыты - ничего не делать

    if(gbGGraph->width() < 300)
    {//увеличение размера графиков
        gbGGraph->setFixedSize(gbGGraph->width() * 2, gbGGraph->height() * 2);
        gbNGraph->setFixedSize(gbNGraph->width() * 2, gbNGraph->height() * 2);
        //увеличение количества отображаемых значений на графике
        graphicG->setValuesCount(50);
        graphicN->setValuesCount(50);
    }else
    {//уменьшение размера графиков
        gbGGraph->setFixedSize(gbGGraph->width() / 2, gbGGraph->height() / 2);
        gbNGraph->setFixedSize(gbNGraph->width() / 2, gbNGraph->height() / 2);
        //уменьшение количества отображаемых значений на графике
        graphicG->setValuesCount(30);
        graphicN->setValuesCount(30);
    }
}

void WidgetMonitor::switchViewMode()
{
    if(gbGGraph->isHidden())
        setMonitorViewMode(false);
    else
        setMonitorViewMode(true);
}

void WidgetMonitor::sendCommands()
{
    //отправка запроса на считывание состояния счетчиков
    emit sendRequest(cmdMonStatus);
}

void WidgetMonitor::ResetErrorIndicator()
{
    static QPalette gray;
    gray.setColor(QPalette::Text,Qt::gray);

    labelErrorIndicator->setPalette(gray);
    labelErrorIndicator->setToolTip("Ошибок нет");
    labelErrorIndicator->setStatusTip("Ошибок нет");
}

void WidgetMonitor::createWidget()
{
    //элементы текстового отображения
    QLabel *labelTitle = new QLabel("Монитор №");
    QLabel *labelMonNumber = new QLabel(QString::number(monitorNumber));
    labelTextStatus = new QLabel("Статус монитора");
    labelTextStatus->setWordWrap(true);
    labelTextStatus->setFixedWidth(180);
    labelTextStatus->setMinimumHeight(120);
    labelLevelN = new QLabel("Состояние тревоги: ");
    labelLevelN->setAutoFillBackground(true);
    labelLevelG = new QLabel("Состояние тревоги: ");
    labelLevelG->setAutoFillBackground(true);
    labelFonN = new QLabel("Фон:");
    labelFonG = new QLabel("Фон:");
    progressG = new QProgressBar();
    progressG->setFormat("%v");
    progressG->setFixedHeight(16);
    progressG->setMaximum(200000);
    progressN = new QProgressBar();
    progressN->setFormat("%v");
    progressN->setFixedHeight(16);
    progressN->setMaximum(200000);
    labelErrorIndicator = new QLabel("ERR");
    labelErrorIndicator->setToolTip("Ошибок нет");
    labelErrorIndicator->setStatusTip("Ошибок нет");
    labelMonitoringStatus = new QLabel("M");
    labelMonitoringStatus->setToolTip("Мониторинг включен");
    labelMonitoringStatus->setStatusTip("Мониторинг включен");
    QPalette green, gray;
    green.setColor(QPalette::Text,Qt::darkGreen);
    gray.setColor(QPalette::Text,Qt::gray);
    labelErrorIndicator->setPalette(gray);
    labelMonitoringStatus->setPalette(green);

    //элементы графического отображения
    labelKU = new QLabel("КУ");//Нажата кнопка управления
    labelKU->setToolTip("Нажата кнопка управления");
    labelKU->setStatusTip("Нажата кнопка управления");
    labelNSP = new QLabel("НСП");//Нет сетевого питания
    labelNSP->setToolTip("Нет сетевого питания");
    labelNSP->setStatusTip("Нет сетевого питания");
    labelNNB = new QLabel("ННБ");//Низкое напряжение батареи
    labelNNB->setToolTip("Низкое напряжение батареи");
    labelNNB->setStatusTip("Низкое напряжение батареи");
    labelDV = new QLabel("ДВ");//Датчик вскрытия
    labelDV->setToolTip("Датчик вскрытия");
    labelDV->setStatusTip("Датчик вскрытия");
    labelOU = new QLabel("ОУ");//Отказ в устройстве
    labelOU->setToolTip("Отказ в устройстве");
    labelOU->setStatusTip("Отказ в устройстве");
    labelNSVP = new QLabel("НСВП");//Устройство не смогло восстановить параметры при старте
    labelNSVP->setToolTip("Устройство не смогло восстановить параметры при старте");
    labelNSVP->setStatusTip("Устройство не смогло восстановить параметры при старте");

    graphicG = new Graphic(180, 100, this);//график гамма
    graphicN = new Graphic(180, 100, this);//график нейтронов

    //общий лэйаут
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(2);
    layout->setSpacing(2);

    //название монитора
    QHBoxLayout *layoutTitle = new QHBoxLayout;
    layoutTitle->addWidget(labelMonitoringStatus);
    layoutTitle->addStretch(1);
    layoutTitle->addWidget(labelTitle);
    layoutTitle->addWidget(labelMonNumber);
    layoutTitle->addStretch(1);
    layoutTitle->addWidget(labelErrorIndicator);

    layout->addLayout(layoutTitle, Qt::AlignTop);

    //статус монитора
    layout->addWidget(labelTextStatus, Qt::AlignVCenter);

    //гамма, текстовый режим
    gbG = new QGroupBox("Гамма канал");
    gbG->setFixedHeight(80);
    QVBoxLayout *layG = new QVBoxLayout;
    layG->setSpacing(0);
    layG->setMargin(0);
    layG->addWidget(labelLevelG, Qt::AlignLeft);
    layG->addWidget(progressG, Qt::AlignRight);
    layG->addWidget(labelFonG, Qt::AlignLeft);
    gbG->setLayout(layG);
    layout->addWidget(gbG, Qt::AlignBottom);

    //нейтроны, текстовый режим
    gbN = new QGroupBox("Нейтронный канал");
    gbN->setFixedHeight(80);
    QVBoxLayout *layN = new QVBoxLayout;
    layN->setSpacing(0);
    layN->setMargin(0);
    layN->addWidget(labelLevelN, Qt::AlignLeft);
    layN->addWidget(progressN, Qt::AlignRight);
    layN->addWidget(labelFonN, Qt::AlignLeft);
    gbN->setLayout(layN);
    layout->addWidget(gbN, Qt::AlignBottom);

    //символьное отображение состояние монитора
    gbGraphStatus = new QGroupBox("Состояние");
    gbGraphStatus->setMinimumWidth(180);
    QHBoxLayout *layGraphStatus = new QHBoxLayout;
    layGraphStatus->setSpacing(0);
    layGraphStatus->setMargin(0);
    layGraphStatus->addWidget(labelKU);
    layGraphStatus->addStretch(1);
    layGraphStatus->addWidget(labelNSP);
    layGraphStatus->addStretch(1);
    layGraphStatus->addWidget(labelNNB);
    layGraphStatus->addStretch(1);
    layGraphStatus->addWidget(labelDV);
    layGraphStatus->addStretch(1);
    layGraphStatus->addWidget(labelOU);
    layGraphStatus->addStretch(1);
    layGraphStatus->addWidget(labelNSVP);
    gbGraphStatus->setLayout(layGraphStatus);
    layout->addWidget(gbGraphStatus, Qt::AlignTop);
    //график гамма
    gbGGraph = new QGroupBox("Гамма канал");
    gbGGraph->setFlat(true);
    gbGGraph->setFixedSize(220, 120);
    QVBoxLayout *layGraphG = new QVBoxLayout;
    layGraphG->setSpacing(0);
    layGraphG->setMargin(0);
    layGraphG->addWidget(graphicG);
    gbGGraph->setLayout(layGraphG);
    layout->addWidget(gbGGraph, Qt::AlignBottom);
    //график нейтронов
    gbNGraph = new QGroupBox("Нейтронный канал");
    gbNGraph->setFlat(true);
    gbNGraph->setFixedSize(220, 120);
    QVBoxLayout *layGraphN = new QVBoxLayout;
    layGraphN->setSpacing(0);
    layGraphN->setMargin(0);
    layGraphN->addWidget(graphicN);
    gbNGraph->setLayout(layGraphN);
    layout->addWidget(gbNGraph, Qt::AlignBottom);

    //параметры рамки вокруг монитора
    setFrameShadow(QFrame::Sunken);
    setFrameShape(QFrame::StyledPanel);
    setLineWidth(3);
    setMidLineWidth(3);

    setLayout(layout);
}

void WidgetMonitor::changeStatus(const quint16 reg)
{
    if(!labelTextStatus->isHidden())
    {//текстовое отбражение статуса
        QString stat;

        if(reg & 0x0010) stat += "Нажата кнопка управления;";
        if(reg & 0x0100) stat += "\nНет сетевого питания;";
        if(reg & 0x0200) stat += "\nНизкое напряжение батареи;";
        if(reg & 0x0400) stat += "\nДатчик вскрытия;";
        if(reg & 0x0800) stat += "\nОтказ в устройстве;";
        if(reg & 0x8000) stat += "\nУстройство не смогло восстановить параметры при старте;";

        if(stat.isEmpty())
            stat = "Устройство исправно;";

        labelTextStatus->setText(stat.trimmed());
    }else
        if(!gbGraphStatus->isHidden())
    {//графический режим
        static QPalette gray, red;
        gray.setColor(QPalette::Text,Qt::gray);
        red.setColor(QPalette::Text,Qt::darkRed);

        if(reg & 0x0010) labelKU->setPalette(red); else labelKU->setPalette(gray);
        if(reg & 0x0100) labelNSP->setPalette(red); else labelNSP->setPalette(gray);
        if(reg & 0x0200) labelNNB->setPalette(red); else labelNNB->setPalette(gray);
        if(reg & 0x0400) labelDV->setPalette(red); else labelDV->setPalette(gray);
        if(reg & 0x0800) labelOU->setPalette(red); else labelOU->setPalette(gray);
        if(reg & 0x8000) labelNSVP->setPalette(red); else labelNSVP->setPalette(gray);
    }
}

void WidgetMonitor::changeWarningLevel(QLabel *labelLevel, quint16 reg)
{
    //выделение двух младших битов
    reg <<= 14;
    reg >>= 14;
    labelLevel->setText("Состояние тревоги: " + QString::number(reg));
    QPalette pal;
    //цвета уровней тревоги
    static QColor cl1(200, 0, 0, 50),//тревоги уровней 1-3
            cl2(200, 0, 0, 150),
            cl3(250, 0, 0, 200),
            cl0(0, 200, 0, 50);//нет тревоги

    switch(reg)
    {
    case 0:
        pal.setColor(labelLevel->backgroundRole(), cl0);
        labelLevel->setPalette(pal);
        break;
    case 1:
        pal.setColor(labelLevel->backgroundRole(), cl1);
        labelLevel->setPalette(pal);
        break;
    case 2:
        pal.setColor(labelLevel->backgroundRole(), cl2);
        labelLevel->setPalette(pal);
        break;
    case 3:
        pal.setColor(labelLevel->backgroundRole(), cl3);
        labelLevel->setPalette(pal);
        break;
    }
}

void WidgetMonitor::setErrorIndicator(quint8 err = 0)
{
    static QPalette red;
    red.setColor(QPalette::Text,Qt::darkRed);

    labelErrorIndicator->setPalette(red);
    labelErrorIndicator->setToolTip("Последняя ошибка: " + getErrorDescription(err));
    labelErrorIndicator->setStatusTip("Последняя ошибка: " + getErrorDescription(err));

    errorTimer->start();//таймер обнуления ошибки
}

QString WidgetMonitor::getErrorDescription(quint8 err)
{
    QString ret = "";

    switch(err)
    {
    case 1:
        ret = "Неправильный код функции";
        break;
    case 2:
        ret = "Неверные адреса регистров";
        break;
    case 3:
        ret = "Неверное количество регистров";
        break;
    case 4:
        ret = "Ошибка при выполнении функции";
        break;
    case 5:
        ret = "Выполнение функции занимает слишком много времени";
        break;
    case 6:
        ret = "Устройство занято: отказ выполнения функции";
        break;
    case 10:
        ret = "Gateway недоступен";
        break;
    case 11:
        ret = "Устройство не найдено";
        break;
    case 128://данный код ошибки не принадлежит протоколу ModBus
        ret = "Timeout запроса";
        break;
    default:
        ret = "Неизвестная ошибка " + QString::number(err);
    }

    return ret;
}
