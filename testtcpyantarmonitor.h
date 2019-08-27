#ifndef TESTTCPYANTARMONITOR_H
#define TESTTCPYANTARMONITOR_H
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QtWidgets>
#include <QTcpServer>

#include "cmdholder.h"

//эмуляция мониторов Янтарь, объединенных через последовательное соединение в сеть, подключенной по TCP/IP
class TestTcpYantarMonitor: public QWidget
{
    Q_OBJECT
private:
    QTcpServer *tcpServer;//сокет для принятия подключений
    QTcpSocket *tcpClient;//активный клиент
    QTextEdit *textEdit;//поле вывода уведомлений
    QLineEdit *lineEdit;//поле для ввода команд

    void CmdToText(CmdHolder ch);//вывести комнду в поле вывода уведолений
    CmdHolder getCmdFromArray(QByteArray arr);//расшифровать команду
    QString ArrayToString(QByteArray arr);//преобразовать массив байтов в стровый вид
    void SendResponse(CmdHolder ch);//ответ на запрос клиента
public:
    TestTcpYantarMonitor(QWidget *parent = 0);
public slots:
    void slotNewConnection();//действия при подключении клиента
    void slotReadClient();//действия при получении запроса от клиента
    void slotClientDisconnected();//действия при отключении клиента
protected:
    void closeEvent(QCloseEvent *);//действия при закрытии окна
};

#endif // TESTTCPYANTARMONITOR_H
