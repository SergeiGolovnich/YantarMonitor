#ifndef TEST_CMDHOLDER_H
#define TEST_CMDHOLDER_H

#include <QObject>
#include <QTest>
#include "cmdholder.h"

class Test_CmdHolder : public QObject
{
    Q_OBJECT
private slots:
    void getQueryCommand();//проверка правильности составления команды для отправки
    void getQueryCommand_data();//данные

    void SwapUInt32();//проверка смены 16-битных слов местами в Int32
    void SwapFloat();//проверка смены 16-битных слов в Float
    void TestGetSetFuncs();//проверка выставления параметров запроса

    void ProcessResponse();//проверка правильности обработки ответа на запрос
    void ProcessResponse_data();
};

#endif // TEST_CMDHOLDER_H
