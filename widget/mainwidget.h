#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QtSerialPort>
#include <QTableWidgetItem>
#include <QWebEngineView>
#include <QApplication>
#include <QMediaPlayer>
#include <QWebChannel>
#include <QMessageBox>
#include <QWebSocket>
#include <iostream>
#include <stdlib.h>
#include <QWidget>
#include <QString>
#include <QScreen>
#include <QPixmap>
#include <QRegExp>
#include <QTimer>
#include <thread>
#include <QRect>
#include <list>
#include <QDir>
#include "mycontral/mybtn.h"
#include "algorithm/astar.h"
#include "algorithm/cs.h"
#include "videowidget.h"
#include "mqtt/mqtt.h"
#include "com/comm.h"
#include "contral.h"
#include "dialog.h"
#include "edit.h"

struct taskNameAndCode{
    int code;
    QString name;
};

//struct Node
//{
//public:
//    Node() : x(0), y(0){}
//private:
//    int x = 0;
//    int y = 0;
//};

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    QWebChannel *channel;
    double generateRand(float min, float max);            //产生随机数函数
    static void setAckTrue();
    friend void createPath(MainWidget *e);

public slots:
    void receiveTaskCoordinate(QString, QString);
    void receiveSetPathPoint(QString lng, QString lat);
    void receiveShipCurrentPoint(QString lng, QString lat);
    void area_GPS();
    void slot_createFinish(const QString text);

private slots:
    int find_taskName(int code);
    void readcom();
    void btn_addTaskCoor_slot();
    void btn_addTask_slot();
    void on_btn_clearTask_clicked();
    void on_btn_createPath_clicked();
    void on_btn_opencpm_clicked();
    void on_btn_closecom_clicked();
    void on_btn_setUWBPath_clicked();
    void on_btn_clearPath_clicked();
    void on_radioButton_GPS_clicked();
    void on_radioButton_UWB_clicked();
    void on_btn_editTask_clicked();
    void on_btn_contral_clicked();
    void on_btn_video_clicked();
    void on_btn_run_clicked();
    void on_btn_back_clicked();
    
signals:
    void init_close();
    void createFinish(const QString text);

private:
    Ui::MainWidget *ui;

    virtual void closeEvent(QCloseEvent *event) override;
//----------------------------------------------------------------
    //任务路径部分
    EDIT *edit;
    //bool abnormal = false;
    bool stopCalc = false;
    bool m_isAddTaskCoor = false;
    bool finishedTask = false;
    int contIndex = 0;
    int UWBindex = 0;
    int m_taskI = 0;
    int m_taskNumber = 0;
    QTimer *workTimer;
    taskNameAndCode task[60];
    QString m_taskCode = 0;
    QString m_taskCoorLng = 0, m_taskCoorLat = 0;
    QString m_taskName = 0;
    QString m_taskLevel = 0;
    void dowork(int _contIndex);
//----------------------------------------------------------------
    //手动设置路径部分
    double m_setPathPointLng[50];                       //手动路径点经度数组
    double m_setPathPointLat[50];                       //手动路径点纬度数组
    int m_setPathPointIndex = 0;
//----------------------------------------------------------------
    //时间显示
    bool m_isStartUp = true; //启动/停止标志
    bool timeIsRun;   //时间运行标志
    bool m_isRun = true;  //按钮标志
    int timer_speedStopNum = 0;
    int runTime_s = 1;      //储存时间
    int runTime_m = 0;
    int runTime_h = 0;
    QTimer *timer_btnRun = new QTimer(this);  //开始运行按钮计时
    QTimer *timer_speed = new QTimer(this); //速度计时器
    QTimer *timer_speedStop = new QTimer(this); //停止速度计时器
//-----------------------------------------------------------------
    //窗口样式
    //void singShot(const size_t &sec);
    void setStateColor(const unsigned char &r, const unsigned char &g, const unsigned char &b);
//-----------------------------------------------------------------
    //串口部分
    COMM *m_spcomm;
    bool input = false;
    QString enstrData = NULL;
    void disposeData();
    //friend void sendData(MainWidget *e);
//-----------------------------------------------------------------
    //UWB部分
    int num = 0;
    double xbuflast = 0;
    double ybuflast = 0;
    double xbuf = 0;
    double ybuf = 0;
    Dialog *dialog;
    QTimer *sendDataTimer;
    QPalette palette;
    void magicTime();
    bool eventFilter(QObject *watched, QEvent *event);
//-----------------------------------------------------------------
    //路径规划
    CS *sa;
    PATH path;
    ASTAR *astar;
    int _nodeNum = 0;                                                                               //节点数量（城市个数）
    int route_index = 0;
    std::vector<QPoint> _node;
    int codeBuf[MAX_CITY_NUM];
    bool isCreatePath = false;
    void rebuildPath(APoint &curpoint);
//-----------------------------------------------------------------
    //MQTT协议连接物联网
    MQTT *m_client;
    //遥控
    CONTRAL *contral;
//-----------------------------------------------------------------
    //视频模块
    VideoWidget *m_videowidget;  //视频窗口
};

void createPath(MainWidget *e);                                                                     //生成路径回调函数

#endif // MAINWIDGET_H
