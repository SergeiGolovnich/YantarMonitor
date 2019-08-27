#include "graphic.h"

Graphic::Graphic(int width, int height, QWidget *parent) : QWidget(parent),
    ValuesCount(30), MaxValue(0), MinValue(0), TimeDiff(0)
{
    //установка размеров поля для вывода изображения
    resize(width, height);
}

void Graphic::checkForOverFlow()
{
    if(valuesList.size() > (int)ValuesCount)
    {
        Values first = valuesList.takeFirst();

        if(first.seconds != 0)//если точка не ошибочна
        {
            if((first.value >= MaxValue) || ((uint)first.fon >= MaxValue))
            {
                culcMaxValue();//пересчет максимального значения
            }
            if(((uint)first.fon <= MinValue) || ((first.value <= MinValue)))
            {
                culcMinValue();//пересчет минимального значения
            }
        }
    }
}

void Graphic::AddValues(quint32 seconds, quint32 value, float fon, quint16 level)
{
    //не добавляем первой точкой ошибочное значение
    if((seconds == 0) && (valuesList.isEmpty())) return;

    if(valuesList.isEmpty()) {
        MinValue = fon > value ? value : fon;
        MaxValue = value > fon ? value : fon;
    }

    //оставляем только два младших байта (показывают уровень тревоги)
    level <<= 14;
    level >>= 14;

    valuesList.append(Values(fon, value, seconds, level));//добавление новой точки

    if(seconds != 0)
    {//если последняя добавленная точка не ошибочна, то
        //обновление максимума и минимума на графике
        //величина фона может превышать значение счета
        if(fon > value)
        {
            if(MaxValue < fon) MaxValue = fon;
            if(MinValue > value) MinValue = value;
        }else
        {
            if(MaxValue < value) MaxValue = value;
            if(MinValue > fon) MinValue = fon;
        }
    }

    //удаляем самое старое значение, если слишком много значений
    checkForOverFlow();

    update();//перерисование графика (событие paintEvent)
}

void Graphic::Clear()
{
    valuesList.clear();//удаление всех точек на графике
    //обнуление вспомогательных переменных
    MaxValue = 0;
    MinValue = 0;
    TimeDiff = 0;
}

