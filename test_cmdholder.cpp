#include "test_cmdholder.h"

void Test_CmdHolder::getQueryCommand()
{
    //использование предопределенных столбцов-переменных с данными
    QFETCH(QByteArray, actual);
    QFETCH(QByteArray, expected);

    QCOMPARE(actual, expected);
}

void Test_CmdHolder::getQueryCommand_data()
{
    //задание столбцов-переменных для данного теста
    QTest::addColumn<QByteArray>("actual");
    QTest::addColumn<QByteArray>("expected");

    CmdHolder cmd;
    cmd.setStartReg(0);
    cmd.setRegCount(1);

    //COM
    QByteArray resultCom;
    QDataStream in(&resultCom, QIODevice::WriteOnly);
    in << (quint8)0 << (quint8)3 << (quint16)0
       << (quint16)1 << (quint8)133 << (quint8)219;
    //добавление значений в таблицу
    QTest::newRow("com") << cmd.getQueryCommand() << resultCom;

    //TCP
    QByteArray resultTCP;
    QDataStream inTCP(&resultTCP, QIODevice::WriteOnly);
    inTCP << (quint16)0 << (quint16)0 << (quint16)0
          << (quint8)0 << (quint8)3 << (quint16)0
          << (quint16)1;
    inTCP.device()->seek(4);
    inTCP << (quint16)6;

    QTest::newRow("tcp") << cmd.getQueryCommand(false) << resultTCP;
}

void Test_CmdHolder::SwapUInt32()
{
    quint32 testValue = 0x75BCD15;
    CmdHolder::SwapUInt32(testValue);
    quint32 swapedValue = 0xCD15075B;

    QCOMPARE(testValue, swapedValue);

    //проверка циклического swap
    {
        quint32 testValue = 156548;
        quint32 valueBeforeSwap = testValue;

        CmdHolder::SwapUInt32(testValue);
        CmdHolder::SwapUInt32(testValue);

        QCOMPARE(testValue, valueBeforeSwap);
    }
}

void Test_CmdHolder::SwapFloat()
{
    {//проверка функции swapFloat
        float testFloatValue = 1;
        CmdHolder::SwapFloat(&testFloatValue);

        quint32 swapedUint = 0x00003F80;
        float *swapedFloat = (float *)(&swapedUint);

        QCOMPARE(testFloatValue, *swapedFloat);
    }

    {//проверка циклического swapFloat
        float testFloatValue = 123;
        float testFloatBeforeSwap = testFloatValue;

        CmdHolder::SwapFloat(&testFloatValue);
        CmdHolder::SwapFloat(&testFloatValue);

        QCOMPARE(testFloatValue, testFloatBeforeSwap);
    }
}

void Test_CmdHolder::TestGetSetFuncs()
{
    CmdHolder cmd;//команда по-умолчанию

    //проверка значений по-умолчанию
    QCOMPARE((int)cmd.getDeviceAdress(), 0);
    QCOMPARE((int)cmd.getFunc(), 3);

    cmd.setDeviceAdress(1);
    QCOMPARE((int)cmd.getDeviceAdress(), 1);

    cmd.setFunc(CmdHolder::PresetMultipleRegisters);
    QCOMPARE((int)cmd.getFunc(), 16);

    cmd.setStartReg(12);
    QCOMPARE((int)cmd.getStartReg(), 12);

    cmd.setRegCount(25);
    QCOMPARE((int)cmd.getRegCount(), 25);

    QByteArray testArray("test string");
    cmd.setData(testArray);
    QCOMPARE(cmd.getData(), testArray);
}

void Test_CmdHolder::ProcessResponse()
{
    QFETCH(QByteArray, responseByteArray);
    QFETCH(QByteArray, expectedData);
    QFETCH(int, expectedAddr);
    QFETCH(int, expectedFunc);

    CmdHolder cmd;
    cmd.processResponse(responseByteArray);
    QCOMPARE(cmd.getData(), expectedData);
    QCOMPARE((int)cmd.getDeviceAdress(), expectedAddr);
    QCOMPARE((int)cmd.getFunc(), expectedFunc);
}

void Test_CmdHolder::ProcessResponse_data()
{
    QTest::addColumn<QByteArray>("responseByteArray");
    QTest::addColumn<int>("expectedAddr");
    QTest::addColumn<int>("expectedFunc");
    QTest::addColumn<QByteArray>("expectedData");

    {
        //Нормальный ответ по COM
        QByteArray comNorm;
        QDataStream in(&comNorm, QIODevice::WriteOnly);
        in << (quint8)0 << (quint8)3 << (quint8)2
           << (quint16)0x4D41 << (quint8)219;

        QTest::newRow("COM_Normal") << comNorm
                                    << 0 << 3 << QByteArray("MA");
    }
    {
        //Нормальный ответ по TCP
        QByteArray tcpNorm;
        QDataStream in(&tcpNorm, QIODevice::WriteOnly);
        in << (quint16)0 << (quint16)0 << (quint16)5
           << (quint8)0 << (quint8)3 << (quint8)2
           << (quint16)0x4D41;

        QTest::newRow("TCP_Normal") << tcpNorm
                                    << 0 << 3 << QByteArray("MA");
    }
    {
        //Ошибочный ответ по TCP
        QByteArray tcpErr;
        QDataStream in(&tcpErr, QIODevice::WriteOnly);
        in << (quint16)0 << (quint16)0 << (quint16)3
           << (quint8)0 << (quint8)(3 + 0x80) << (quint8)25;

        QTest::newRow("TCP_Err") << tcpErr
                                    << 0 << (3 + 0x80)
                                    << QByteArray("\x19");
    }
}

