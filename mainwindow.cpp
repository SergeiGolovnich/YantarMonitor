#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dbViewer(0)
{
    //установка минимального размера окна
    setMinimumWidth(260);
    setMinimumHeight(370);

    listMonitors = new ListOfMonitors(this);
    createActions();
    createMenus();

    setCentralWidget(listMonitors);//установка списка мониторов главным виджетом окна

    resize(820, 680);
}

MainWindow::~MainWindow() {}

void MainWindow::createActions()
{
    findMonitorsAct = new QAction("&Поиск устройств",this);
    connect(findMonitorsAct, SIGNAL(triggered()), SLOT(findMonitors()));
    findMonitorsAct->setStatusTip("Поиск подключенных устройств");

    connectionSettingsAct = new QAction("&Настройка соединения...",this);
    connect(connectionSettingsAct, SIGNAL(triggered()), SLOT(openConnectionSettings()));
    connectionSettingsAct->setStatusTip("Настройка соединения с устройствами");

    changeUpdateIntervalAct = new QAction("&Интервал обновления...",this);
    connect(changeUpdateIntervalAct, SIGNAL(triggered()), SLOT(openChangeUpdateInterval()));
    changeUpdateIntervalAct->setStatusTip("Изменить интервал опроса устройств");

    switchMonitoringAct = new QAction("&Вкл/Выкл мониторинг", this);
    connect(switchMonitoringAct, SIGNAL(triggered(bool)), listMonitors, SLOT(switchMonitoringAll()));
    switchMonitoringAct->setStatusTip("Включить/Выключить опрос устройств");

    textModeAct = new QAction("&Текстовый режим", this);
    connect(textModeAct, SIGNAL(triggered(bool)), this, SLOT(changeViewMode()));
    textModeAct->setStatusTip("Текстовое отображение состояния устройств");

    graphModeAct = new QAction("&Графический режим", this);
    connect(graphModeAct, SIGNAL(triggered(bool)), this, SLOT(changeViewMode()));
    graphModeAct->setStatusTip("Графическое отображение состояния устройств");

    switchLoggingAct = new QAction("&Логирование", this);
    switchLoggingAct->setCheckable(true);
    connect(switchLoggingAct,SIGNAL(toggled(bool)),listMonitors,SLOT(setLoggerActive(bool)));
    switchLoggingAct->setStatusTip("Включить/Выключить сохранение состояний устройств в базе данных");

    //считывание состояния логирования
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    switchLoggingAct->setChecked(conSettings.value("/isloggingactive", true).toBool());
    conSettings.endGroup();

    viewDBTableAct = new QAction("Просмотр базы данных...", this);
    connect(viewDBTableAct, SIGNAL(triggered(bool)), this, SLOT(openDBViewer()));
    viewDBTableAct->setStatusTip("Просмотр последних сохраненных состояний устройств");

    dbConSettingsAct = new QAction("&Параметры соединения...", this);
    connect(dbConSettingsAct, SIGNAL(triggered(bool)), this, SLOT(openDBConnectionSettings()));
    dbConSettingsAct->setStatusTip("Настроить параметры соединения с базой данных");
}

void MainWindow::createMenus()
{
    menuBar()->addAction(findMonitorsAct);
    menuBar()->addAction(connectionSettingsAct);
    menuBar()->addAction(changeUpdateIntervalAct);
    menuBar()->addAction(switchMonitoringAct);

    QMenu *viewModeMenu = new QMenu("&Отображение", menuBar());
    viewModeMenu->addAction(textModeAct);
    viewModeMenu->addAction(graphModeAct);
    menuBar()->addMenu(viewModeMenu);

    viewDBMenu = new QMenu("&Логирование", menuBar());
    viewDBMenu->addAction(switchLoggingAct);
    viewDBMenu->addAction(viewDBTableAct);
    viewDBMenu->addAction(dbConSettingsAct);
    menuBar()->addMenu(viewDBMenu);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    listMonitors->deleteLater();
    if(dbViewer)
        dbViewer->deleteLater();
}

void MainWindow::setLoggingBtnChecked(bool active)
{
    switchLoggingAct->setChecked(active);
    //показ подменю "Логирование" при оключении соединения с базой
    if(!active)
    {
        viewDBMenu->popup(this->mapToGlobal(QPoint(this->width()-100, this->menuBar()->height())));
        viewDBMenu->setActiveAction(switchLoggingAct);
        statusBar()->showMessage("Логирование отключено");
    }
}

void MainWindow::findMonitors()
{
    statusBar()->showMessage("Выполняется поиск подключенных устройств");

    listMonitors->findMonitors();
}

void MainWindow::openConnectionSettings()
{
    ConnectionSettings cs;
    if(cs.exec() == QDialog::Accepted)//ввод новых параметров соединения с мониторами
    {
        //переподключаемся к мониторам
        findMonitors();
    }
}

void MainWindow::openChangeUpdateInterval()
{
    int currentInterval;
    //считывание текущего интервала обновления
    QSettings conSettings;
    currentInterval = conSettings.value("/updateinterval", 1000).toInt();

    //вывод окна ввода нового значения
    bool ok;
    currentInterval = QInputDialog::getInt(this, "Изменение интервала обновления", "Интервал обновления в миллисекундах:",
                                           currentInterval, 100, 60000, 50, &ok);
    //если ввод отменен
    if(!ok)
        return;

    //сохранение нового интервала
    conSettings.setValue("/updateinterval", currentInterval);
    //изменение интервала для подключенных мониторов
    listMonitors->changeUpdateIntervals(currentInterval);
}

void MainWindow::changeViewMode()
{
    QAction *senderAct = (QAction *)sender();
    if(senderAct == textModeAct) listMonitors->setTextMode(true);
    if(senderAct == graphModeAct) listMonitors->setTextMode(false);
}

void MainWindow::openDBConnectionSettings()
{
    DBConnectionSettings dbcs;
    if(dbcs.exec() == QDialog::Accepted)
    {
        if(switchLoggingAct->isChecked()){
            //переподключение к базе
            ResponseLogger::ResetConnection();
        }
    }
}

void MainWindow::openDBViewer()
{
    if(!dbViewer){
        dbViewer = new DBViewer();
    }

    dbViewer->showNormal();
    dbViewer->activateWindow();
}
