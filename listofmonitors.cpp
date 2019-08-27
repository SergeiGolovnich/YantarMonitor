#include "listofmonitors.h"

void ListOfMonitors::createContextMenu()
{
    contexMenu = new QMenu(this);

    switchMonitoringAct = new QAction("Вкл/Выкл мониторинг",this);
    connect(switchMonitoringAct, SIGNAL(triggered(bool)), SLOT(switchMonitoring()));
    contexMenu->addAction(switchMonitoringAct);

    changeMonitorViewModeAct = new QAction("Переключить отображение", this);
    connect(changeMonitorViewModeAct, SIGNAL(triggered(bool)), SLOT(changeMonitorViewMode()));
    contexMenu->addAction(changeMonitorViewModeAct);

    changeRegAct = new QAction("Регистры",this);
    connect(changeRegAct, SIGNAL(triggered(bool)), SLOT(changeRegValue()));
    contexMenu->addAction(changeRegAct);

    openArchiveAct = new QAction("Архив", this);
    connect(openArchiveAct, SIGNAL(triggered(bool)), SLOT(openArchive()));
    contexMenu->addAction(openArchiveAct);
}

ListOfMonitors::ListOfMonitors(QWidget *parent) : QListWidget(parent)
{
    //настройки отображения списка
    setViewMode(QListWidget::IconMode);
    setDragEnabled(false);
    setSelectionRectVisible(false);
    setSelectionMode(QAbstractItemView::NoSelection);
    setSpacing(10);

    //создание меню
    createContextMenu();

    clickedItem = 0;

    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(changeMonitorSize(QListWidgetItem*)));

    //регистрация класса для его использования в межпотоковом обмене событиями
    qRegisterMetaType<CmdHolder>("CmdHolder");

    //соединение с мониторами
    connection = ConnectionManager::GetConnection();
    connect(connection, SIGNAL(ResponseReceived(CmdHolder)), this, SLOT(ProcessResponse(CmdHolder)));
    connect(this, SIGNAL(sendRequest(CmdHolder)), connection, SLOT(SendRequest(CmdHolder)));

}

ListOfMonitors::~ListOfMonitors()
{
    for(int i = 0; i < this->count(); ++i)
    {
        WidgetMonitor *mon = getWdgtMonitorFromIndex(i);
        mon->deleteLater();
    }

    //закрытие соединений
    connection->deleteLater();
    if(logger)
        ResponseLogger::CloseConnection();
}

void ListOfMonitors::addMonitor(int number)
{
    //создание нового монитора
    WidgetMonitor *wm = new WidgetMonitor(number);

    //создание элемента списка
    QListWidgetItem *it = new QListWidgetItem(this);

    //задание размера элемента списка для вмещения монитора
    it->setSizeHint(wm->sizeHint());

    //добавление монитора в элеменет списка
    setItemWidget(it, wm);

    //обновление размеров элементов листа
    this->doItemsLayout();
}

void ListOfMonitors::findMonitors()
{
    this->clear();

    //отправляем запросы всем возможным устройствам
    CmdHolder cmd;
    cmd.setStartReg(0);
    cmd.setRegCount(1);

    for(int monAdr = 1; monAdr <= 16; ++monAdr)
    {
        cmd.setDeviceAdress(monAdr);
        emit sendRequest(cmd);
    }
}

void ListOfMonitors::changeUpdateIntervals(int interval)
{
    int count = this->count();

    for(int i = 0; i < count; ++i)
    {
        WidgetMonitor *mon = getWdgtMonitorFromIndex(i);

        mon->ChangeTimerInterval(interval);
    }
}

void ListOfMonitors::switchMonitoringAll()
{
    int count = this->count();

    for(int i = 0; i < count; ++i)
    {
        WidgetMonitor *mon = getWdgtMonitorFromIndex(i);

        mon->switchMonitoring();
    }
}

void ListOfMonitors::setTextMode(bool textMode)
{
    int count = this->count();

    for(int i = 0; i < count; ++i)
    {
        WidgetMonitor *mon = getWdgtMonitorFromIndex(i);

        mon->setMonitorViewMode(textMode);
    }

    resizeEvent(0);//обновление геометрии элементов
}

void ListOfMonitors::setLoggerActive(bool enable)
{
    //сохранение состояния логирования
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    conSettings.setValue("/isloggingactive", enable);
    conSettings.endGroup();

    if(enable)
    {
        //соединение с базой данных
        logger = ResponseLogger::GetResponseLogger();

        connect(this, SIGNAL(SaveToDB(CmdHolder)),logger,SLOT(SaveResponse(CmdHolder)));
        connect(logger, SIGNAL(ConnectionOpened(bool)),this->parent(),SLOT(setLoggingBtnChecked(bool)));
    }
    else
    {
        if(logger)
        {
            this->disconnect(logger);
            ResponseLogger::CloseConnection();
            logger = 0;
        }
    }
}

