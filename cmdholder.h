#ifndef CMDHOLDER
#define CMDHOLDER
#include <QtCore>

//класс для хранения команд, отправляемых мониторам
class CmdHolder
{
public:
    //функции, поддерживаемые мониторами
    enum Function{
        ReadHoldingRegisters = 3,//чтение нескольких регистров
        PresetMultipleRegisters = 16,//запись нескольких регистров
        Diagnostics = 8,//диагностические функции
        ReadDeviceIdentification = 43//чтение идентификатора устройства
    };

    //базовые адреса блоков регистров
    enum RegBlockAdresses{
        ConSettings = 50,//регистры настройки узла сети
        CountersStatus = 89,//регистры состояния счетчиков
        MonitorRegs = 100,//регистры монитора
        InnerModbusNetwork = 200,//регистры доступа к внутренней сети
        ModulesStatus = 300//регистры состояния модулей монитора
    };

    CmdHolder(quint8 adress = 0, Function function = ReadHoldingRegisters);//конструктор
    CmdHolder(const CmdHolder &obj);//конструктор копирования
    void processResponse(const QByteArray &message);//преобразование ответа от устройства в объект CmdHolder
    ~CmdHolder();//деструктор
    QByteArray getQueryCommand(bool com = true);//создание запроса для передачи по выбранному протоколу
    //адрес монитора
    quint8 getDeviceAdress() const;
    void setDeviceAdress(const quint8 &value);
    //функция
    Function getFunc() const;
    void setFunc(const Function &value);
    //начальный регистр
    quint16 getStartReg() const;
    void setStartReg(const quint16 &value);
    //количество регистров
    quint16 getRegCount() const;
    void setRegCount(const quint16 &value);
    //данные
    QByteArray getData() const;
    void setData(const QByteArray &value);

    //поменять местами 16-битные слова
    static void SwapUInt32(quint32 &num);
    static void SwapFloat(float *num);

private:
    quint8 deviceAdress;//адрес устройства
    Function func;//номер функции
    quint16 startReg;//номер первого регистра
    quint16 regCount;//количество регистров
    QByteArray data;//данные регистров

    QByteArray resultCommand;//массив байтов для передачи

    quint16 calcCRC16();//подсчет контрольной суммы сообщения
};

#endif // CMDHOLDER

