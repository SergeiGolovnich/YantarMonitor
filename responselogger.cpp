#include "responselogger.h"

ResponseLogger *ResponseLogger::logger;
QThread ResponseLogger::thread;

ResponseLogger::~ResponseLogger()
{
    WriteResponsesToDB();
    db.close();
    thread.exit();
}

ResponseLogger *ResponseLogger::GetResponseLogger()
{
    if(!logger)
    {
        thread.start(QThread::NormalPriority);
        logger = new ResponseLogger();
        logger->CreateConnection();
    }else
    {
        if(!logger->db.isOpen())
            logger->CreateConnection();
    }

    return logger;
}

void ResponseLogger::CloseConnection()
{
    if(logger)
    {
        QTimer::singleShot(0, logger, SLOT(SaveAndCloseDB()));
    }
}

ResponseLogger::ResponseLogger(QObject *parent) : QObject(parent)
{
    //перемещаем логер в отдельный поток
    this->moveToThread(&thread);

    timer = new QTimer();//таймер для отправки сообщений в базу
    timer->setSingleShot(true);//вызывается единожды
    timer->setInterval(5000);//интервал отправки ответов в базу

    connect(timer, SIGNAL(timeout()), SLOT(WriteResponsesToDB()));

    timer->moveToThread(&thread);
    timer->setParent(this);

    db = QSqlDatabase::addDatabase("QPSQL");//использование базы PostgreSQL
}

QString ResponseLogger::getInsertStringFromCmdHolder(CmdHolder ch)
{
    QString ret = "INSERT INTO monitorsresponses (time, device, func, reg_start, reg_count, data) "
                  "VALUES (now(), %1, %2, %3, %4, '%5');";

    return ret.arg(QString::number(ch.getDeviceAdress()), QString::number(ch.getFunc()),
                   QString::number(ch.getStartReg()), QString::number(ch.getRegCount()),
                   ch.getData().toHex());//данные хранятся в HEX формате для лучшей переносимости
}

bool ResponseLogger::CreateConnection()
{
    //загрузка сохраненных параметров подключения
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    db.setDatabaseName(conSettings.value("/dbname", "db name").toString());
    db.setHostName(conSettings.value("/host", "localhost").toString());
    db.setPort(conSettings.value("/port", 0).toInt());
    db.setUserName(conSettings.value("/user", "username").toString());
    db.setPassword(conSettings.value("/pass", "").toString());
    conSettings.endGroup();

    if(!db.open())
    {
        QMessageBox::warning(nullptr, "Ошибка подключения к базе данных",
                             db.lastError().text(),
                             QMessageBox::Ok, QMessageBox::NoButton);
        emit ConnectionOpened(false);//выключение логирования
        return 0;
    }

#ifdef QT_DEBUG
    //список существующих таблиц в бд
    QStringList lst = db.tables();
    foreach(QString str, lst)
    {
        qDebug() << "Table: " << str;
        QSqlQuery query;
        if(query.exec("SELECT COUNT(*) FROM " + str + ";"))
        {
            query.next();//переход на строку с результатом
            qDebug() << "rows: " << query.value(0).toInt();
        }
    }
#endif

    if(createTable())
    {
        return 1;
    }
    else
    {//если не удалось создать таблицу
        db.close();
        emit ConnectionOpened(false);
        return 0;
    }
}

bool ResponseLogger::createTable()
{
    QStringList tablesList = db.tables();
    QString tableName = "monitorsresponses";//название таблицы
    if(!tablesList.contains(tableName))
    {
        QString createTableStr = "CREATE TABLE " +
                tableName + " ("
                            "time TIMESTAMP PRIMARY KEY,"
                            "device SMALLINT,"
                            "func INTEGER,"
                            "reg_start INTEGER,"
                            "reg_count INTEGER,"
                            "data TEXT"
                            ");";
        QSqlQuery query;
        if(!query.exec(createTableStr))
        {
            qDebug() << "Не удалось создать таблицу " + tableName;
            QMessageBox::warning(NULL, "Ошибка создания таблицы",
                                 query.lastError().text(),
                                 QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
        }
    }

    return 1;
}

void ResponseLogger::SaveResponse(CmdHolder ch)
{
    responsesQueue.enqueue(ch);//помещение ответа в конец очереди

    if(!timer->isActive() && db.isOpen()) timer->start();//отложенный асинхронный вызов метода для отправки данных в базу
}

void ResponseLogger::ResetConnection()
{
    if(logger)
    {
        //закрытие старого соединения
        if(logger->db.isOpen())
            logger->db.close();

        logger->CreateConnection();
    }
}

void ResponseLogger::WriteResponsesToDB()
{
    QSqlQuery query;
    QString str;

    while(!responsesQueue.empty())
    {
        if(!db.isOpen())
        {
            emit ConnectionOpened(false);//выключение логирования
            timer->stop();//остановка записи в базу
            break;
        }

        str = getInsertStringFromCmdHolder(responsesQueue.dequeue());
        if(!query.exec(str))//выполнение запроса на вставку данных
        {
            qDebug() << "Ошибка при выполнении запроса: " + str + " - " + query.lastError().text();
        }
    }
}

void ResponseLogger::SaveAndCloseDB()
{
    if(db.isOpen())
        WriteResponsesToDB();

    db.close();
}

