#include "cmdholder.h"

CmdHolder::CmdHolder(quint8 adress, CmdHolder::Function function):
    deviceAdress(adress), func(function), startReg(0), regCount(0), data()
{

}

CmdHolder::CmdHolder(const CmdHolder &obj):
    deviceAdress(obj.deviceAdress), func(obj.func),
    startReg(obj.startReg), regCount(obj.regCount),
    data(obj.data)
{

}

void CmdHolder::processResponse(const QByteArray &message)
{
    //определение канала, по которому получено сообщение
    QDataStream in(message);
    in.device()->seek(2);
    quint16 pi;//идентификатор протокола
    in >> pi;
    quint16 messLen;
    in >> messLen;//последующая длина сообщения
    //если идентификатор протокола pi == 0 и длина сообщения messLen == оставшемуся количеству байт в сообщении
    if((pi == 0) && ((message.length() - 6) == messLen))
    {//сообщение принято по TCP/IP
        ;//никаких действий не требуется
    }else
    {//сообщение по COM
        in.device()->seek(0);//перемещаем курсор в начало
    }
    //считывание PDU
    in >> deviceAdress;//адрес
    quint8 f;
    in >> f;//номер функции
    func = (Function)f;

    data.clear();//стирание существующих данных

    if(f & 0x80)
    {//ошибка
        quint8 errorCode;
        in >> errorCode;//номер ошибки

        QDataStream dataStream(&data, QIODevice::WriteOnly);
        dataStream << errorCode;//запись номера ошибки в блок данных
        return;
    }

    if(func == ReadHoldingRegisters)//ответ на запрос чтения регистров
    {
        quint8 dataLen;
        in >> dataLen;//длина последующих данных
        QDataStream dataStream(&data, QIODevice::WriteOnly);
        quint8 tempByte;
        for(int i = 0; i < dataLen; ++i)
        {
            in >> tempByte;
            dataStream << tempByte;
        }
    }else if(func == PresetMultipleRegisters)//ответ на запрос записи регистров
    {
        in >> startReg;//начальный адрес

        in >> regCount;//длина записанных данных
    }
    //считывание контрольной суммы CRC для проверки сообщения на ошибки при передаче данных через последовательный порт
}

CmdHolder::~CmdHolder()
{

}

quint8 CmdHolder::getDeviceAdress() const
{
    return deviceAdress;
}

void CmdHolder::setDeviceAdress(const quint8 &value)
{
    deviceAdress = value;
}

CmdHolder::Function CmdHolder::getFunc() const
{
    return func;
}

void CmdHolder::setFunc(const Function &value)
{
    func = value;
}

quint16 CmdHolder::getStartReg() const
{
    return startReg;
}

void CmdHolder::setStartReg(const quint16 &value)
{
    startReg = value;
}

quint16 CmdHolder::getRegCount() const
{
    return regCount;
}

void CmdHolder::setRegCount(const quint16 &value)
{
    regCount = value;
}

QByteArray CmdHolder::getData() const
{
    return data;
}

void CmdHolder::setData(const QByteArray &value)
{
    data = value;
}

void CmdHolder::SwapUInt32(quint32 &num)
{
    quint32 temp;
    temp = num >> 16;//получение старшего слова
    num = (num << 16) | temp;//смена слов местами
}

void CmdHolder::SwapFloat(float *num)
{
    quint32 *floatInUint = (quint32 *)num;

    SwapUInt32(*floatInUint);
}

quint16 CmdHolder::calcCRC16()
{
    quint16 crc;
    quint16 tempcrc = 0xFFFF;
    const quint16 poly = 0xA001;//заданный полином

    QByteArray::const_iterator ci;

    for(ci = resultCommand.constBegin(); ci != resultCommand.constEnd(); ++ci)
    {
        tempcrc ^= (quint8)(*ci);

        for(int i = 0; i < 8; ++i)
        {
            bool LSB = tempcrc & 1;
            tempcrc >>= 1;
            if(LSB) tempcrc ^= poly;
        }
    }
    //байты контрольной суммы нужно поменять местами
    crc = tempcrc;
    crc <<= 8;
    tempcrc >>= 8;
    crc |= tempcrc;

    return crc;
}

QByteArray CmdHolder::getQueryCommand(bool com)
{
    resultCommand.clear();

    QDataStream in(&resultCommand, QIODevice::WriteOnly);
    //для передачи по TCP требуется свой заголовок сообщения MBAP
    if(!com){
        in << (quint16)0//Transaction Identifier
           << (quint16)0//Protocol Identifier = 0
           << (quint16)0;//длина последующего сообщения(пока не посчитана)
    }
    in << deviceAdress;//номер устройства(в сети ModBus по SerialPort)
    //далее вставка Protocol Data Unit
    in << (quint8)func;
    //запрос имеет разное строение в зависимости от Функции
    switch(func)
    {
    case ReadHoldingRegisters://чтение регистров
        in << startReg << regCount;
        break;

    case PresetMultipleRegisters://запись регистров
        in << startReg << regCount << (quint8)data.length();
        //данные для записи в регистры устройства
        if(!data.isEmpty())
        {
            QByteArray::const_iterator ci;
            for(ci = data.constBegin(); ci != data.constEnd(); ++ci)
                in << (quint8)(*ci);
        }
        break;
    default:
        //если задана неучтенная функция
        break;
    }

    if(com)//расчет контрольной суммы для COM
        in << calcCRC16();
    else
    {//запись длины сообщения в заголовок MBAP для TCP
        quint16 len = 6;//длина в байтах
        if(func == PresetMultipleRegisters)
        {
            len += (quint16)data.length() + 1;
        }
        in.device()->seek(4);
        in << len;
    }

    return resultCommand;
}
