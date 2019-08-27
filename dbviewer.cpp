#include "dbviewer.h"

DBViewer::DBViewer(QWidget *parent) : QWidget(parent)
{
    createWindow();
    setQueryModel();
}

void DBViewer::createWindow()
{
    setWindowTitle("Просмотр базы данных");
    this->setMinimumHeight(200);
    this->setMinimumWidth(700);

    btnUpdate = new QPushButton("Обновить");
    connect(btnUpdate, SIGNAL(clicked(bool)), this, SLOT(setQueryModel()));

    QPushButton *btnSaveToFile = new QPushButton("Сохранить...");
    connect(btnSaveToFile, SIGNAL(clicked(bool)), this, SLOT(saveDBToFile()));

    sbRowsLimit = new QSpinBox();
    sbRowsLimit->setRange(1, qPow(2,31) - 1);//максимальное количество отображаемых строк
    //загрузка количества отображаемых строк
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    sbRowsLimit->setValue(conSettings.value("/rowlimit", 10000).toInt());
    conSettings.endGroup();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(&view);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(new QLabel("Количество отображаемых строк:"), 0, Qt::AlignLeft);
    bottomLayout->addWidget(sbRowsLimit, 0, Qt::AlignLeft);
    bottomLayout->addWidget(btnUpdate, 0, Qt::AlignLeft);
    bottomLayout->addWidget(btnSaveToFile, 1, Qt::AlignRight);

    layout->addLayout(bottomLayout);
    this->setLayout(layout);
}

void DBViewer::setQueryModel()
{
    //выключение кнопки
    btnUpdate->setEnabled(false);
    //считывание сохраненного состояния логирования
    QSettings Settings;
    Settings.beginGroup("/DataBase");
    bool isLoggingActive = Settings.value("/isloggingactive", true).toBool();
    Settings.endGroup();

    ResponseLogger *logger = ResponseLogger::GetResponseLogger();//Получение соединения с базой
    model.setQuery("SELECT * FROM monitorsresponses ORDER BY time DESC LIMIT " +
                   QString::number(sbRowsLimit->value()) + ";", logger->db);//запрос на получение данных таблицы

    if(model.lastError().isValid() || !logger->db.isOpen()){
        qDebug() << "Ошибка запроса данных: " << model.lastError();

        btnUpdate->setEnabled(true);
        return;
    }

    //выставление названий столбцов
    model.setHeaderData(0, Qt::Horizontal, "Дата/Время");
    model.setHeaderData(1, Qt::Horizontal, "Адрес монитора");
    model.setHeaderData(2, Qt::Horizontal, "Номер функции");
    model.setHeaderData(3, Qt::Horizontal, "Начальный регистр");
    model.setHeaderData(4, Qt::Horizontal, "Количество регистров");
    model.setHeaderData(5, Qt::Horizontal, "Данные (HEX)");

    view.setModel(&model);//вывод результата в таблицу
    view.resizeColumnsToContents();//подгонка ширины столбцов таблицы под данные

    if(!isLoggingActive)//закрываем соединение с базой, если оно не используется
        ResponseLogger::CloseConnection();

    //включение кнопки
    btnUpdate->setEnabled(true);
}
void DBViewer::saveDBToFile()
{
    int colCount = model.columnCount();//количество колонок
    int rowCount = model.rowCount();//количество строк

    if(!rowCount)
    {
        QMessageBox::information(this, "Ошибка", "Нет записей для сохранения.");
        return;
    }

    QString defaultName = "/База Данных Мониторов Янтарь [" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "]";
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить", QDir::homePath() + defaultName);
    if (fileName.isEmpty())
        return;

    QFile file(fileName);//файл для записи
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream out(&file);
    QString splitter = ";";//разделитель для csv файла

    //вывод названия столбцов
    QString rowNum = "Строка №";
    out << rowNum;
    out << splitter;
    for(int i = 0; i < colCount; ++i)
    {
        out << model.headerData(i, Qt::Horizontal).toString();//вывод заголовка столбца
        if(i != colCount - 1)//запрет вывода разделителя в конце строки
            out << splitter;
    }
    out << '\n';

    //вывод строк
    if(rowCount < 1)
        return;
    model.query().seek(0);//выставление указателя на первую запись
    for(int row = 0; row < rowCount; ++row)//перебор строк
    {
        out << QString::number(row + 1);//вывод номера строки
        out << splitter;
        for(int col = 0; col < colCount; ++col)//перебор столбцов
        {
            if(!col)
            {
                out << model.query().value(col).toDateTime().toString("dd.MM.yyyy hh:mm:ss:z");//вывод времени из таблицы
            }else
            {
                out << model.query().value(col).toString();//вывод ячейки из таблицы
            }
            if(col != colCount - 1)//не выводить разделитель последним
                out << splitter;
        }

        if(row != rowCount - 1)//чтобы не выводилась пустая строка в конце файла
            out << "\n";

        model.query().next();//переход указателя на следующую запись
    }
}

void DBViewer::closeEvent(QCloseEvent *)
{
    //сохранение количества отображаемых строк
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    conSettings.setValue("/rowlimit", sbRowsLimit->value());
    conSettings.endGroup();
}
