#include "registerblockhelper.h"

RegisterBlockHelper::RegisterBlockHelper(QWidget *parent) : QWidget(parent)
{
    createTables();
    createHelpWindow();
}

void RegisterBlockHelper::changedSelectedRegister(int regNumber)
{
    //выставление блока и номера регистра
    if(regNumber < (quint16)CmdHolder::CountersStatus)
    {
        mainTabs->setCurrentIndex(0);
        tableConSettings->setCurrentCell(regNumber - (quint16)CmdHolder::ConSettings, 0);
    }else
        if(regNumber < (quint16)CmdHolder::MonitorRegs)
        {
            mainTabs->setCurrentIndex(1);
            tableCountersStatus->setCurrentCell(regNumber - (quint16)CmdHolder::CountersStatus, 0);
        }else
            if(regNumber < (quint16)CmdHolder::InnerModbusNetwork)
            {
                mainTabs->setCurrentIndex(2);
                tableMonitorRegs->setCurrentCell(regNumber - (quint16)CmdHolder::MonitorRegs, 0);
            }else
                if(regNumber < (quint16)CmdHolder::ModulesStatus)
                {
                    mainTabs->setCurrentIndex(3);
                    tableInnerModbusNetwork->setCurrentCell(regNumber - (quint16)CmdHolder::InnerModbusNetwork, 0);
                }else
                    if(regNumber >= (quint16)CmdHolder::ModulesStatus)
                    {
                        mainTabs->setCurrentIndex(4);
                        tableModulesStatus->setCurrentCell(regNumber - (quint16)CmdHolder::ModulesStatus, 0);
                    }
}

void RegisterBlockHelper::doubleClickedRegister(int row, int )
{
    int regBaseAdrr = 0;
    QTableWidget *currentTable = 0;
    //определение базового адреса блока
    switch(mainTabs->currentIndex())
    {
    case 0:
        regBaseAdrr = (int)CmdHolder::ConSettings;
        currentTable = tableConSettings;
        break;
    case 1:
        regBaseAdrr = (int)CmdHolder::CountersStatus;
        currentTable = tableCountersStatus;
        break;
    case 2:
        regBaseAdrr = (int)CmdHolder::MonitorRegs;
        currentTable = tableMonitorRegs;
        break;
    case 3:
        regBaseAdrr = (int)CmdHolder::InnerModbusNetwork;
        currentTable = tableInnerModbusNetwork;
        break;
    case 4:
        regBaseAdrr = (int)CmdHolder::ModulesStatus;
        currentTable = tableModulesStatus;
        break;
    }

    bool Twobytes = false;
    if((mainTabs->currentIndex() == 2) &&
            ((tableMonitorRegs->item(row, 1)->text() == "FLOAT") ||
             (tableMonitorRegs->item(row, 1)->text() == "DWORD")))
        Twobytes = true;

    emit registerDoubleClicked(regBaseAdrr + currentTable->currentRow(), Twobytes);
}

void RegisterBlockHelper::createHelpWindow()
{
    setWindowTitle("Справка по регистрам");
    setMinimumSize(600, 400);

    mainTabs = new QTabWidget(this);
    mainTabs->addTab(tableConSettings, "Регистры настройки узла сети");
    mainTabs->addTab(tableCountersStatus, "Регистры состояния счетчиков");
    mainTabs->addTab(tableMonitorRegs, "Регистры монитора");
    mainTabs->addTab(tableInnerModbusNetwork, "Регистры доступа к внутренней сети");
    mainTabs->addTab(tableModulesStatus, "Регистры состояния модулей монитора");

    connect(tableConSettings, SIGNAL(cellDoubleClicked(int,int)), SLOT(doubleClickedRegister(int,int)));
    connect(tableCountersStatus, SIGNAL(cellDoubleClicked(int,int)), SLOT(doubleClickedRegister(int,int)));
    connect(tableMonitorRegs, SIGNAL(cellDoubleClicked(int,int)), SLOT(doubleClickedRegister(int,int)));
    connect(tableInnerModbusNetwork, SIGNAL(cellDoubleClicked(int,int)), SLOT(doubleClickedRegister(int,int)));
    connect(tableModulesStatus, SIGNAL(cellDoubleClicked(int,int)), SLOT(doubleClickedRegister(int,int)));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(3);
    layout->addWidget(mainTabs);
    this->setLayout(layout);
}

