#ifndef RESPONSELOGGER_H
#define RESPONSELOGGER_H

#include <QObject>
#include <QtSql>
#include <QDataStream>
#include <QQueue>
#include <QMessageBox>

#include "cmdholder.h"

//Класс предназначен для управления подключением к базе данных
class ResponseLogger : public QObject
{
    Q_OBJECT
public:
    QSqlDatabase db;
    ~ResponseLogger();
    static ResponseLogger *GetResponseLogger();//получение объекта подключения к базе данных
    static void CloseConnection();//удаление существующего соединения
    static void ResetConnection();//Переподключение к базе
private:
    explicit ResponseLogger(QObject *parent = 0);
    static ResponseLogger *logger;//ссылка на единственный экземпляр логгера
    static QThread thread; //объект управления потоком, в котором выполняется соединение с базой данных
    QQueue<CmdHolder> responsesQueue;//очередь для хранения ответов мониторов
    QTimer *timer;//таймер, по которому происходит сохранение значений в базу

    QString getInsertStringFromCmdHolder(CmdHolder ch);//получить строку-запрос на добавление ответа в базу
    bool CreateConnection();//подключиться к базе данных
    bool createTable();//создать таблицу для хранения ответов мониторов
signals:
    void ConnectionOpened(bool);//сигнал об успешном/неуспешном подключении к базе данных
public slots:
    void SaveResponse(CmdHolder);//поместить ответ от монитора в очередь
private slots:
    void WriteResponsesToDB();//записать ответы из очереди в базу данных
    void SaveAndCloseDB();//сохранить данные и закрыть соединение с базой
};

#endif // RESPONSELOGGER_H
