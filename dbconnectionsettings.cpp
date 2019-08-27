#include "dbconnectionsettings.h"

DBConnectionSettings::DBConnectionSettings(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    CreateWindowElems();//создание окна
    LoadConSettings();//загрузка сохраненных параметров
}

void DBConnectionSettings::accept()
{
    SaveConSettings();

    QDialog::accept();
}

void DBConnectionSettings::SaveConSettings()
{
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    conSettings.setValue("/host", lHost->text());
    conSettings.setValue("/port", sbPort->value());
    conSettings.setValue("/user", lUser->text());
    conSettings.setValue("/pass", lPass->text());
    conSettings.setValue("/dbname", lDBName->text());
    conSettings.endGroup();
}

void DBConnectionSettings::LoadConSettings()
{
    QSettings conSettings;
    conSettings.beginGroup("/DataBase");
    lHost->setText(conSettings.value("/host", "localhost").toString());
    sbPort->setValue(conSettings.value("/port", 0).toInt());
    lUser->setText(conSettings.value("/user", "username").toString());
    lPass->setText(conSettings.value("/pass", "").toString());
    lDBName->setText(conSettings.value("/dbname", "db name").toString());
    conSettings.endGroup();
}

void DBConnectionSettings::CreateWindowElems()
{
    setWindowTitle("Параметры подключения базы данных");

    QLabel *lbHost = new QLabel("Адрес:",this);
    lHost = new QLineEdit(this);

    QLabel *lbPort = new QLabel("Порт:",this);
    sbPort = new QSpinBox(this);
    sbPort->setRange(0, 999999);//диапазон портов
    sbPort->setSingleStep(1);

    QLabel *lbUser = new QLabel("Имя пользователя:",this);
    lUser = new QLineEdit(this);

    QLabel *lbPass = new QLabel("Пароль:",this);
    lPass = new QLineEdit(this);
    lPass->setEchoMode(QLineEdit::Password);//скрытие вводимого текста

    QLabel *lbDBName = new QLabel("Имя базы данных:",this);
    lDBName = new QLineEdit(this);

    //кнопки
    QPushButton *btnOK = new QPushButton("Ок");
    QPushButton *btnCancel = new QPushButton("Отмена");
    QHBoxLayout *lButtons = new QHBoxLayout();
    lButtons->addWidget(btnOK);
    lButtons->addWidget(btnCancel);
    connect(btnOK, SIGNAL(clicked()), SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

    //общий лэйаут
    QFormLayout *layout = new QFormLayout();
    layout->addRow(lbHost, lHost);
    layout->addRow(lbPort, sbPort);
    layout->addRow(lbUser, lUser);
    layout->addRow(lbPass, lPass);
    layout->addRow(lbDBName, lDBName);
    layout->addRow(lButtons);

    setLayout(layout);
}

