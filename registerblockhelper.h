#ifndef REGISTERBLOCKHELPER_H
#define REGISTERBLOCKHELPER_H

#include <QWidget>
#include <QtWidgets>
#include "cmdholder.h"

//окно справки по блокам регистров
class RegisterBlockHelper : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterBlockHelper(QWidget *parent = 0);

signals:
    void registerClicked(int);//изменен выбранный регистер в окне справки
    void registerDoubleClicked(int, bool);//сигнал окну изменения значения регистра для выставления выбранного регистра
public slots:
    void changedSelectedRegister(int);//изменен текущий регистер в окне изменения значения регистра
private slots:
    void doubleClickedRegister(int, int);//двойной клик на регистр
private:
    void createHelpWindow();//создание окна
    void createTables();//создание таблиц с описанием регистров

    QTabWidget *mainTabs;
    QTableWidget *tableConSettings;
    QTableWidget *tableCountersStatus;
    QTableWidget *tableMonitorRegs;
    QTableWidget *tableInnerModbusNetwork;
    QTableWidget *tableModulesStatus;
};

#endif // REGISTERBLOCKHELPER_H
