#ifndef MONITORHISTORY_H
#define MONITORHISTORY_H
#include <QObject>
#include <QtWidgets>
#include "widgetmonitor.h"
#include "cmdholder.h"

class ConnectionManager;

//окно отображения архива монитора
class MonitorHistory : public QWidget
{
    Q_OBJECT
public:
    explicit MonitorHistory(int monNum, QWidget *parent = 0);
signals:
    void sendRequest(CmdHolder);//послать запрос монитору
public slots:
    void readEventFromArchive(CmdHolder &);//добавление события в таблицу
    void ResetArchive();//поставить указатель архива на первую запись
    void saveArchiveToFile();//сохранить архив в файл
private:
    int monitorNumber;//номер монитора
    QTableWidget *tableHistory;//таблица для отображения записей архива
    ConnectionManager *connection;

    void createWidget();//создать разметку окна
    void sendRequestForEvent();//запросить следующую запись архива
    QString getArchiveEventDescription(const quint16 reg);//получить текстовое описание события
    QString getArchiveChannelDescription(const quint16 reg);//получить текстовое название канала
    void sendResetCmd();//обнуление регистра управления (№160)
};

#endif // MONITORHISTORY_H
