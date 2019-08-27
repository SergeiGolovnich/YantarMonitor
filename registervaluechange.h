#ifndef REGISTERVALUECHANGE_H
#define REGISTERVALUECHANGE_H

#include <QtWidgets>
#include <QWidget>
#include "cmdholder.h"
#include "connectionmanager.h"
#include "registerblockhelper.h"

//окно просмотра/изменения регистров монитора
class RegisterValueChange : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterValueChange(int monNum, QWidget *parent = 0);

private:
    void createWidget();//построение графики
    QString getExeptionDescription(quint8 err);//расшифровка кода ошибки
    void setRegBlockAndRegNumber(int regNumber);//выставление блока и номера регистра

    QObject *parentObj;//ссылка на родительский виджет
    ConnectionManager *connection;//соединение с мониторами
    int monitorNumber;//номер монитора
    QSpinBox *sbRegNumber;//номер регистра в блоке
    QComboBox *cbRegBlock;//выбор блока регистров
    QLineEdit *lRegValue;//значение регистра
    QLineEdit *lRegValueBinary;//значение регистра в двоичном формате
    QCheckBox *cb2regs;//значение хранится в 2-х регистрах
    QCheckBox *cbFloat;//вещественное значение хранится в 2-х регистрах
    RegisterBlockHelper *helperWindow;//окно справки по регистрам

    int getBlockAddress();
    
signals:
    void sendRequest(CmdHolder);//отправить запрос
    void regSelectionChanged(int);//сигнал изменения выбранного регистра для справки

public slots:
    void registerValueRecieved(CmdHolder &);//получение значения регистра
    void registerSelectionChanged(int, bool);//изменен регистр из справки

private slots:
    void readValueButton();//кнопка Чтение
    void writeValueButton();//кнопка Запись
    void openRegHelperWindow();//открыть окно справки по блокам регистров
    void regBlockChanged(int);//изменен блок регистров
    void regValueChanged(QString);//изменено значение регистра
    void regNumberChanged(int);//изменен номер регистра

protected:
    virtual void closeEvent(QCloseEvent *);//событие закрытия окна
};

#endif // REGISTERVALUECHANGE_H
