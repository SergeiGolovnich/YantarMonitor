#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H
#include <QObject>
#include <QtSerialPort/QtSerialPort>
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include "cmdholder.h"


//Класс предназначен для отправки/приема сообщений от устройств
class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ~ConnectionManager();
    static ConnectionManager *GetConnection();//получение объекта подключения

    int method;//метод подключения, 0 - COM, 1 - TCP
    int port;//порт TCP/IP
    int timeout;//время ожидания ответа от устройства
    QSerialPort serialPort;
    QTcpSocket socket;
    QHostAddress address;
private:
    explicit ConnectionManager(QObject *parent = 0);
    static ConnectionManager *connection;//ссылка на единственный экземпляр соединения с мониторами
    static QThread thread; //объект управления потоком, в котором выполняется соединение с устройствами

    void writeToSocket(CmdHolder &ch);//отправить через tcp сокет
    void writeToSerial(CmdHolder &ch);//отправить через последовательный порт
    void readConnectionSettings();//чтение параметров из памяти
    void timeOutResponse(CmdHolder &ch);//Выставление ошибки TimeOut

signals:
    void ResponseReceived(CmdHolder);//сигнал получения ответа от устройств
public slots:
    void reset();//перезагрузка параметров соединений
    void SendRequest(CmdHolder);//оправить команду установленным методом
private slots:

};

#endif // CONNECTIONMANAGER_H
