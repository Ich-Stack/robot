﻿#include "videowidget.h"
#include "ui_videowidget.h"
#include <QPaintEvent>
#include <QPainter>

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void* pUser)
{
    char tempbuf[256] = { 0 };
    switch (dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        printf("----------reconnect--------%d\n", time(NULL));
        break;
    default:
        break;
    }
}

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);              //隐藏窗口框架

    ui->widget->installEventFilter(this);
    ui->btn_close->setNorAndPre(":/btn_close.png");
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::on_btn_close_clicked()                        //关闭按钮隐藏窗口
{
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);
    //注销用户
    NET_DVR_Logout(lUserID);
    //释放SDK资源
    NET_DVR_Cleanup();

    this->close();
}

bool VideoWidget::openVideo()
{
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

    //---------------------------------------
    // 注册设备
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = { 0 };
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy_s(struLoginInfo.sDeviceAddress, "192.168.0.82"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy_s(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy_s(struLoginInfo.sPassword, "Hik12345"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = { 0 };

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return false;
    }

    //---------------------------------------
    //启动预览并设置回调数据流
    HWND hWnd = (HWND)ui->video->winId();     //获取窗口句柄
    struPlayInfo.hPlayWnd = hWnd;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel = 1;       //预览通道号
    struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode = 1;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked = 1;       //0- 非阻塞取流，1- 阻塞取流

    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);
    if (lRealPlayHandle < 0)
    {
        printf("NET_DVR_RealPlay_V40 error\n");
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }

    return true;
}

bool VideoWidget::eventFilter(QObject *obj, QEvent *evt)        //事件过滤器，实现自定义窗口，可拖动
{
    QMouseEvent *mouse =  dynamic_cast<QMouseEvent *>(evt);
    if(obj == ui->widget&&mouse)                 //判断拖动
    {
        if(this->isMaximized())
        {
            return true;
        }
        static bool dragFlag = false;
        static QPoint dragPoint(0,0);
         if(mouse->button()==Qt::LeftButton && mouse->type() ==QEvent::MouseButtonPress)    //按下
        {
            dragFlag =true;
            dragPoint = mouse->pos();                                  //记录鼠标所在的界面位置
            return true;
        }
        else if(dragFlag &&  mouse->type() ==QEvent::MouseMove)     //拖动
        {
            this->move(mouse->globalPos() - dragPoint);
            return true;
        }
        else if(mouse->type() ==QEvent::MouseButtonRelease)
        {
            dragFlag = false;
            return true;
        }
    }
    return QWidget::eventFilter(obj,evt);
}
