#ifndef DBCONNECTIONSETTINGS_H
#define DBCONNECTIONSETTINGS_H

#include <QObject>
#include <QtWidgets>

//окно настройки соединения с базой данных
class DBConnectionSettings : public QDialog
{
public:
    DBConnectionSettings(QWidget *parent = nullptr);
protected:
    virtual void accept();//обработка кнопки "ок"
private:
    void SaveConSettings();//сохранить настройки
    void LoadConSettings();//загрузить настройки
    void CreateWindowElems();//создать структуру окна

    QSpinBox *sbPort;//порт
    QLineEdit *lHost;//адрес бд
    QLineEdit *lUser;//пользователь
    QLineEdit *lPass;//пароль
    QLineEdit *lDBName;//имя базы данных
};

#endif // DBCONNECTIONSETTINGS_H
