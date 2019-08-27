#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include "listofmonitors.h"
#include "connectionsettings.h"
#include "dbconnectionsettings.h"
#include "dbviewer.h"

//главное окно программы
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void findMonitors();//поиск подключенного оборудования
    void openConnectionSettings();//открыть окно настроек соединения
    void openChangeUpdateInterval();//открыть окно изменения интервала обновления
    void changeViewMode();//обработчик нажатия кнопок тектсовый/графический режим
    void openDBConnectionSettings();//открыть окно параметров подключения базы данных
    void openDBViewer();//открыть окно просмотра базы данных
private:
    void createActions();//задание возможных действий верхнего меню
    void createMenus();//построение меню

    ListOfMonitors *listMonitors;//список мониторов
    //возможные действия
    QAction *findMonitorsAct;//поиск мониторов
    QAction *connectionSettingsAct;//найстройка соединения с мониторами
    QAction *changeUpdateIntervalAct;//изменить интервал обновления
    QAction *switchMonitoringAct;//включить/выключить мониторинг
    QAction *textModeAct;//текстовое отображение
    QAction *graphModeAct;//графическое отображение
    QAction *switchLoggingAct;//включить/выключить логирование
    QAction *viewDBTableAct;//просмотр базы данных
    QAction *dbConSettingsAct;//настройка соединения с базой данных

    QMenu *viewDBMenu;//меню логирования

    DBViewer *dbViewer;//окно просмотра базы данных
protected:
    virtual void closeEvent(QCloseEvent *);//событие закрытия окна программы
public slots:
    void setLoggingBtnChecked(bool active);//вкл/выкл логирования
};

#endif // MAINWINDOW_H
