#include "registervaluechange.h"

RegisterValueChange::RegisterValueChange(int monNum, QWidget *parent) : QWidget(), monitorNumber(monNum), helperWindow(nullptr)
{
    parentObj = parent;
    createWidget();
    connection = ConnectionManager::GetConnection();
    connect(this, SIGNAL(sendRequest(CmdHolder)), connection, SLOT(SendRequest(CmdHolder)));
}

void RegisterValueChange::createWidget()
{
    setWindowTitle("Регистры Монитора № "+ QString::number(monitorNumber));

    QLabel *lbRegBlock = new QLabel("Блок регистров:", this);
    QLabel *lbRegNumber = new QLabel("Номер регистра в блоке:", this);
    QLabel *lbRegValue = new QLabel("Значение регистра:", this);
    QLabel *lbRegValueBinary = new QLabel("В двоичном формате:", this);
    cbRegBlock = new QComboBox(this);
    connect(cbRegBlock, SIGNAL(currentIndexChanged(int)), SLOT(regBlockChanged(int)));
    sbRegNumber = new QSpinBox(this);
    connect(sbRegNumber, SIGNAL(valueChanged(int)), SLOT(regNumberChanged(int)));
    cbRegBlock->addItem("Регистры настройки узла сети");
    cbRegBlock->addItem("Регистры состояния счетчиков");
    cbRegBlock->addItem("Регистры монитора");
    cbRegBlock->addItem("Регистры доступа к внутренней сети");
    cbRegBlock->addItem("Регистры состояния модулей монитора");
    lRegValue = new QLineEdit(this);
    lRegValueBinary = new QLineEdit(this);
    lRegValueBinary->setReadOnly(true);
    connect(lRegValue, SIGNAL(textChanged(QString)), SLOT(regValueChanged(QString)));
    cb2regs = new QCheckBox("Значение хранится в 2-х регистрах", this);
    cbFloat = new QCheckBox("Вещественное число", this);
    cbFloat->setEnabled(false);
    QPushButton *btnRead = new QPushButton("Чтение", this);
    connect(btnRead, SIGNAL(clicked(bool)), SLOT(readValueButton()));
    QPushButton *btnWrite = new QPushButton("Запись", this);
    connect(btnWrite, SIGNAL(clicked(bool)), SLOT(writeValueButton()));
    QPushButton *btnHelp = new QPushButton("Справка...", this);
    connect(btnHelp, SIGNAL(clicked(bool)), SLOT(openRegHelperWindow()));

    QFormLayout *layout = new QFormLayout();
    layout->addRow(lbRegBlock, cbRegBlock);
    layout->addRow(lbRegNumber, sbRegNumber);
    layout->addRow(lbRegValue, lRegValue);
    layout->addRow(lbRegValueBinary,lRegValueBinary);
    layout->addRow(cb2regs, cbFloat);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(btnRead, 1);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(btnWrite, 1);
    buttonsLayout->addStretch(5);
    buttonsLayout->addWidget(btnHelp, 1);

    layout->addRow(buttonsLayout);
    this->setLayout(layout);
}

QString RegisterValueChange::getExeptionDescription(quint8 err)
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
    case 128://данный код ошибки не принадлежит протоколу ModBus
        ret = "Timeout запроса";
        break;
    default:
        ret = "Неизвестная ошибка";
    }

    return ret;
}

void RegisterValueChange::setRegBlockAndRegNumber(int regNumber)
{
    if(regNumber < (quint16)CmdHolder::CountersStatus)
    {
        cbRegBlock->setCurrentIndex(0);//выставление блока регистров
        sbRegNumber->setValue(regNumber - (quint16)CmdHolder::ConSettings);//выставление номера регистра
    }else
        if(regNumber < (quint16)CmdHolder::MonitorRegs)
        {
            cbRegBlock->setCurrentIndex(1);
            sbRegNumber->setValue(regNumber - (quint16)CmdHolder::CountersStatus);
        }else
            if(regNumber < (quint16)CmdHolder::InnerModbusNetwork)
            {
                cbRegBlock->setCurrentIndex(2);
                sbRegNumber->setValue(regNumber - (quint16)CmdHolder::MonitorRegs);
            }else
                if(regNumber < (quint16)CmdHolder::ModulesStatus)
                {
                    cbRegBlock->setCurrentIndex(3);
                    sbRegNumber->setValue(regNumber - (quint16)CmdHolder::InnerModbusNetwork);
                }else
                    if(regNumber >= (quint16)CmdHolder::ModulesStatus)
                    {
                        cbRegBlock->setCurrentIndex(4);
                        sbRegNumber->setValue(regNumber - (quint16)CmdHolder::ModulesStatus);
                    }
}

