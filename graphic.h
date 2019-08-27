#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <QWidget>
#include <QtWidgets>

//класс отвечает за отрисовку графика
class Graphic : public QWidget
{
    Q_OBJECT
public:
    explicit Graphic(int width, int height, QWidget *parent = 0);
    void AddValues(quint32 seconds, quint32 value, float fon, quint16 level);//добавить к графику значения фона, счет, и время в секундах
    void Clear();//очистить график
    int getValuesCount() const;//получение максимального количества отображаемых значений
    void setValuesCount(int value);//задание максимального количества отображаемых значений
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;//процедура отрисовывания графика
private:
    //структура для хранения значений счета, фона в конкретный момент времени
    struct Values
    {
        Values(float fon, quint32 value, quint32 seconds, quint16 level)
        {
            this->fon = fon;
            this->value = value;
            this->seconds = seconds;
            this->level = level;
        }
        float fon;//фон
        quint32 value;//счет
        quint32 seconds;//время монитора в секундах
        quint16 level;//уровень тревоги
    };

    void culcMaxValue();//расчет максимального значения на графике
    void culcMinValue();//расчет минимального значения на графике
    void checkForOverFlow();//проверка на превышение максимального количества значений на графике

    QLinkedList<Values> valuesList;//список, хранящий значения для отображения на графике
    uint ValuesCount;//максимальное количество значений
    uint MaxValue;//максимальное значение на графике
    uint MinValue;//минимальное значение на графике
    int TimeDiff;//временная разница на графике

};

#endif // GRAPHIC_H
