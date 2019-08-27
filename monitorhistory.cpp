#include "monitorhistory.h"

MonitorHistory::MonitorHistory(int monNum, QWidget *parent) : QWidget(parent),
    monitorNumber(monNum),connection(ConnectionManager::GetConnection())
{
    createWidget();

    connect(this, SIGNAL(sendRequest(CmdHolder)), connection, SLOT(SendRequest(CmdHolder)));
    ResetArchive();
}

void MonitorHistory::readEventFromArchive(CmdHolder &ch)
{
    if(ch.getFunc() & 0x80)//ошибка
    {
        sendRequestForEvent();
    }else
    {
        //обработка сообщения
        QByteArray dataArray = ch.getData();
        QDataStream data(dataArray);
        data.setFloatingPointPrecision(QDataStream::SinglePrecision);//чтобы считывалось 4 байта float
        QTableWidgetItem *item = nullptr;
        int currentRow = tableHistory->rowCount();
        tableHistory->insertRow(currentRow);

        //время
        quint32 time = 0;
        data >> time;
        CmdHolder::SwapUInt32(time);
        QDateTime dateTime(QDate(1970, 1, 1), QTime(0, 0));//отсчет времени в приборе ведется от этой даты (в секундах)
        dateTime = dateTime.addSecs(time);
        item = new QTableWidgetItem(dateTime.toString("dd.MM.yyyy hh:mm:ss"));
        item->setData(0, dateTime);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 0, item);

        //событие
        quint16 event = 0;
        data >> event;
        item = new QTableWidgetItem(getArchiveEventDescription(event));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 1, item);

        //канал
        item = new QTableWidgetItem(getArchiveChannelDescription(event));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 2, item);

        //счёт
        quint32 count = 0;
        data >> count;
        CmdHolder::SwapUInt32(count);
        item = new QTableWidgetItem(QString::number(count));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 3, item);

        //Фон
        float fon = 0;
        data >> fon;
        CmdHolder::SwapFloat(&fon);
        item = new QTableWidgetItem(QString::number(fon, 'f', 2));//вывод 2-х знаков после запятой
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 4, item);

        //номер объекта
        quint16 obj = 0;
        data >> obj;
        item = new QTableWidgetItem(QString::number(obj));

        //добавление строки в таблицу
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableHistory->setItem(currentRow, 5, item);

        //регистр управления
        qint16 c = 0;
        data >> c;//пропускаем один регистр (дополнительная информация об объекте)
        data >> c;//считывание регистра управления

        if(c != -1)
            //если регистр управления не содержит значение "-1", то отправляем запрос на следующую запись архива
            sendRequestForEvent();
        else
        {//все записи архива загружены
            tableHistory->resizeColumnsToContents();
            //вывод подтверждения загрузки архива
            QMessageBox::information(this, this->windowTitle(), "Записи архива успешно загружены.");
        }
    }
}

void MonitorHistory::saveArchiveToFile()
{
    int colCount = tableHistory->columnCount();
    int rowCount = tableHistory->rowCount();

    if(!rowCount)
    {
        QMessageBox::information(this, "Ошибка", "Нет записей для сохранения.");
        return;
    }

    QString defaultName = "/Архив Монитора " + QString::number(monitorNumber) +
            " [" +QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "]";
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить", QDir::homePath() + defaultName);
    if (fileName.isEmpty())//не выбран файл
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream out(&file);
    QString splitter = ";";

    //вывод названия столбцов
    QString rowNum = "Строка №";
    out << rowNum;
    out << splitter;
    for(int i = 0; i < colCount; ++i)
    {
        out << tableHistory->horizontalHeaderItem(i)->text();//вывод заголовка столбца
        if(i != colCount - 1)//запрет вывода разделителя в конце строки
            out << splitter;
    }
    out << '\n';

    //вывод строк
    for(int row = 0; row < rowCount; ++row)
    {
        out << QString::number(row + 1);//вывод номера столбца
        out << splitter;
        for(int col = 0; col < colCount; ++col)
        {
            if(!col){
                out << tableHistory->item(row, col)->data(0).toDateTime().toString("dd.MM.yyyy hh:mm:ss");//вывод времени из таблицы
            }else{
                out << tableHistory->item(row, col)->text();//вывод ячейки из таблицы
            }
            if(col != colCount - 1)
                out << splitter;
        }

        if(row != rowCount - 1)//чтобы не выводилась пустая строка в конце файла
            out << "\n";
    }
    file.close();//закрытие файла
}

