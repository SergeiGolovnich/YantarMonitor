#include <QApplication>

#include "testtcpyantarmonitor.h"
#include "mainwindow.h"

#include "test_cmdholder.h"

int main(int argc, char *argv[])
{
    QApplication::setStyle(QStyleFactory::create("QFusionStyle"));

    QApplication a(argc, argv);

    //исправление налезания заголовка GroupBox на содержимое
    a.setStyleSheet("QGroupBox::title {"
                    "text-decoration: none;"
                    "padding-bottom: 15px;}");
    //задание общих параметров для сохранения/загрузки настроек
    QCoreApplication::setOrganizationName("12CNII");
    QCoreApplication::setApplicationName("YantarMonitor");

    MainWindow w;
    w.show();

#ifdef QT_DEBUG
    //тестовая модель монитора Янтарь, подключенного через tcp/ip
    TestTcpYantarMonitor t;
    t.show();
    t.hide();//скрытие окна
    //тесты
    Test_CmdHolder test_cmdholder;
    if(QTest::qExec(&test_cmdholder) != 0)
        return 1;
#endif

    return a.exec();
}


