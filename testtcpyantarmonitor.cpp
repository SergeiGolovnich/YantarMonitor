#include "testtcpyantarmonitor.h"

void TestTcpYantarMonitor::CmdToText(CmdHolder ch)
{
    QString str;
    str += "Request: ";
    str += QString::number(ch.getDeviceAdress()) + " ";
    str += QString::number((int)ch.getFunc()) + " ";
    str += QString::number(ch.getStartReg()) + " ";
    str += QString::number(ch.getRegCount());
    str += "\n";

    textEdit->append(str);
}

CmdHolder TestTcpYantarMonitor::getCmdFromArray(QByteArray arr)
{
    CmdHolder cmd;
    //определение канала, по которому получено сообщение
    QDataStream in(arr);
    in.device()->seek(2);
    quint16 pi;//идентификатор протокола
    in >> pi;
    quint16 messLen;
    in >> messLen;
    //если идентификатор протокола pi == 0 и длина сообщения messLen == оставшемуся количеству байт в сообщении
    if((pi == 0) && ((arr.length() - 6) == messLen))
    {//получается сообщение принято по TCP/IP
        ;//действия не требуются
    }else
    {//сообщение по COM
        in.device()->seek(0);
    }
    //считывание PDU
    quint8 address;
    in >> address;//адрес монитора
    cmd.setDeviceAdress(address);
    quint8 f;
    in >> f;//номер функции
    cmd.setFunc((CmdHolder::Function)f);
    if(cmd.getFunc() == CmdHolder::ReadHoldingRegisters)
    {//запрос на чтение регистров
        quint16 regStart;
        in >> regStart;
        cmd.setStartReg(regStart);

        quint16 regCount;
        in >> regCount;
        cmd.setRegCount(regCount);
    }else if(cmd.getFunc() == CmdHolder::PresetMultipleRegisters)
    {//запрос на запись регистров
        quint16 regStart;
        in >> regStart;
        cmd.setStartReg(regStart);

        quint16 regCount;
        in >> regCount;
        cmd.setRegCount(regCount);
    }

    return cmd;
}

QString TestTcpYantarMonitor::ArrayToString(QByteArray arr)
{
    QString ret;
    QByteArray::const_iterator i;
    QDataStream in(arr);
    quint8 byte;

    for(i = arr.constBegin(); i != arr.constEnd(); ++i)
    {
        in >> byte;
        ret += " ";
        ret.append(QString::number(byte, 16).toUpper());
    }

    return ret;
}