void MonitorHistory::createWidget()
{
    setWindowTitle("Архив Монитора № "+ QString::number(monitorNumber));
    this->setMinimumHeight(200);
    this->setMinimumWidth(700);

    tableHistory = new QTableWidget(this);
    tableHistory->setSortingEnabled(false);//выключение сортировки по колонкам
    tableHistory->setSelectionBehavior(QAbstractItemView::SelectItems);
    tableHistory->clear();
    //выставление названий столбцов
    tableHistory->setColumnCount(6);
    QStringList colHead;
    colHead<< "Время" << "Событие" << "Канал" << "Счёт" << "Фон" << "Объект №";
    tableHistory->setHorizontalHeaderLabels(colHead);

    QPushButton *saveBtn = new QPushButton("Сохранить...", this);
    connect(saveBtn, SIGNAL(clicked(bool)), this, SLOT(saveArchiveToFile()));
    QPushButton *resetBtn = new QPushButton("Перезагрузить", this);
    connect(resetBtn, SIGNAL(clicked(bool)), this, SLOT(ResetArchive()));

    QHBoxLayout *layBtns = new QHBoxLayout();
    layBtns->addWidget(resetBtn, 0, Qt::AlignLeft);
    layBtns->addStretch(1);
    layBtns->addWidget(saveBtn, 0, Qt::AlignRight);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tableHistory);
    layout->addLayout(layBtns);
    this->setLayout(layout);
}

void MonitorHistory::sendResetCmd()
{
    //формирование запроса
    CmdHolder cmd(monitorNumber, CmdHolder::PresetMultipleRegisters);
    cmd.setRegCount(1);
    cmd.setStartReg(CmdHolder::MonitorRegs + 60);
    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);
    in << (quint16)0;
    cmd.setData(data);

    //отправка запроса
    emit sendRequest(cmd);
}

void MonitorHistory::ResetArchive()
{
    //удаляем предыдущие данные
    tableHistory->clearContents();
    tableHistory->setRowCount(0);

    //при обнулении регистра управления (60) монитор отображает в регистры 51-60 первую запись архива
    sendResetCmd();
    //отправлям запрос на чтение регистров архива (51-60)
    sendRequestForEvent();
}

void MonitorHistory::sendRequestForEvent()
{
    CmdHolder cmd(monitorNumber);
    cmd.setRegCount(10);
    cmd.setStartReg(CmdHolder::MonitorRegs + 51);

    emit sendRequest(cmd);
}

QString MonitorHistory::getArchiveEventDescription(const quint16 reg)
{
    QString desc;
    //значение младшего байта - информация о событии
    const quint16 byte = reg & 0x00FF;

    switch(byte)
    {
    case 0:
        desc = "Включен";
        break;
    case 1:
        desc = "Выключен";
        break;
    case 2:
        desc = "Отбой";
        break;
    case 3:
        desc = "Тревога";
        break;
    case 6:
        desc = "Вскрыт";
        break;
    case 7:
        desc = "Закрыт";
        break;
    case 8:
        desc = "Батарея разряжена";
        break;
    case 9:
        desc = "Батарея в норме";
        break;
    case 10:
        desc = "Сеть выключена";
        break;
    case 11:
        desc = "Сеть включена";
        break;
    case 12:
        desc = "Блокировка сигнализации";
        break;
    case 13:
        desc = "Отмена блокировки сигнализации";
        break;
    case 14:
        desc = "Включение сигнализации";
        break;
    case 15:
        desc = "Отмена включения сигнализации";
        break;
    case 16:
        desc = "Часы сброшены";
        break;
    case 17:
        desc = "Архив очищен";
        break;
    case 18:
        desc = "Установка параметров";
        break;
    case 19:
        desc = "Установка времени";
        break;
    case 20:
        desc = "Новое время";
        break;
    case 21:
        desc = "Перегруз";
        break;
    case 22:
        desc = "Максимум";
        break;
    case 23:
        desc = "Минимум";
        break;
    case 24:
        desc = "Отказ счетчика";
        break;
    case 25:
        desc = "Состояние";
        break;
    default:
        desc = "Неопределенное событие";
    }

    return desc;
}

QString MonitorHistory::getArchiveChannelDescription(const quint16 reg)
{
    QString desc;
    //значение старшего байта - код канала
    const quint16 byte = (reg & 0xFF00) >> 8;

    switch(byte)
    {
    case 0:
        desc = "Гамма канал";
        break;
    case 1:
        desc = "Нейтронный канал";
        break;
    default:
        desc = "Неопределенный канал";
    }

    return desc;
}