void RegisterBlockHelper::createTables()
{
    QTableWidgetItem *item = 0;
    QStringList colHead;

    //Регистры настройки узла сети
    {
        tableConSettings = new QTableWidget(2, 2, this);
        tableConSettings->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableConSettings->setSelectionMode(QAbstractItemView::SingleSelection);
        colHead.clear();
        colHead << "Регистр" << "Описание";
        tableConSettings->setHorizontalHeaderLabels(colHead);
        item = new QTableWidgetItem("0");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableConSettings->setItem(0, 0, item);
        item = new QTableWidgetItem("1");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableConSettings->setItem(1, 0, item);
        item = new QTableWidgetItem("Младший байт - скорость обмена:"
                                    "\n0 - 1200\n1 - 2400\n2 - 4800\n3 - 9600\n4 - 19200\n5 - 38400\n6 - 57600\n7 - 115200"
                                    "\nСтарший байт - режим:\n0 - ASCII\n1 - RTU");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableConSettings->setItem(0, 1, item);
        item = new QTableWidgetItem("Адрес узла: \n1...247");
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableConSettings->setItem(1, 1, item);

        tableConSettings->verticalHeader()->setMaximumWidth(0);
        tableConSettings->resizeColumnsToContents();
        tableConSettings->resizeRowsToContents();
    }
    //Регистры состояния счетчиков
    {
    tableCountersStatus = new QTableWidget(11, 2, this);
    tableCountersStatus->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableCountersStatus->setSelectionMode(QAbstractItemView::SingleSelection);
    tableCountersStatus->setHorizontalHeaderLabels(colHead);
    for(int i = 0; i < 11; ++i){
    item = new QTableWidgetItem(QString::number(i));
    tableCountersStatus->setItem(i, 0, item);
    }
    item = new QTableWidgetItem("Флаги отказов");
    tableCountersStatus->setItem(0, 1, item);
    item = new QTableWidgetItem("Флаги выхода счета за установленные пределы");
    tableCountersStatus->setItem(1, 1, item);
    item = new QTableWidgetItem("Резерв");
    tableCountersStatus->setItem(2, 1, item);
    for(int i = 1; i <= 4; ++i){
        item = new QTableWidgetItem("Счет гамма-модуля " + QString::number(i));
        tableCountersStatus->setItem(i + 2, 1, item);
        item = new QTableWidgetItem("Счет нейтронного модуля " + QString::number(i));
        tableCountersStatus->setItem(i + 6, 1, item);
    }
    for(int i = 0; i < tableCountersStatus->rowCount(); ++i){
        tableCountersStatus->item(i, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableCountersStatus->item(i, 1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    tableCountersStatus->verticalHeader()->setMaximumWidth(0);
    tableCountersStatus->resizeColumnsToContents();
    tableCountersStatus->resizeRowsToContents();
}
    //Регистры монитора
    {
    tableMonitorRegs = new QTableWidget(64, 4, this);
    tableMonitorRegs->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableMonitorRegs->setSelectionMode(QAbstractItemView::SingleSelection);
    colHead.clear();
    colHead << "Регистр" << "Формат" << "Единицы" << "Описание";
    tableMonitorRegs->setHorizontalHeaderLabels(colHead);
    for(int i = 0; i < 64; ++i){
    //номера регистров
    switch(i){
    case 0: case 4: case 6: case 8: case 11: case 13: case 15:
    case 51: case 54: case 56:
        item = new QTableWidgetItem(QString::number(i) + "-" + QString::number(i + 1));
        break;
    case 1: case 5: case 7: case 9: case 12: case 14: case 16:
    case 52: case 55: case 57:
        item = new QTableWidgetItem("");
        break;
    default:
        item = new QTableWidgetItem(QString::number(i));
    }

    tableMonitorRegs->setItem(i, 0, item);
    //формат регистров
    switch(i){
    case 0: case 4:case 11:case 51: case 54:
        item = new QTableWidgetItem("DWORD");
        break;
    case 6: case 8: case 13: case 15: case 56:
        item = new QTableWidgetItem("FLOAT");
        break;
    case 1: case 5: case 7: case 9: case 12: case 14: case 16:
    case 52: case 55: case 57:
        item = new QTableWidgetItem("");
        break;
    default:
        item = new QTableWidgetItem("WORD");
    }
    tableMonitorRegs->setItem(i, 1, item);
    //единицы измерения
    switch(i){
    case 0: case 43:case 44:case 45:case 46:case 51:
        item = new QTableWidgetItem("Секунды");
        break;
    case 4: case 6: case 11: case 13:  case 26: case 27:
        case 28: case 35: case 36:case 37: case 54:case 56: case 61: case 62:
        item = new QTableWidgetItem("Импульсы");
        break;
    case 25: case 34:
        item = new QTableWidgetItem("Миллисекунды");
        break;
    case 19:
        item = new QTableWidgetItem("мм/с");
        break;
    case 31: case 32: case 33:case 40:case 41:case 42:
        item = new QTableWidgetItem("0.1 сигма");
        break;
    default:
        item = new QTableWidgetItem("");
    }
    tableMonitorRegs->setItem(i, 2, item);
    }
//описание
    item = new QTableWidgetItem("Время, прошедшее с 1.1.1970 00:00:00");
    tableMonitorRegs->setItem(0, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(1, 3, item);
    item = new QTableWidgetItem("Состояние входных сигналов");
    tableMonitorRegs->setItem(2, 3, item);
    item = new QTableWidgetItem("Состояние гамма канала");
    tableMonitorRegs->setItem(3, 3, item);
    item = new QTableWidgetItem("Счет в гамма канале");
    tableMonitorRegs->setItem(4, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(5, 3, item);
    item = new QTableWidgetItem("Оценка фона в гамма канале");
    tableMonitorRegs->setItem(6, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(7, 3, item);
    item = new QTableWidgetItem("Оценка дисперсии в гамма канале");
    tableMonitorRegs->setItem(8, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(9, 3, item);
    item = new QTableWidgetItem("Состояние нейтронного канала");
    tableMonitorRegs->setItem(10, 3, item);
    item = new QTableWidgetItem("Счет в нейтронном канале");
    tableMonitorRegs->setItem(11, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(12, 3, item);
    item = new QTableWidgetItem("Оценка фона в нейтронном канале");
    tableMonitorRegs->setItem(13, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(14, 3, item);
    item = new QTableWidgetItem("Оценка дисперсии в нейтронном канале");
    tableMonitorRegs->setItem(15, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(16, 3, item);
    item = new QTableWidgetItem("Номер последнего объекта");
    tableMonitorRegs->setItem(17, 3, item);
    item = new QTableWidgetItem("Дополнительная информация об объекте");
    tableMonitorRegs->setItem(18, 3, item);
    item = new QTableWidgetItem("Скорость последнего объекта");
    tableMonitorRegs->setItem(19, 3, item);
    item = new QTableWidgetItem("Состояние выходных сигналов");
    tableMonitorRegs->setItem(20, 3, item);
    item = new QTableWidgetItem("Версия программного обеспечения");
    tableMonitorRegs->setItem(21, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество модулей");
    tableMonitorRegs->setItem(22, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество гамма детекторов");
    tableMonitorRegs->setItem(23, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество нейтронных \nдетекторов");
    tableMonitorRegs->setItem(24, 3, item);
    item = new QTableWidgetItem("Экспозиция гамма канала");
    tableMonitorRegs->setItem(25, 3, item);
    item = new QTableWidgetItem("Минимальный счет для гамма детектора");
    tableMonitorRegs->setItem(26, 3, item);
    item = new QTableWidgetItem("Максимальный счет для гамма детектора");
    tableMonitorRegs->setItem(27, 3, item);
    item = new QTableWidgetItem("Уровень перегрузки для гамма детектора");
    tableMonitorRegs->setItem(28, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество интервалов \nдля гамма канала");
    tableMonitorRegs->setItem(29, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество интервалов \n\"взгляда после\" для гамма канала");
    tableMonitorRegs->setItem(30, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 1 для гамма канала");
    tableMonitorRegs->setItem(31, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 2 для гамма канала");
    tableMonitorRegs->setItem(32, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 3 для гамма канала");
    tableMonitorRegs->setItem(33, 3, item);
    item = new QTableWidgetItem("Экспозиция нейтронного канала");
    tableMonitorRegs->setItem(34, 3, item);
    item = new QTableWidgetItem("Минимальный счет для нейтронного детектора");
    tableMonitorRegs->setItem(35, 3, item);
    item = new QTableWidgetItem("Максимальный счет для нейтронного детектора");
    tableMonitorRegs->setItem(36, 3, item);
    item = new QTableWidgetItem("Уровень перегрузки для нейтронного детектора");
    tableMonitorRegs->setItem(37, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество интервалов \nдля нейтронного канала");
    tableMonitorRegs->setItem(38, 3, item);
    item = new QTableWidgetItem("Младший байт: Количество интервалов \n\"взгляда после\" для нейтронного канала");
    tableMonitorRegs->setItem(39, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 1 для нейтронного канала");
    tableMonitorRegs->setItem(40, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 2 для нейтронного канала");
    tableMonitorRegs->setItem(41, 3, item);
    item = new QTableWidgetItem("Уровень тревоги 3 для нейтронного канала");
    tableMonitorRegs->setItem(42, 3, item);
    item = new QTableWidgetItem("Время адаптации к фону");
    tableMonitorRegs->setItem(43, 3, item);
    item = new QTableWidgetItem("Длительность объекта");
    tableMonitorRegs->setItem(44, 3, item);
    item = new QTableWidgetItem("Младший байт - длительность сигнала тревоги"
                                "\nБит 14 - режим архивации всех объектов"
                                "\nБит 15 - режим тестирования сигнала тревоги");
    tableMonitorRegs->setItem(45, 3, item);
    item = new QTableWidgetItem("Младший байт - длительность сигнала \nвидео записи");
    tableMonitorRegs->setItem(46, 3, item);
    item = new QTableWidgetItem("Оценка количества ложных тревог \nдля гамма канала");
    tableMonitorRegs->setItem(47, 3, item);
    item = new QTableWidgetItem("Количество измерений, выполненных \nдля оценки ложных тревог, для гамма канала");
    tableMonitorRegs->setItem(48, 3, item);
    item = new QTableWidgetItem("Оценка количества ложных тревог \nдля нейтронного канала");
    tableMonitorRegs->setItem(49, 3, item);
    item = new QTableWidgetItem("Количество измерений, выполненных для \nоценки ложных тревог, для нейтронного канала");
    tableMonitorRegs->setItem(50, 3, item);
    item = new QTableWidgetItem("Архив: Время");
    tableMonitorRegs->setItem(51, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(52, 3, item);
    item = new QTableWidgetItem("Архив: Событие"
                                "\nМладший байт - код события"
                                "\nСтарший байт - код канала (0-гамма, 1-нейтроны)");
    tableMonitorRegs->setItem(53, 3, item);
    item = new QTableWidgetItem("Архив: Счет");
    tableMonitorRegs->setItem(54, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(55, 3, item);
    item = new QTableWidgetItem("Архив: Фон");
    tableMonitorRegs->setItem(56, 3, item);
    item = new QTableWidgetItem("");
    tableMonitorRegs->setItem(57, 3, item);
    item = new QTableWidgetItem("Архив: Номер объекта");
    tableMonitorRegs->setItem(58, 3, item);
    item = new QTableWidgetItem("Архив: Дополнительная информация об объекте");
    tableMonitorRegs->setItem(59, 3, item);
    item = new QTableWidgetItem("Архив: Управление");
    tableMonitorRegs->setItem(60, 3, item);
    item = new QTableWidgetItem("Порог тревоги 1 для гамма канала");
    tableMonitorRegs->setItem(61, 3, item);
    item = new QTableWidgetItem("Порог тревоги 1 для нейтронного канала");
    tableMonitorRegs->setItem(62, 3, item);
    item = new QTableWidgetItem("Резерв");
    tableMonitorRegs->setItem(63, 3, item);


    for(int i = 0; i < tableMonitorRegs->rowCount(); ++i){
        tableMonitorRegs->item(i, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableMonitorRegs->item(i, 1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableMonitorRegs->item(i, 2)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tableMonitorRegs->item(i, 3)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    tableMonitorRegs->verticalHeader()->setMaximumWidth(0);
    tableMonitorRegs->resizeColumnsToContents();
    tableMonitorRegs->resizeRowsToContents();
    }
    //Регистры доступа к внутренней сети
    {
    tableInnerModbusNetwork = new QTableWidget(5, 2, this);
    tableInnerModbusNetwork->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableInnerModbusNetwork->setSelectionMode(QAbstractItemView::SingleSelection);
    colHead.clear();
    colHead << "Регистр" << "Описание";
    tableInnerModbusNetwork->setHorizontalHeaderLabels(colHead);
    item = new QTableWidgetItem("0");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(0, 0, item);
    item = new QTableWidgetItem("1");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(1, 0, item);
    item = new QTableWidgetItem("2");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(2, 0, item);
    item = new QTableWidgetItem("3");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(3, 0, item);
    item = new QTableWidgetItem("4-35");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(4, 0, item);
    item = new QTableWidgetItem("Адрес модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(0, 1, item);
    item = new QTableWidgetItem("Функция");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(1, 1, item);
    item = new QTableWidgetItem("Номер регистра");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(2, 1, item);
    item = new QTableWidgetItem("Количество регистров");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(3, 1, item);
    item = new QTableWidgetItem("Данные, передаваемые или принимаемые из модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableInnerModbusNetwork->setItem(4, 1, item);

    tableInnerModbusNetwork->verticalHeader()->setMaximumWidth(0);
    tableInnerModbusNetwork->resizeColumnsToContents();
    tableInnerModbusNetwork->resizeRowsToContents();
    }
    //Регистры состояния модулей монитора
    {
    tableModulesStatus = new QTableWidget(9, 2, this);
    tableModulesStatus->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableModulesStatus->setSelectionMode(QAbstractItemView::SingleSelection);
    colHead.clear();
    colHead << "Регистр" << "Описание";
    tableModulesStatus->setHorizontalHeaderLabels(colHead);
    for(int i = 0; i < 6; ++i){
    item = new QTableWidgetItem(QString::number(i));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(i, 0, item);
    }
    item = new QTableWidgetItem("6...");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(6, 0, item);
    item = new QTableWidgetItem("2+(регистр 1)");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(7, 0, item);
    item = new QTableWidgetItem("Количество модулей");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(0, 1, item);
    item = new QTableWidgetItem("Количество регистров, занимаемых данными одного модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(1, 1, item);
    item = new QTableWidgetItem("Адрес первого модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(2, 1, item);
    item = new QTableWidgetItem("Идентификатор блока регистров модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(3, 1, item);
    item = new QTableWidgetItem("Базовый адрес блока регистров модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(4, 1, item);
    item = new QTableWidgetItem("Количество регистров модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(5, 1, item);
    item = new QTableWidgetItem("Регистры, считываемые из модуля \nСтруктура регистров определяется идентификатором блока"
                                "\nрегистров модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(6, 1, item);
    item = new QTableWidgetItem("Адрес второго модуля");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(7, 1, item);
    item = new QTableWidgetItem("...");
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableModulesStatus->setItem(8, 1, item);

    tableModulesStatus->verticalHeader()->setMaximumWidth(0);
    tableModulesStatus->resizeColumnsToContents();
    tableModulesStatus->resizeRowsToContents();
    }
}

