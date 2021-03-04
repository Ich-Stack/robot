#ifndef MYLABEL_H
#define MYLABEL_H

#include <QMouseEvent>
#include <QPushButton>
#include <QPainter>
#include <stdio.h>
#include <QLabel>
#include <QDebug>
#include <math.h>
#include <QDebug>
#include <vector>
#include <list>
#include "widget/mainwidget.h"
#include "algorithm/astar.h"

typedef struct{
    double x[60];
    double y[60];
    int index = 0;
}AAPOINT;

typedef struct{
    QString UWBTaskName;
    QString UWBTaskLevel;
    int UWBTaskCode;
    int x;
    int y;
}UWBTASK;

class myLabel : public QLabel
{
    Q_OBJECT
public:
    explicit myLabel(QWidget *parent = nullptr);

    int model = 0;                                                  //0不可编辑模式， 1设置可行域模式， 2手动设置路径模式， 3设置任务鼠标事件 4绘画坐标点
    int UWBTaskIndex = 0;
    int node_index = 1;                                             //节点索引，0为起点，从第一个目标点开始
    void set_omega(double _omega);
    void set_taskContantIndex(int _contIndex);
    bool taskModel = true;
    bool isSetEnableArea = false;
    bool isEditArea = false;
    bool isCanUpdate = false;
    bool work_finish = false;
    bool isClickLeftButton = false;
    std::vector<std::vector<bool>> maze;
    //QPoint start;
    APoint start;
    APoint allStart;                                                //全局起点
    //APoint end[60];
    std::vector<APoint> end;
    QTimer *timer;                                                  //定时器，改变坐标点颜色
    QTimer *initTimeout;
    UWBTASK UWBtask[60];
    std::vector<APoint> logInPoint;

public:
    double omega = 1;
    int contIndex = 0;

    static AAPOINT* sendPoint();
    bool getInArea();
    void setInArea(bool _inArea);
    void clearSetPath();                                            //清除
    void clearTaskModel();
    void getCurrent(double x, double y);                            //接受返回的坐标
    void setIsSetPoint(bool _isSetPoint);
    std::vector<QPoint> get_vector_node();
    void get_Node(const std::list<QPoint> &_node, int _route);
    bool pnpoly(int nvert, double *vertx, double *verty, int testx, int testy);           //PNPoly算法，判断点是否再多边形区域内
    int setNowIndex(int _test);
    size_t taskSize() const;
    void setRatio(const double &_ratio);
    void clearNode();
    void clearArea();
    void setStopCalc(const bool &statu);
    bool getStopCalc() const;
    void slot_load();
    void slot_save();
    void slot_waringShow();
    void setLanding(const bool &flag);
    bool intersect(const QPoint &a, const QPoint &b);
    void addlandAfter(const int &_landAfter);
    void setlandIndex(const uchar &idx);
    void setIsVertical(const bool &res);

private:
    int nowIndex = 0;
    uchar landIndex = 0;
    uchar landAfterIndex = 0;
    //int route = 0;
    std::vector<uchar> landAfter;
    int _x = 0, _y = 0;
    double ratio = 0;
    bool inArea = false;
    bool stopCalc = true;
    bool allFirst = true;                                          //全局第一次生成路径记录，起点为全局起点的标志位
    bool landing = true;                                           //正在登陆标志
    bool isLandingPoint = false;                                   //登陆点标志
    bool isChange = false;                                         //标志
    bool isVertical = false;
    //bool isCanDraw = false;
    bool isCreatePath = false;
    std::vector<QPoint> node;                                         //储存发送的坐标&&画路径
    AAPOINT pointbuf;                                                 //已经到达点的数组
    Dialog *dialog;
    const unsigned char max_map_num = 60;
    QPushButton *btn_areaClear;

private:
    virtual void paintEvent(QPaintEvent * event) override;          //绘图事件
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;       //鼠标释放事件
    virtual void mouseMoveEvent(QMouseEvent *ev) override;          //鼠标移动事件
    virtual void mouseDoubleClickEvent(QMouseEvent *ev) override;

    int left_right(QPoint a, QPoint b, double x, double y);           //两线段相交判断函数
    bool pointAndLine(const QPoint &a, const QPoint &b, const QPoint &c, const QPoint &d);
    void vecRemove(const APoint &rpoint);
    void loadEnableArea();
    void loadLandPoint();
    void initMaze();

signals:
    void isArrive();
    void abnormal();
    void isAddPoint();
    void signal_load();
    void signal_clearTask();
    void signal_textAppend(const QString &str);
    void signal_addLandingPoint();
    void signal_landing();
    void signal_landed();
};

#endif // MYLABEL_H
