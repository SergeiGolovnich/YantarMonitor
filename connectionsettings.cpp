#include "connectionsettings.h"

ConnectionSettings::ConnectionSettings(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    connection = ConnectionManager::GetConnection();
    connect(this, SIGNAL(connectionSettingsChanged()), connection, SLOT(reset()));

    createWindow();//создание элементов окна
    findCOMPorts();//поиск COM-портов
    readConSettings();//выставление сохраненных настроек
}

void ConnectionSettings::findCOMPorts()
{

    //поиск доступных портов
    QList<QSerialPortInfo> listInfo = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo pi, listInfo)
    {
        cbCOMPort->addItem(pi.portName());
    }

    //поиск доступных скоростей
    QList<qint32> listBaudRate = QSerialPortInfo::standardBaudRates();
    foreach (qint32 br, listBaudRate)
    {
        if(br <= 115200 && br >= 1200 && br != 1800)//ограниечение на поддерживаемые монитором скорости
            cbCOMBaudRate->addItem(QString::number(br));
    }

    //высталение доступных количества бит данных
    cbDataBits->addItem(QString::number((int)QSerialPort::Data5));
    cbDataBits->addItem(QString::number((int)QSerialPort::Data6));
    cbDataBits->addItem(QString::number((int)QSerialPort::Data7));
    cbDataBits->addItem(QString::number((int)QSerialPort::Data8));

    //добавление вариантов четность
    cbParity->addItem("NoParity");
    cbParity->addItem("EvenParity");
    cbParity->addItem("OddParity");
    cbParity->addItem("SpaceParity");
    cbParity->addItem("MarkParity");

    //добавление вариантов стоп битов
    cbStopBits->addItem("OneStop");
    cbStopBits->addItem("TwoStop");
    cbStopBits->addItem("OneAndHalfStop");

}

void ConnectionSettings::readConSettings()
{
    //TCP
    lIP->setText(connection->address.toString());
    sbPort->setValue(connection->port);
    sbTimeOut->setValue(connection->timeout);

    //метод соединения
    cbMethod->setCurrentIndex(connection->method);

    if(!cbCOMPort->count())
        return;//если нет доступных COM-портов

    //выставление настроек COM
    if(connection->serialPort.portName() != "")
    {
        cbCOMPort->setCurrentIndex(cbCOMPort->findText(connection->serialPort.portName()));
        cbCOMBaudRate->setCurrentIndex(cbCOMBaudRate->findText(QString::number(connection->serialPort.baudRate(QSerialPort::AllDirections))));
        cbDataBits->setCurrentIndex(cbDataBits->findText(QString::number(connection->serialPort.dataBits())));
        cbParity->setCurrentIndex(connection->serialPort.parity() == 0 ? 0 : connection->serialPort.parity() - 1);
        cbStopBits->setCurrentIndex(connection->serialPort.stopBits() - 1);
    }else
    {
        //значения по умолчанию
        cbCOMPort->setCurrentIndex(0);
        cbCOMBaudRate->setCurrentIndex(12);
        cbDataBits->setCurrentIndex(3);
        cbParity->setCurrentIndex(0);
        cbStopBits->setCurrentIndex(0);
    }
}

void ConnectionSettings::writeConSettings()
{
    QRegExp ipValid = QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");//проверка ввода ip адреса

    conSettings.beginGroup("/ConnectionSettings");

    //параметры COM-порта
    conSettings.setValue("/serialportname", cbCOMPort->currentText());
    conSettings.setValue("/baudrate", cbCOMBaudRate->currentText().toInt());
    conSettings.setValue("/databits", cbDataBits->currentText().toInt());
    conSettings.setValue("/parity", cbParity->currentIndex() == 0 ? 0 : cbParity->currentIndex() + 1);
    conSettings.setValue("/stopbits", cbStopBits->currentIndex() + 1);

    //TCP
    if(lIP->text().trimmed().contains(ipValid))
        conSettings.setValue("/ip", lIP->text().trimmed());
    else{
        QMessageBox::warning(this, "Ошибка сохранения настроек", "IP-адрес не сохранен. Неверный формат ввода.");
    }
    conSettings.setValue("/port", sbPort->value());
    conSettings.setValue("/timeout", sbTimeOut->value());
    //метод соединения
    conSettings.setValue("/method", cbMethod->currentIndex());

    conSettings.endGroup();
}

void ConnectionSettings::createWindow()
{
    setWindowTitle("Настройка соединения");
    //элементы COM порта
    QLabel *lbCOMPort = new QLabel("COM-порт:",this);
    cbCOMPort = new QComboBox(this);

    QLabel *lbCOMBaudRate = new QLabel("Скорость:",this);
    cbCOMBaudRate = new QComboBox(this);

    QLabel *lbDataBits = new QLabel("Биты данных:",this);
    cbDataBits = new QComboBox(this);

    QLabel *lbParity = new QLabel("Четность:",this);
    cbParity = new QComboBox(this);

    QLabel *lbStopBits = new QLabel("Стоп биты:",this);
    cbStopBits = new QComboBox(this);

    QGroupBox *gbCOM = new QGroupBox("Параметры COM-порта",this);
    QFormLayout *lCOM = new QFormLayout();
    lCOM->addRow(lbCOMPort,cbCOMPort);
    lCOM->addRow(lbCOMBaudRate,cbCOMBaudRate);
    lCOM->addRow(lbDataBits,cbDataBits);
    lCOM->addRow(lbParity,cbParity);
    lCOM->addRow(lbStopBits,cbStopBits);
    gbCOM->setLayout(lCOM);


    //элементы TCP
    QLabel *lbIP = new QLabel("IP адрес:",this);
    lIP = new QLineEdit(this);

    QLabel *lbPort = new QLabel("Порт:",this);
    sbPort = new QSpinBox(this);
    sbPort->setRange(0,999999);//возможный диапазон портов
    sbPort->setSingleStep(1);

    QLabel *lbTimeOut = new QLabel("Timeout (ms):",this);
    sbTimeOut = new QSpinBox(this);
    sbTimeOut->setRange(50, 70000);//диапазон таймаут
    sbTimeOut->setSingleStep(50);

    QGroupBox *gbTCP= new QGroupBox("Параметры TCP/IP",this);
    QFormLayout *lTCP = new QFormLayout();
    lTCP->addRow(lbIP, lIP);
    lTCP->addRow(lbPort, sbPort);
    lTCP->addRow(lbTimeOut, sbTimeOut);
    gbTCP->setLayout(lTCP);

    //кнопки
    QPushButton *btnOK = new QPushButton("Ок");
    QPushButton *btnCancel = new QPushButton("Отмена");
    QHBoxLayout *lButtons = new QHBoxLayout();
    lButtons->addWidget(btnOK);
    lButtons->addWidget(btnCancel);
    connect(btnOK, SIGNAL(clicked()), SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

    //метод соединения
    QLabel *lbMethod = new QLabel("Метод соединения:",this);
    cbMethod = new QComboBox(this);
    QFormLayout *lMethod = new QFormLayout();
    lMethod->addRow(lbMethod, cbMethod);
    cbMethod->addItem("COM");
    cbMethod->addItem("TCP/IP");

    //общий лэйаут
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(gbCOM, 0, 0);
    layout->addWidget(gbTCP, 0, 1);
    layout->addLayout(lMethod, 1, 0);
    layout->addLayout(lButtons, 1, 1);

    setLayout(layout);
}

void ConnectionSettings::accept()
{
    writeConSettings();
    //сигнал изменения параметров соединения
    emit connectionSettingsChanged();

    QDialog::accept();
}