void TestTcpYantarMonitor::SendResponse(CmdHolder ch)
{
    QByteArray byteArray;
    QDataStream in(&byteArray, QIODevice::WriteOnly);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 val8;
    quint16 val16;
    quint32 val32;
    float valFloat;
    //заголовок MBAP
    in << (val16 = 0) << (val16 = 0)//константы протокола ModBus
       << (val16 = 0);//количество байт данных передаваемых далее
    in << ch.getDeviceAdress();//адрес монитора
    in << (quint8)ch.getFunc();//номер функции

    if((ch.getFunc() == CmdHolder::ReadHoldingRegisters) && (ch.getStartReg() == 0))
    {//инициализация
        in << (val8 = 2);//количество байт данных
        in << (val16 = 0x4D41);//значение регистра
        in.device()->seek(4);//переход на нужный байт
        in << (val16 = 5);//число байт в сообщении, указанное в заголовке
    }else if((ch.getFunc() == CmdHolder::ReadHoldingRegisters) && (ch.getStartReg() == 100) && (ch.getRegCount()== 17))
    {//счет каналов
        in << (val8 = 34);//количество записанных байт
        //время монитора
        static QDateTime dateTimeStart(QDate(1970, 1, 1), QTime(0, 0));
        uint time = dateTimeStart.secsTo(QDateTime::currentDateTime());
        time++;
        CmdHolder::SwapUInt32(time);
        in << (val32 = time);
        //состояние входных каналов
        in << (val16 = (quint16)qrand());
        //гамма канал
        in << (val16 = qrand());//состояние канала

        val32 = (quint8)qrand() / 5 + 180;
        CmdHolder::SwapUInt32(val32);
        in << val32;//счет канала

        valFloat = (quint8)qrand() / 6 + 100;
        CmdHolder::SwapFloat(&valFloat);
        in << (valFloat);//оценка фона

        in << (valFloat = (quint8)qrand());//оценка дисперсии
        //нейтронный канал
        in << (val16 = qrand());//состояние канала

        val32 = (quint8)qrand() / 2 + 450;
        CmdHolder::SwapUInt32(val32);
        in << val32;//счет канала

        valFloat = (quint8)qrand() / 5 + 300;
        CmdHolder::SwapFloat(&valFloat);
        in << valFloat;//оценка фона

        in << (valFloat = (quint8)qrand());//оценка дисперсии

        in.device()->seek(4);
        in << (val16 = 37);

        if(((quint8)qrand()) < 8)
        {//случайная ошибка
            in.device()->seek(7);
            in << (val8 = 131);//выставление кода ошибки
            in << (val16 = 256);//код ошибки
        }
    }else if((ch.getFunc() == CmdHolder::ReadHoldingRegisters) && (ch.getStartReg() == 126) && (ch.getRegCount()== 12))
    {//диапазоны измерений
        in << (val8 = 22);
        //гамма канал
        in << (val16 = 0);
        in << (val16 = 1024);
        in << (val16 = 0);
        in << (val16 = 0) << (val16 = 0) << (val16 = 0) << (val16 = 0) << (val16 = 0);
        //нейтронный канал
        in << (val16 = 0);
        in << (val16 = 0);
        in << (val16 = 1024);

        in.device()->seek(4);
        in << (val16 = 25);
    }
    else if((ch.getFunc() == CmdHolder::ReadHoldingRegisters) &&
            (ch.getStartReg() == 151) &&
            (ch.getRegCount() == 10))
    {//архивная запись
        in << (val8 = 20);

        val32 = (quint32)qrand();
        CmdHolder::SwapUInt32(val32);
        in << val32;//время
        in << (val16 = (quint8)qrand() << 1);//код события

        val32 = (quint32)qrand();
        CmdHolder::SwapUInt32(val32);
        in << val32;//счет

        valFloat = (qint32)qrand();
        CmdHolder::SwapFloat(&valFloat);
        in << valFloat;//фон
        in << (val16 = (quint16)qrand());//номер объекта
        in << (val16 = (quint8)qrand());//доп информация об объекте
        in << (val16 = (qint8)qrand() ? 1 : -1);//управление

        in.device()->seek(4);
        in << (val16 = 23);
    }else if((ch.getFunc() == CmdHolder::ReadHoldingRegisters) &&
             ((ch.getRegCount() == 1) || (ch.getRegCount() == 2)))
    {//запрос регистра
        if(ch.getRegCount() == 1)//один регистр
        {
            in << (val8 = 2);

            in << (val16 = (quint16)qrand());

            in.device()->seek(4);
            in << (val16 = 5);
        }else
        {//два регистра
            in << (val8 = 4);

            val32 = (quint32)qrand();
            CmdHolder::SwapUInt32(val32);
            in << val32;

            in.device()->seek(4);
            in << (val16 = 7);
        }

    }else if((ch.getFunc() == CmdHolder::PresetMultipleRegisters) && ((ch.getRegCount()== 1) || (ch.getRegCount()== 2)))
    {//запрос на запись регистра
        if((quint8)qrand() > 128)
        {//случайная ошибка
            in << (val8 = 4);

            in.device()->seek(7);
            in << (val8 = (quint8)ch.getFunc() + 0x80);

            in.device()->seek(4);
            in << (val16 = 3);
        }else
        {//успешная запись
            in << (val16 = ch.getStartReg());

            in << (val16 = ch.getRegCount());

            in.device()->seek(4);
            in << (val16 = 6);
        }
    }

        tcpClient->write(byteArray);
}

TestTcpYantarMonitor::TestTcpYantarMonitor(QWidget *parent): QWidget(parent), tcpClient(0)
{
    //графический интерфейс
    setWindowTitle("Тестовый монитор Янтарь по Tcp/ip");

    textEdit = new QTextEdit(this);
    //lineEdit = new QLineEdit(this);

    QVBoxLayout *vLay = new QVBoxLayout();
    vLay->addWidget(textEdit);

    this->setLayout(vLay);

    //сокет
    tcpServer = new QTcpServer(this);
    if(!tcpServer->listen(QHostAddress::Any, 502))//запуск сервера на стандартном ModBus порту
    {
        QMessageBox::critical(0, "Ошибка запуска сервера", tcpServer->errorString());
        tcpServer->close();
        return;
    }

    tcpServer->setMaxPendingConnections(1);//возможность подключения только одного клиента единовременно
    connect(tcpServer, SIGNAL(newConnection()), SLOT(slotNewConnection()));
}

void TestTcpYantarMonitor::slotNewConnection()
{
    tcpClient = tcpServer->nextPendingConnection();
    connect(tcpClient, SIGNAL(disconnected()), SLOT(slotClientDisconnected()));
    connect(tcpClient, SIGNAL(readyRead()), SLOT(slotReadClient()));

    textEdit->append("Клиент подключен\n");

    tcpServer->pauseAccepting();//прекращение принятия подключений
}

void TestTcpYantarMonitor::slotReadClient()
{
    QByteArray data = tcpClient->readAll();

    if(tcpClient->waitForReadyRead(10))
        data += tcpClient->readAll();

    CmdHolder ch = getCmdFromArray(data);
    //CmdToText(ch);//вывод принятого сообщения

    SendResponse(ch);
}

void TestTcpYantarMonitor::slotClientDisconnected()
{
    QTcpSocket *cli = (QTcpSocket *)sender();
    cli->deleteLater();
    tcpClient = 0;
    textEdit->append("Клиент отключен\n");

    try{
        tcpServer->resumeAccepting();//вызывает аварийное завершение программы при закрытии окна
    }
    catch(...){
        qDebug()<<"Ошибка повторного запуска сервера";
    }
}

void TestTcpYantarMonitor::closeEvent(QCloseEvent *)
{
    if(tcpClient)
        emit tcpClient->disconnected();//сигнал об отсоединении клиента
    if(tcpServer->isListening())
        tcpServer->close();
}


