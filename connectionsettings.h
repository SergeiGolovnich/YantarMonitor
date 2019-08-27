#ifndef CONNECTIONSETTINGS_H
#define CONNECTIONSETTINGS_H
#include <QDialog>
#include <QtWidgets>
#include <QtSerialPort/QtSerialPort>
#include "connectionmanager.h"

//окно настройки соединения
class ConnectionSettings: public QDialog
{
    Q_OBJECT
public:
    ConnectionSettings(QWidget *parent = 0);

private:
    ConnectionManager *connection;

    QSettings conSettings;
    QComboBox *cbMethod;
    //элементы COM-порта
    QComboBox *cbCOMPort;
    QComboBox *cbCOMBaudRate;
    QComboBox *cbDataBits;
    QComboBox *cbParity;
    QComboBox *cbStopBits;
    //элементы TCP
    QLineEdit *lIP;
    QSpinBox *sbPort;
    QSpinBox *sbTimeOut;

    void findCOMPorts();//поиск COM портов
    void readConSettings();//считывание параметров
    void writeConSettings();//запись параметров соединения
    void createWindow();//создание элементов окна
protected:
    virtual void accept();//обработка кнопки "ок"
signals:
    void connectionSettingsChanged();//сигнал изменения парметров соединения
};

#endif // CONNECTIONSETTINGS_H
