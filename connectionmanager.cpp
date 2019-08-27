#include "connectionmanager.h"

ConnectionManager *ConnectionManager::connection;
QThread ConnectionManager::thread;

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent)
{
    //перемещение класса в отдельный поток
    this->moveToThread(&thread);
    socket.moveToThread(&thread);
    serialPort.moveToThread(&thread);

    readConnectionSettings();
}

void ConnectionManager::writeToSocket(CmdHolder &ch)
{
    //запись в сокет
    socket.write(ch.getQueryCommand(false));

    if(!socket.waitForBytesWritten(timeout))//ожидание отправки запроса
    {
        qWarning() << socket.errorString();
        return;
    }

    QByteArray data;
    //ожидание ответа
    if(socket.waitForReadyRead(timeout))
    {
        data = socket.readAll();

        if(socket.waitForReadyRead(10))//дополнительное время ожидания из-за возможной малой скорости передачи данных
            data += socket.readAll();//чтение оставшихся данных

        ch.processResponse(data);//расшифровка ответа

        emit ResponseReceived(ch);//сигнал о получении ответа
    }
    else{
        qWarning() << socket.errorString();
        timeOutResponse(ch);//сигнал о таймауте ожидания ответа
    }
}

void ConnectionManager::writeToSerial(CmdHolder &ch)
{
    //запись в последовательный порт
    serialPort.write(ch.getQueryCommand(true));
    if(!serialPort.waitForBytesWritten(timeout))//ожидание отправки запроса
    {
        qWarning() << serialPort.errorString();
        return;
    }

    QByteArray data;
    //ожидание ответа
    if(serialPort.waitForReadyRead(timeout))
    {
        data = serialPort.readAll();

        if(serialPort.waitForReadyRead(10))//дополнительное время ожидания из-за возможной малой скорости передачи данных
            data += serialPort.readAll();

        ch.processResponse(data);//расшифровка ответа

        emit ResponseReceived(ch);//сигнал о получении ответа
    }
    else{
        qWarning() << serialPort.errorString();
        timeOutResponse(ch);//сигнал о таймауте ожидания ответа
    }
}

ConnectionManager::~ConnectionManager()
{
    thread.exit();
    connection->deleteLater();
    connection = 0;
}

ConnectionManager *ConnectionManager::GetConnection()
{
    if(!connection)
    {
        thread.start(QThread::NormalPriority);
        connection = new ConnectionManager();
    }
    return connection;
}

void ConnectionManager::readConnectionSettings()
{
    QSettings conSettings;
    conSettings.beginGroup("/ConnectionSettings");
    //параметры COM-порта
    QString serialPortName = conSettings.value("/serialportname", "").toString();//название порта
    int baudRate = conSettings.value("/baudrate", -1).toInt();//скорость
    int dataBits = conSettings.value("/databits", -1).toInt();//количество бит данных
    int parity = conSettings.value("/parity", -1).toInt();//четность
    int stopBits = conSettings.value("/stopbits", -1).toInt();//количество стоп битов

    //TCP
    QString ip = conSettings.value("/ip", "127.0.0.1").toString();//ip адрес
    port = conSettings.value("/port", 502).toInt();//порт
    timeout = conSettings.value("/timeout", 200).toInt();//максимальное время выполнения операций
    //метод соединения
    method = conSettings.value("/method", 0).toInt();
    conSettings.endGroup();

    //COM
    if(!serialPortName.isEmpty())
    {
        serialPort.setPortName(serialPortName);
        serialPort.setBaudRate((QSerialPort::BaudRate)baudRate, QSerialPort::AllDirections);
        serialPort.setDataBits((QSerialPort::DataBits)dataBits);
        serialPort.setParity((QSerialPort::Parity)parity);
        serialPort.setStopBits((QSerialPort::StopBits)stopBits);
    }
    //TCP
    address.setAddress(ip);
}

void ConnectionManager::timeOutResponse(CmdHolder &ch)
{
    ch.setFunc((CmdHolder::Function)(ch.getFunc() + 0x80));

    QByteArray errCode;
    QDataStream dataStream(&errCode, QIODevice::WriteOnly);
    dataStream << (quint8)0x80;//запись номера ошибки в блок данных

    ch.setData(errCode);

    emit ResponseReceived(ch);//отправление сигнала о timeout
}

void ConnectionManager::reset()
{
    serialPort.close();
    socket.close();
    readConnectionSettings();
}

void ConnectionManager::SendRequest(CmdHolder cmd)
{
    if(method)
    {//tcp
        if(socket.state() != socket.ConnectedState)//если нет соединения
        {
            socket.setReadBufferSize(300);//ограничение на размер буфера сокета
            socket.connectToHost(address, port, QTcpSocket::ReadWrite);
            socket.setSocketOption(QTcpSocket::LowDelayOption, QVariant(1));//маленькие блоки данных отправляются сразу же, не объединяясь

            if(!socket.waitForConnected(timeout))
            {
                qWarning() << socket.errorString();
                timeOutResponse(cmd);
                return;
            }
        }

        writeToSocket(cmd);
    }
    else
    {//com
        if(!serialPort.isOpen())//если порт не открыт
        {
            if(!serialPort.open(QIODevice::ReadWrite))
            {
                qWarning() << serialPort.errorString();
                timeOutResponse(cmd);
                return;
            }
            serialPort.setReadBufferSize(300);//ограничение на буфер последовательного порта
        }

        writeToSerial(cmd);
    }
}