void Graphic::paintEvent(QPaintEvent *)
{
    QPixmap pixmap(width(),height());//объект для рисования в памяти
    pixmap.fill();//заливка фона белым цветом

    QPainter pixPainter(&pixmap);//объект для рисования на pixMap
    pixPainter.setRenderHint(QPainter::Antialiasing);//включение сглаживания (убирание лесенок на прямых линиях)

    pixPainter.setPen(Qt::black);//черные контуры фигур
    pixPainter.setBrush(Qt::NoBrush);//без заливки фигур
    static QFont font("Times", 9, QFont::Normal);//шрифт
    pixPainter.setFont(font);//установка шрифта

    //рисование координатной плоскости
    QPoint coordStart(5, height() - 12);//точка начала координат

    pixPainter.drawLine(coordStart.x(), 0, coordStart.x(), coordStart.y());//вертикальныя линия
    pixPainter.drawLine(coordStart.x(), coordStart.y(), width(), coordStart.y());//горизонтальная линия

    //пространственные коэффициенты
    float koefY = (float)(coordStart.y() - 10) / (float)(MaxValue - MinValue);
    float koefX = (float)(width() - coordStart.x() - 25) / (float)ValuesCount;

    //временная разница между самым новым и самым старым измерением на графике
    if(!valuesList.isEmpty())
    {
        if((valuesList.first().seconds != 0) && (valuesList.last().seconds != 0))
        {
            TimeDiff = valuesList.first().seconds - valuesList.last().seconds;
            pixPainter.drawText(0, height(), QString::number(TimeDiff) + " сек.");
        }
    }

    //рисование графиков
    if(!valuesList.isEmpty())
    {
        QLinkedList<Values>::const_iterator i;
        int pointNumber = 0;//номер точки на графике
        qreal x, yV, yF, xPrev = 0, yVPrev = 0, yFPrev = 0;//координаты точек на графике
        for (i = valuesList.constBegin(); i != valuesList.constEnd(); ++i)
        {
            if((*i).seconds != 0)
            {//если значения не ошибочны
                //расчет координат точек на графике
                x = coordStart.x() + pointNumber * koefX;
                yV = coordStart.y() - ((int)(*i).value - (int)MinValue) * koefY;
                yF = coordStart.y() - ((int)(*i).fon - (int)MinValue) * koefY;

#ifdef QT_DEBUG
                if((yV < 0) || (yF < 0))
                {
                    qDebug() << "yV = "<< yV<<" | yF = "<< yF;
                    qDebug() << "V = "<< (*i).value<<" | F = "<< (*i).fon;
                }
#endif

                if(i != valuesList.constBegin())
                {
                    if((*(i - 1)).seconds != 0)
                    {//если предыдущая точка не ошибочна, рисуем линию
                        //счет
                        pixPainter.setPen(Qt::black);
                        pixPainter.drawLine(xPrev, yVPrev, x, yV);
                        //фон
                        pixPainter.setPen(Qt::gray);
                        pixPainter.drawLine(xPrev, yFPrev, x, yF);
                    }else
                    {//рисуем точку
                        //счет
                        pixPainter.setPen(Qt::black);
                        pixPainter.drawLine(x - 2, yV, x, yV);
                        //фон
                        pixPainter.setPen(Qt::gray);
                        pixPainter.drawLine(x - 2, yF, x, yF);
                    }
                }

                //отображение уровня тревоги
                if((*i).level)
                {
                    pixPainter.setPen(QPen(QBrush(Qt::red), 1, Qt::DotLine));//установка красного цвета для отображения тревоги
                    //уровень тревоги
                    pixPainter.drawText(x, yV - 1,
                                        QString::number((*i).level));
                    //вертикальная линия
                    if((yV + 5) < coordStart.y())//рисуется, если есть место
                        pixPainter.drawLine(x, yV + 5,
                                            x, coordStart.y());
                }

                xPrev = x;
                yVPrev = yV;
                yFPrev = yF;
            }

            pointNumber++;
        }


        //вывод значения последней не ошибочной точки
        pointNumber--;
        QLinkedList<Values>::const_iterator j;
        for (j = valuesList.constEnd() - 1; j != valuesList.constBegin(); --j)
        {
            if((*j).seconds != 0)
            {
                //расчет координат текста на графике
                x = coordStart.x() + pointNumber * koefX;
                yV = coordStart.y() - ((*j).value - MinValue) * koefY;
                yF = coordStart.y() - ((*j).fon - MinValue) * koefY;
                //значение счета
                pixPainter.setPen(Qt::black);
                pixPainter.drawText(x + 3, yV + 12,
                                    QString::number((*j).value));
                //значение фона
                pixPainter.setPen(Qt::gray);
                pixPainter.drawText(x + 3, yF,
                                    QString::number((*j).fon));
                break;
            }
            pointNumber--;
        }

    }

    //максимальное значение счета на графике
    pixPainter.setPen(Qt::black);
    pixPainter.drawText(coordStart.x() + 1, 10, QString::number(MaxValue));
    //минимальное значение фона на графике
    pixPainter.drawText(coordStart.x() + 1, coordStart.y() - 1, QString::number(MinValue));

    QPainter painter(this);//объект для рисования на форме
    painter.drawPixmap(0, 0, pixmap);//вывод графика на виджет
}

void Graphic::culcMaxValue()
{
    MaxValue = 0;
    QLinkedList<Values>::const_iterator i;
    for (i = valuesList.constBegin(); i != valuesList.constEnd(); ++i)
    {
        if((*i).seconds == 0) continue;//ошибочные точки игнорируются
        quint32 maxVal = ((quint32)(*i).fon) < (*i).value ? (*i).value : (quint32)(*i).fon;
        if(MaxValue < maxVal) MaxValue = maxVal;
    }
}

void Graphic::culcMinValue()
{
    MinValue = MaxValue;
    QLinkedList<Values>::const_iterator i;
    for (i = valuesList.constBegin(); i != valuesList.constEnd(); ++i)
    {
        if((*i).seconds == 0) continue;//ошибочные точки игнорируются
        quint32 minVal = ((quint32)(*i).fon) > ((*i).value) ? (*i).value : (quint32)(*i).fon;
        if(MinValue > minVal) MinValue = minVal;
    }
}

int Graphic::getValuesCount() const
{
    return ValuesCount;
}

void Graphic::setValuesCount(int value)
{
    int count = valuesList.size() - value;//количество лишних значений на графике
    //удаляем самые старые значения на графике
    for(int i = 0; i < count; ++i) valuesList.removeFirst();

    ValuesCount = value;
}
