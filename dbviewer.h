#ifndef DBVIEWER_H
#define DBVIEWER_H

#include <QWidget>
#include <QtWidgets>
#include <responselogger.h>

//окно просмотра содержимого базы данных
class DBViewer : public QWidget
{
    Q_OBJECT
public:
    explicit DBViewer(QWidget *parent = 0);
private:
    void createWindow();//создание окна

    QTableView view;//таблица для вывода результата запроса
    QSqlQueryModel model;//запрос

    QSpinBox *sbRowsLimit;//количество отображаемых строк
    QPushButton *btnUpdate;//кнопка обновления запроса
private slots:
       void setQueryModel();//создание запроса к базе
       void saveDBToFile();//сохранить полученные записи из базы в файл
protected:
    virtual void closeEvent(QCloseEvent *);//событие закрытия окна

};

#endif // DBVIEWER_H
