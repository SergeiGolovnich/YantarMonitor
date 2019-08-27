#ifndef LISTOFMONITORS_H
#define LISTOFMONITORS_H

#include <QtWidgets>

#include "widgetmonitor.h"
#include "connectionmanager.h"
#include "responselogger.h"

//класс для отображения списка мониторов
class ListOfMonitors : public QListWidget
{
    Q_OBJECT

public:
    ListOfMonitors(QWidget *parent = 0);
    ~ListOfMonitors();
    ConnectionManager *connection;//ссылка на соединение с мониторами
    ResponseLogger *logger;//ссылка на логер

public slots:
    void addMonitor(int);//добавление монитора в список
    void findMonitors();//поиск устройств
    void changeUpdateIntervals(int interval);//изменение интервала обновления мониторов
    void switchMonitoringAll();//вкл/выкл мониторинг для всех мониторов
    void setTextMode(bool);//выставит заданное отображение для мониторов
    void setLoggerActive(bool);//вкл/выкл логирование в базу

private slots:
    void changeRegValue();//изменение значения регистра
    void switchMonitoring();//вкл/выкл мониторинга
    void openArchive();//открыть архив монитора
    void ProcessResponse(CmdHolder);//обработка полученных сообщений от мониторов
    void changeMonitorSize(QListWidgetItem *);//изменить размер графиков монитора по двойному клику
    void changeMonitorViewMode();//сменить режим отображения монитора

private:
    QListWidgetItem *clickedItem;//текущий кликнутый элемент
    QMenu *contexMenu;//контекстное меню
    QAction *switchMonitoringAct;//включение/выключение запросов к монитору
    QAction *changeRegAct;//изменение значения регистра
    QAction *openArchiveAct;//открытие архива монитора
    QAction *changeMonitorViewModeAct;//переключить режим отображения монитора

    WidgetMonitor *getWdgtMonitorFromIndex(int index);//получить указатель на монитор по индексу
    WidgetMonitor *getWdgtMonFromLstItem(QListWidgetItem *Item);//получить указатель на монитор по элементу листа
    void createContextMenu();//создание контекстного меню

protected:
    virtual void contextMenuEvent(QContextMenuEvent *cme);//обработка контекстного меню
    virtual void resizeEvent(QResizeEvent *);//обработка изменения размера окна

signals:
    void sendRequest(CmdHolder);//отправить запрос
    void SaveToDB(CmdHolder);//сохранить ответ в базе данных
};

#endif // LISTOFMONITORS_H
