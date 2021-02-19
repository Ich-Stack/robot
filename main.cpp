#include <QApplication>
#include <QObject>
#include <QDebug>
#include "widget/init.h"
#include "widget/mainwidget.h"
#include "widget/edit.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget *mainwidget = new MainWidget();
//    mainwidget->setWindowIcon(QIcon(":/winicon.png"));
//    INIT *start = new INIT();
//    start->setWindowIcon(QIcon(":/winicon.png"));
//    start->show();

//    QObject::connect(start, &INIT::btn_start_click, [=](){
//        start->close();
        mainwidget->show();
//    });
    return a.exec();
}