void RegisterValueChange::registerValueRecieved(CmdHolder &ch)
{
    if(ch.getFunc() & 0x80)//ошибка
    {
        QDataStream in(ch.getData());
        quint8 err;
        in >> err;
        QString funcDesc = (((quint8)ch.getFunc() - 0x80) == CmdHolder::ReadHoldingRegisters) ? "чтении" : "записи";
        QMessageBox::warning(this, "Ошибка при " + funcDesc, getExeptionDescription(err));
        return;
    }

    if(ch.getFunc() == CmdHolder::PresetMultipleRegisters)
    {//если пришел нормальный ответ на запись
        lRegValue->clear();//очистка поля значения регистра
        return;
    }
    //если пришел ответ на запрос чтения регистра
    //выставление блока и номера регистра
    setRegBlockAndRegNumber(ch.getStartReg());
    //считывание значения регистра
    QDataStream in(ch.getData());

    if(ch.getRegCount() == 2)
    {//значение в двух регистрах
        cb2regs->setChecked(true);

        QList<uint> floatRegs;//регистры, хранящие вещественные числа
        floatRegs << 106 << 108 << 113 << 115 << 156;

        if(floatRegs.contains(ch.getStartReg()))//если регистры хранят вещественное число
        {
            cbFloat->setChecked(true);
            in.setFloatingPointPrecision(QDataStream::SinglePrecision);//чтобы считывалось 4 байта float
            float value = 0;
            in >> value;
            CmdHolder::SwapFloat(&value);//смена местами 16-битных слов
            lRegValue->setText(QString::number(value));
            return;
        }

        cbFloat->setChecked(false);
        quint32 value = 0;
        in >> value;
        CmdHolder::SwapUInt32(value);//смена местами 16-битных слов
        lRegValue->setText(QString::number(value));
    }else
    {//значение в одном регистре
        cb2regs->setChecked(false);
        cbFloat->setChecked(false);
        quint16 value = 0;
        in >> value;
        lRegValue->setText(QString::number(value));
    }
}

void RegisterValueChange::registerSelectionChanged(int regNum, bool TwoBytes)
{
   //выставление блока и номера регистра
    setRegBlockAndRegNumber(regNum);

    if(TwoBytes)
        cb2regs->setChecked(true);
    else
        cb2regs->setChecked(false);

    readValueButton();//запрос на чтение значения выбранного регистра
}

void RegisterValueChange::readValueButton()
{
    lRegValue->clear();

    CmdHolder cmd(monitorNumber);

    int regBaseAdrr = getBlockAddress();//определение базового адреса блока

    cmd.setStartReg(regBaseAdrr + sbRegNumber->value());
    //определение количества регистров
    if(cb2regs->isChecked())
    {
        cmd.setRegCount(2);
    }
    else
    {
        cmd.setRegCount(1);
    }

    emit sendRequest(cmd);
}

int RegisterValueChange::getBlockAddress()
{
    int regBaseAdrr = 0;
    //определение базового адреса блока
    switch(cbRegBlock->currentIndex())
    {
    case 0:
        regBaseAdrr = (int)CmdHolder::ConSettings;
        break;
    case 1:
        regBaseAdrr = (int)CmdHolder::CountersStatus;
        break;
    case 2:
        regBaseAdrr = (int)CmdHolder::MonitorRegs;
        break;
    case 3:
        regBaseAdrr = (int)CmdHolder::InnerModbusNetwork;
        break;
    case 4:
        regBaseAdrr = (int)CmdHolder::ModulesStatus;
        break;
    }

    return regBaseAdrr;
}

void RegisterValueChange::writeValueButton()
{
    bool ok = false;
    quint32 value = lRegValue->text().toUInt(&ok);

    if(!ok)
    {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось считать положительное десятичное число. Проверьте ввод.");
        return;
    }
    CmdHolder cmd(monitorNumber, CmdHolder::PresetMultipleRegisters);

    int regBaseAdrr = getBlockAddress();//определение базового адреса блока

    cmd.setStartReg(regBaseAdrr + sbRegNumber->value());
    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);

    if(cb2regs->isChecked())
    {
        cmd.setRegCount(2);
        in << value;

    }else
    {
        cmd.setRegCount(1);
        in << (quint16) value;
    }
    cmd.setData(data);

    emit sendRequest(cmd);
}

void RegisterValueChange::openRegHelperWindow()
{
    if(!helperWindow)
    {
        helperWindow = new RegisterBlockHelper();
        connect(this, SIGNAL(regSelectionChanged(int)), helperWindow, SLOT(changedSelectedRegister(int)));
        connect(helperWindow, SIGNAL(registerDoubleClicked(int, bool)), this, SLOT(registerSelectionChanged(int, bool)));
    }
    helperWindow->showNormal();
    helperWindow->activateWindow();//вывод окна на передний план

    regNumberChanged(sbRegNumber->value());
}

void RegisterValueChange::regBlockChanged(int curInd)
{
    //выставление диапазонов для разных блоков
    switch(curInd)
    {
    case 0:
        sbRegNumber->setRange(0, 1);
        break;
    case 1:
        sbRegNumber->setRange(0, 10);
        break;
    case 2:
        sbRegNumber->setRange(0, 63);
        break;
    case 3:
        sbRegNumber->setRange(0, 35);
        break;
    case 4:
        sbRegNumber->setRange(0, 255);
        break;
    }
    //отправка сигнала справке о смене текущего выбранного регистра
    regNumberChanged(sbRegNumber->value());
}

void RegisterValueChange::regValueChanged(QString str)
{
    bool ok = false;
    quint32 value = str.toUInt(&ok);

    if(!ok || (lRegValue->text() == ""))
    {
        lRegValueBinary->clear();
        return;
    }

    lRegValueBinary->setText(QString::number(value, 2));//вывод бинарного представления числа
}

void RegisterValueChange::regNumberChanged(int regNum)
{
    int regBaseAdrr = getBlockAddress();//определение базового адреса блока

    emit regSelectionChanged(regNum + regBaseAdrr);
}

void RegisterValueChange::closeEvent(QCloseEvent *)
{
    if(helperWindow)
    {
        helperWindow->deleteLater();
        helperWindow = 0;
    }
    disconnect(parentObj, 0, this, SLOT(registerValueRecieved(CmdHolder&)));
}

