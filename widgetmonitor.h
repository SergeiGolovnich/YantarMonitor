#ifndef WIDGETMONITOR_H
#define WIDGETMONITOR_H

#include <QObject>
#include <QtWidgets>
#include <QFrame>
#include "monitorhistory.h"
#include "connectionmanager.h"
#include "registervaluechange.h"
#include "graphic.h"
//#include "yantarmonitoralertaction.h"

class MonitorHistory;

//объекты этого класса отображают информацию об одном мониторе
class WidgetMonitor : public QFrame
{
    Q_OBJECT
public:
    explicit WidgetMonitor(int num, QWidget *parent = 0);
    ~WidgetMonitor();
    const int monitorNumber;//номер (адрес) монитора

    void switchMonitoring();//вкл/выкл отправку запросов монитору
    void changeRegValue();//окно изменения значения регистров монитора
    void ProcessResponse(CmdHolder &ch);//обработка ответа от монитора
    void ChangeTimerInterval(int interval);//изменение интервала таймера обновления значений монитора
    void openMonitorHistory();//открыть окно просмотра архива событий монитора
    void setMonitorViewMode(bool textMode);//выставить режим отображения монитора: true - текстовый режим, false - графический
    void changeGraphSize();//изменить размер графиков
    void switchViewMode();//сменить режим отображения монитора

private:
    void createWidget();//посторить графику объекта
    void changeStatus(quint16 reg);//выставить статус монитора
    void changeWarningLevel(QLabel *labelLevel, quint16 reg);//выставить уровень тревоги канала
    QString getErrorDescription(quint8 err = 0);//получить расшифровку ошибки
    void setErrorIndicator(quint8 err);//выставить индикатор ошибки
    void setMonitoringStatus(bool enabled);//выставление статуса мониторинга

    MonitorHistory *monHistoryWindow;//окно архива
    RegisterValueChange *regValueChangeWindow;//окно изменения значения регистра

    //текстовое отображение
    QVBoxLayout *textModeLayout;
    QLabel *labelTextStatus;
    QLabel *labelLevelN;
    QLabel *labelLevelG;
    QLabel *labelFonN;
    QLabel *labelFonG;
    QLabel *labelMonitoringStatus;
    QLabel *labelErrorIndicator;
    QProgressBar *progressG;
    QProgressBar *progressN;
    QGroupBox *gbG;
    QGroupBox *gbN;

    //графическое отображение
    QLabel *labelKU;//Нажата кнопка управления
    QLabel *labelNSP;//Нет сетевого питания
    QLabel *labelNNB;//Низкое напряжение батареи
    QLabel *labelDV;//Датчик вскрытия
    QLabel *labelOU;//Отказ в устройстве
    QLabel *labelNSVP;//Устройство не смогло восстановить параметры при старте
    QGroupBox *gbGraphStatus;
    QGroupBox *gbGGraph;
    QGroupBox *gbNGraph;
    Graphic *graphicG;
    Graphic *graphicN;

    ConnectionManager *connection;
    QTimer *cmdSendTimer;//таймер для периодического отправления запроса на состояние монитора
    CmdHolder cmdMonStatus;//периодичестки отправляемый запрос устройству
    QTimer *errorTimer;//таймер сброса ошибки

    void requestForMeasureRanges();
    
private slots:
    void sendCommands();//отправить запрос устройству
    void ResetErrorIndicator();//сбросить индикатор ошибки

signals:
    void sendRequest(CmdHolder);//отправить запрос
    void archiveEventRecieved(CmdHolder &);//получена запись из архива
    void registerValueRecieved(CmdHolder &);//получено значение регистра
};

#endif // WIDGETMONITOR_H