void ListOfMonitors::changeRegValue()
{
    if(!clickedItem) return;

    //получение объкта монитора
    QWidget *wdgt = this->itemWidget(clickedItem);
    WidgetMonitor *mon = qobject_cast<WidgetMonitor *>(wdgt);
    if(mon)
    {
        mon->changeRegValue();
    }
    //выставление новых размеров элемента списка
    clickedItem->setSizeHint(mon->sizeHint());
    this->doItemsLayout();//обновление геометрии элементов
}

WidgetMonitor * ListOfMonitors::getWdgtMonFromLstItem(QListWidgetItem *Item)
{
    QWidget *wdgt = this->itemWidget(Item);
    WidgetMonitor *mon = qobject_cast<WidgetMonitor *>(wdgt);

    return mon;
}

void ListOfMonitors::switchMonitoring()
{
    if(!clickedItem) return;

    WidgetMonitor *mon = getWdgtMonFromLstItem(clickedItem);
    if(mon)
    {
        mon->switchMonitoring();
    }
    //выставление новых размеров элемента списка
    clickedItem->setSizeHint(mon->sizeHint());
    this->doItemsLayout();//обновление геометрии элементов
}

void ListOfMonitors::openArchive()
{
    if(!clickedItem) return;

    WidgetMonitor *mon = getWdgtMonFromLstItem(clickedItem);
    if(mon)
    {
        mon->openMonitorHistory();
    }
    //выставление новых размеров элемента списка
    clickedItem->setSizeHint(mon->sizeHint());
    this->doItemsLayout();//обновление геометрии элементов
}

void ListOfMonitors::ProcessResponse(CmdHolder ch)
{   
    int addr = ch.getDeviceAdress();
    int count = this->count();

    //отправка команды конкретному монитору
    for(int i = 0; i < count; ++i)
    {
        WidgetMonitor *mon = getWdgtMonitorFromIndex(i);
        if(mon->monitorNumber == addr)
        {
            //расшифровка ответа от монитора
            mon->ProcessResponse(ch);
            //сохранить ответ в базу данных
            emit SaveToDB(ch);
            return;
        }
    }
    //добавление нового монитора в список
    if(ch.getStartReg() == 0)
    {
        QDataStream in(ch.getData());
        quint16 value;
        in >> value;
        if(value == 0x4D41) //признак начала блока описания устройства
            addMonitor(addr);
    }
}

void ListOfMonitors::changeMonitorSize(QListWidgetItem *item)
{
    WidgetMonitor *mon = getWdgtMonFromLstItem(item);

    mon->changeGraphSize();

    //выставление рекомендуемых размеров виджета
    item->setSizeHint(mon->sizeHint());
    this->doItemsLayout();
}

void ListOfMonitors::changeMonitorViewMode()
{
    if(!clickedItem) return;

    WidgetMonitor *mon = getWdgtMonFromLstItem(clickedItem);

    if(mon)
    {
        mon->switchViewMode();
    }

    //выставление новых размеров элемента списка
    clickedItem->setSizeHint(mon->sizeHint());
    this->doItemsLayout();
}

WidgetMonitor *ListOfMonitors::getWdgtMonitorFromIndex(int index)
{
    QListWidgetItem *item = this->item(index);//получение элемента списка

    WidgetMonitor *mon = getWdgtMonFromLstItem(item);

    return mon;
}

void ListOfMonitors::contextMenuEvent(QContextMenuEvent *cme)
{
    //определяется кликнутный элемент
    clickedItem = this->itemAt(cme->pos());
    //выставляются возможные действия
    if(clickedItem)
    {
        switchMonitoringAct->setEnabled(true);
        changeRegAct->setEnabled(true);
        openArchiveAct->setEnabled(true);
        changeMonitorViewModeAct->setEnabled(true);
    }
    else
    {
        switchMonitoringAct->setEnabled(false);
        changeRegAct->setEnabled(false);
        openArchiveAct->setEnabled(false);
        changeMonitorViewModeAct->setEnabled(false);
    }
    //вызывается меню
    contexMenu->exec(cme->globalPos());
}

void ListOfMonitors::resizeEvent(QResizeEvent *)
{
    for(int i = 0; i < this->count(); ++i)
    {
        QListWidgetItem *item = this->item(i);
        WidgetMonitor *mon = getWdgtMonFromLstItem(item);
        //выставление рекомендуемых размеров виджета
        item->setSizeHint(mon->sizeHint());
    }
    //обновление размеров элементов листа
    this->doItemsLayout();
}

