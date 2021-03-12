#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")                       //解决中文乱码问题
#endif

#include "mylabel.h"
#include <QDebug>
#include <QWidget>

bool initIsTimeout = false;
bool isSetPoint = false;
int pointI = 0;
int num = 0;
int AckNum = 0;
vector<QPoint> areaPoint;
AAPOINT point;                //手动路径坐标点的数组
//AAPOINT tempPoint;            //手动路径模拟版本
AAPOINT enableArea;           //可行域的坐标数组

myLabel::myLabel(QWidget *parent) : QLabel(parent)
{
    timer = new QTimer();
    dialog = new Dialog();
    initTimeout = new QTimer();
    btn_areaClear = new QPushButton();
    start = APoint(35, 25);
    allStart = APoint(10, 10);
    node.push_back(QPoint());                                         //初始化使用

    btn_areaClear->setStyleSheet("color: rgb(246, 184, 75); font: 10pt 'OPPOSans'; border-radius:5px");
    connect(initTimeout, &QTimer::timeout, [=](){
        initIsTimeout = true;
        initTimeout->stop();
    });
    connect(timer, &QTimer::timeout, [=](){                         //0.5s改变一次笔的颜色，实现闪烁
        if(isChange)
        {
            isChange = false;
        }
        else
        {
            isChange = true;
        }
        this->update();
    });
    connect(btn_areaClear, &QPushButton::clicked, this, &myLabel::clearArea);
    //this->installEventFilter(this);                                 //安装事件过滤器

    end.clear();
    landAfter.clear();
    logInPoint.clear();
    loadEnableArea();                                               //打开加载可行域
    loadLandPoint();
    //loadAreaA();
}

void myLabel::mouseReleaseEvent(QMouseEvent *ev)                    //重写鼠标事件
{
    if(ev->button() == Qt::RightButton && isEditArea)               //鼠标右键表示完成放置可行域
    {
        initMaze();
    }
    else if(ev->button() == Qt::LeftButton)
    {
        if(1 == model)                                                             //放置可行区域
        {
            enableArea.x[enableArea.index] = ev->x();
            enableArea.y[enableArea.index] = ev->y();
            areaPoint.push_back(QPoint(ev->x(), ev->y()));
            enableArea.index++;
//            qDebug() << '(' << ev->x() << ", " << ev->y() << ')';
        }
        else if(2 == model)                                                       //手动设置路径
        {
            isSetPoint = true;
            point.x[point.index] = ev->x();
            point.y[point.index] = ev->y();
            point.index++;
            update();                                                             //更新
        }
        else if(3 == model)
        {
            if(pnpoly(enableArea.index, enableArea.x, enableArea.y, ev->x(), ev->y()))          //调用pnpoly函数，判断放置的点是否在可行域内
            {
                if(isLandingPoint)
                {
                    isLandingPoint = false;                                                            //登陆点坐标记录在logInPoint中
                    APoint logPoint(ev->x(), ev->y(), isVertical);                                          //计算需要除以10
                    logInPoint.push_back(logPoint);
                    emit signal_textAppend("成功放置登陆点");
                    emit signal_addLandingPoint();                                              //关闭并初始化edit
                    return;
                }
                UWBtask[UWBTaskIndex].x = ev->x();
                UWBtask[UWBTaskIndex].y = ev->y();
//                end[UWBTaskIndex] = APoint(ev->x()/10, ev->y()/10);//--------------------------------------------------------------------------------
//                end[UWBTaskIndex].omega = omega;
//                end[UWBTaskIndex].taskContantIndex = contIndex;                                 //任务要求索引
//                uchar inarea = 0;
//                if(inA(ev->x(), ev->y()))
//                {
//                    inarea = Area::A;
//                }
                end.push_back(APoint(ev->x()/10, ev->y()/10, omega, contIndex));
                UWBTaskIndex++;
                emit isAddPoint();
            }
            omega = 1;
            update();
        }
    }
}

void myLabel::paintEvent(QPaintEvent * event)                       //绘图事件
{
    QLabel::paintEvent(event);                                      //必须有，才能让背景图片显示出来
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);                  //抗锯齿
    if(isSetEnableArea && areaPoint.size())                         //防止areapoint为0，使程序崩溃
    {
        painter.setPen(QPen(QColor(152, 189, 249, 122), 2));
        painter.setBrush(QColor(152, 189, 249, 122));                           //画可行域的多边形
        painter.drawPolygon(&areaPoint.front(), enableArea.index);              //要用首元素的地址
    }
    if(isCanUpdate)                                                             //画节点路径
    {
        painter.setPen(QPen(Qt::yellow, 5));
        for(int i = 0; i < (int)node.size() - 1; i++)
        {
            painter.drawLine(node[i].x(), node[i].y(), node[i+1].x(), node[i+1].y());
        }
    }
    for(int i = 0; i < logInPoint.size(); i++)                                                          //画登陆点，为矩形
    {
        painter.setPen(Qt::red);
        painter.setBrush(Qt::red);
        int logX = logInPoint.at(i).x;
        int logY = logInPoint.at(i).y;
        bool vert = logInPoint.at(i).vertical;
        if(vert)
        {
            painter.drawRect(logX-7, logY-10, 15, 20);
            painter.drawText(logX-20, logY-15, "登陆点");
        }
        else
        {
            painter.drawRect(logX-10, logY-7, 20, 15);
            painter.drawText(logX-20, logY-10, "登陆点");
        }
    }
    if(1 == model)                                                              //动态绘制多边形可行区域
    {
        painter.setPen(QPen(QColor(152, 189, 249, 122), 2));
        if(enableArea.index > 1)
        {
            for(int i = 0; i < enableArea.index; i++)
            {
                painter.drawLine(enableArea.x[i], enableArea.y[i], enableArea.x[i+1], enableArea.y[i+1]);
            }
        }
        if(enableArea.index > 0)
        {
            painter.drawLine(enableArea.x[0], enableArea.y[0], enableArea.x[enableArea.index], enableArea.y[enableArea.index]);
            painter.drawLine(enableArea.x[enableArea.index-1], enableArea.y[enableArea.index-1], enableArea.x[enableArea.index], enableArea.y[enableArea.index]);
        }
    }
    else if(2 == model)
    {
        painter.setPen(QColor(246, 184, 75));
        painter.setBrush(QColor(246, 184, 75));
        for(int i = 0; i < point.index; i++)                                            //把记录的点画在UWB坐标图
        {
            painter.drawEllipse(QPoint(point.x[i], point.y[i]), 9, 9);                  //画目标点
            painter.drawText(point.x[i]+7, point.y[i]-7, QString("%1").arg(i+1));       //目标点标注，偏移5px
        }
        if(point.index > 1)
        {
            painter.setPen(QPen(QColor(246, 184, 75), 7));
            for(int i = 0; i < point.index - 1; i++)
            {
                painter.drawLine(point.x[i], point.y[i], point.x[i+1], point.y[i+1]);       //画目标路径
            }
        }
        if(point.x[0] && point.y[0])                                                        //设备坐标与地点坐标距离少于10cm判断到达
        {
            double dis = pow(point.x[pointI] - _x, 2) + pow((point.y[pointI] - _y)/ratio, 2);      //计算手动路径下的目标点距离，y方向按比例缩小
            if(dis < arriveDis)//3000
            {
                if(AckNum < point.index)
                {
                    pointbuf.x[pointI+1] = point.x[pointI];
                    pointbuf.y[pointI+1] = point.y[pointI];
                    MainWidget::setAckTrue();                                                   //调用myLabel函数设置ack为真，发送下一个地点坐标
                    AckNum++;
                    pointI++;
                }
            }
        }
        painter.setPen(QPen(QColor(102, 114, 251), 7));
        for(int i = 1; i < pointI; i++)
        {
            painter.drawLine(pointbuf.x[i], pointbuf.y[i], pointbuf.x[i+1], pointbuf.y[i+1]);
        }
    }
    else if(4 == model || model == 3)                                                                     //画任务模式的任务坐标点
    {
        if(isCreatePath)
        {
            QPoint tempStart = start.toQPoint()*10;
            painter.setPen(Qt::red);
            painter.setBrush(Qt::red);
            painter.drawEllipse(tempStart, 9, 9);                                                       //记录起点
            painter.drawText(tempStart.x() + 10, tempStart.y() - 10, "起点");                            //偏移100px
        }
        for(int i = 0; i < UWBTaskIndex; i++)
        {
            painter.setPen(QColor(246, 184, 75));
            painter.setBrush(QColor(246, 184, 75));
            painter.drawEllipse(QPoint(UWBtask[i].x, UWBtask[i].y), 9, 9);                  //画点
            painter.drawText(UWBtask[i].x+5, UWBtask[i].y-5, QString("代号:%1").arg(UWBtask[i].UWBTaskCode));         //显示任务代号
        }
    }
    if(isChange)
    {
        painter.setPen(Qt::transparent);                            //0.5s换一次画笔，实现闪烁功能
        painter.setBrush(Qt::transparent);
    }
    else
    {
        painter.setPen(QColor(102, 114, 251));
        painter.setBrush(QColor(102, 114, 251));
    }
    if(_x && _y)
    {
        painter.drawEllipse(QPoint(_x, _y), 9, 9);                              //画当前设备圆
    }
}

void myLabel::clearSetPath()                                                                //清除数据(手动设置路径)
{
    _x = 0;
    _y = 0;
    pointI = 0;
    taskModel = true;
    point.index = 0;
    UWBTaskIndex = 0;
    pointbuf.x[0] = 0;
    pointbuf.y[0] = 0;
    isSetPoint = false;
    memset(point.x, 0, 60);
    memset(point.y, 0, 60);
}

void myLabel::getCurrent(double x, double y)                      //获取myLabel发送的数据
{
    _x = static_cast<int>((x + 10) / 20) * 20;
    _y = static_cast<int>((y + 10) / 20) * 20;
    inArea = pnpoly(enableArea.index, enableArea.x, enableArea.y, x, y);
//    if(isSetPoint && initIsTimeout && inArea)                      //超过3s的初始化时间并且设置了目标点，记录当前坐标为出发点
    if(!isCreatePath && inArea)                                      //超过3s的初始化时间并且设置了目标点，记录当前坐标为出发点
    {
        APoint temp(x/10, y/10);
        start = temp;                                                //设备开始的位置
        pointbuf.x[0] = x/10;                                        //记录开机的坐标
        pointbuf.y[0] = y/10;
        if(allFirst)
        {
            allStart = temp;
            allFirst = false;
        }
    }
    if(end.size())
    {
        double end_dis = pow(x - end.at(nowIndex).x*10, 2) + pow((y - end.at(nowIndex).y*10)/ratio, 2);
        if(stopCalc)
        {
            if(end_dis < arriveDis)
            {
                arriveBeforeRun = true;
            }
            else
            {
                arriveBeforeRun = false;
            }
            return;
        }
        if(isCreatePath)//taskModel && isSetPoint &&
        {
            double node_dis = pow(node.at(node_index).x() - x, 2) + pow((node.at(node_index).y() - y)/ratio, 2);
            //qDebug() << "nowindex :" << nowIndex << "land :" << landAfter.at(landAfterIndex);
            if((nowIndex == landAfter.at(landAfterIndex)) && notAllLanded)        //判断下个终点是否需要经过登陆点
            {
                double land_dis = pow(x - logInPoint.at(landIndex).x, 2) + pow(y - logInPoint.at(landIndex).y/ratio, 2);//计算距离
                if(land_dis < arriveDis)                 //登陆模式结束
                {
                    if(landAfterIndex < landAfter.size()-1)
                    {
                        landAfterIndex++;
                    }
                    else
                    {
                        notAllLanded = false;
                    }
                    emit signal_landed();
                }
                if(land_dis < landingDis && landing)      //登陆模式
                {
                    landing = false;
                    QString strDis = QString("%1").arg(land_dis);
                    emit signal_textAppend(strDis);
                    emit signal_landing();
                }
            }

            if(node_dis < arriveDis)//3000           //到达节点发送下一个节点
            {
                if(node_index < node.size() - 1)
                {
                    node_index++;
                }

                if(end_dis < arriveDis)//3000        //到达坐标点发送下一个任务坐标
                {
                    //vecRemove(end.at(test));
                    _x = end.at(nowIndex).x*10;
                    _y = end.at(nowIndex).y*10;
                    emit isArrive();
                    //QTimer::singleShot(500, [=](){emit isArrive();});                       //触发信号，产生前往下一个任务坐标的路径节点
                    //UWBTaskIndex--;
                }
            }
    //        else if(star_dis < 2025)
    //        {
    //            emit abnormal();
    //        }
        }
    }
}

void myLabel::mouseMoveEvent(QMouseEvent *ev)                     //画多边形可行域效果
{
    if(model == 1)
    {
        enableArea.x[enableArea.index] = ev->x();
        enableArea.y[enableArea.index] = ev->y();
        update();
    }
}

void myLabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        const int x = ev->x();
        const int y = ev->y();
        btn_areaClear->setParent(this);
        btn_areaClear->setText("清除");
        btn_areaClear->move(x, y);
        btn_areaClear->show();
    }
    else
    {
        return;
    }
}

bool myLabel::pnpoly(int nvert, double *vertx, double *verty, int testx, int testy)         //多边形检测函数（检测点是否在多边形区域内）
{
    bool contain = false;
    int i, j;
    for (i = 0, j = nvert-1; i < nvert; j = i++)
    {
        if (((verty[i]>testy) != (verty[j]>testy)) && (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]))
           contain = !contain;
    }
    return contain;
}

void myLabel::clearTaskModel()                                   //清空enableArea函数（清除任务模式数据）
{
    //route = 0;
    model = 0;                                              //不可编辑模式
    node_index = 1;                                         //节点索引，0为起点，从第一个目标点开始
    UWBTaskIndex = 0;
    landAfterIndex = 0;
    landAfter.clear();
    node.clear();                                           //初始化节点向量
    node.push_back(QPoint());
    work_finish = false;
    isCreatePath = false;
    notAllLanded = true;
    end.clear();
    for(int i = 0; i < MAX_CITY_NUM; i++)
    {
        UWBtask[i].x = 0;
        UWBtask[i].y = 0;
        UWBtask[i].UWBTaskCode = 0;
        UWBtask[i].UWBTaskName.clear();
        UWBtask[i].UWBTaskLevel.clear();
    }
    //start = APoint(_x, _y);                                 //清除任务，记录当前任务点
    update();
}

void myLabel::get_Node(const std::list<QPoint> &_node, int _route)                   //获取节点list储存到数组
{
    if(0 == _node.size())
    {
        emit signal_textAppend("无法找到有效路径，请检查设备位置");
        return;
    }
    uchar idx = 0;
    QPoint node_buf;
    QPoint nodeParent = _node.front();
    QPoint laterPoint;
    QPoint currentPoint;
    node.pop_back();                                                                           //去掉寻路第一个重复的节点
    node.push_back(nodeParent*10);
    //route = _route;
    nodeParent *= 10;
    if(_node.size() > 1)                                                                            //节点数大于1做平滑优化
    {
        for(const auto &p : _node)
        {
            currentPoint = p*10;
            if(intersect(nodeParent, currentPoint))
            {
                node.push_back(laterPoint);
                nodeParent = laterPoint;
            }
            else
            {
                laterPoint = currentPoint;
                continue;
            }
            //实际使用版本
    //--------------------------------------------------------------------------
    //        //模拟版本
    //        node_buf.setX(p.x()*10);                                                               //放大10倍
    //        node_buf.setY(p.y()*10);
    //        node.push_back(node_buf);
    //        qDebug() << "the nodebuf : " << node_buf.x() << ", " << node_buf.y();
        }
    }
    if(_route == -1)                                                               //完成最后一个任务，返回起点回收设备
    {
        node_buf.setX(allStart.x*10);
        node_buf.setY(allStart.y*10);
        node.push_back(node_buf);
    }
    else if(_route == -2)
    {
        node_buf.setX(logInPoint.at(idx).x);
        node_buf.setY(logInPoint.at(idx).y);
        node.push_back(node_buf);
        if(idx < logInPoint.size())
        {
            idx++;
        }
    }
    else                                                                                       //加终点，避免路径断裂
    {
        node_buf.setX(end[_route].x*10);
        node_buf.setY(end[_route].y*10);
        node.push_back(node_buf);
    }
    isCreatePath = true;
}

int myLabel::left_right(QPoint a, QPoint b, double x, double y)
{
    double t;
    a.setX(a.x() - x);
    b.setX(b.x() - x);
    a.setY(a.y() - y);
    b.setY(b.y() - y);
    t = a.x() * b.y() - a.y() * b.x();
    return t == 0 ? 0 : t > 0 ? 1 : -1;
}

bool myLabel::pointAndLine(const QPoint &a, const QPoint &b, const QPoint &c, const QPoint &d)
{
    return (left_right(a, b, c.x(), c.y()) ^ left_right(a, b, d.x(), d.y())) == -2;
}

bool myLabel::intersect(const QPoint &a, const QPoint &b)
{
    int i = 0;
    bool intersect = false;
    QPoint c(areaPoint.front());
    QPoint d(areaPoint.back());

//    for(int i = 0; i < areaPoint.size(); i++)
//    {

//    }
    while (!intersect && i < areaPoint.size() - 2)                                                        //判断节点与多边形所有边是否相交
    {
        intersect = pointAndLine(a, b, c, d) && pointAndLine(c, d, a, b);
        c = areaPoint.at(i);
        d = areaPoint.at(i + 1);
        i++;
    }
    //qDebug() << "intersect : " << intersect;

    return intersect;
    //return pointAndLine(a, b, c, d) && pointAndLine(c, d, a, b);
}

void myLabel::vecRemove(const APoint &rpoint)
{
    vector<APoint>::iterator itr_head = end.begin();
    vector<APoint>::iterator itr_end = end.end();
    while(itr_head != itr_end)
    {
        if(*itr_head == rpoint)
        {
            end.erase(itr_head);
            return;
        }
        else
        {
            itr_head++;
        }
    }
}

void myLabel::clearArea()
{
    isSetEnableArea = false;
    enableArea.index = 0;
    areaPoint.clear();
    logInPoint.clear();
    for(size_t i = 0; i < max_map_num; i++)
    {
        enableArea.x[i] = 0;                              //清除可行域数组
        enableArea.y[i] = 0;
    }
    btn_areaClear->hide();                                //清除完后隐藏
    update();
}

void myLabel::initMaze()
{
    if(1 == model)
    {
        model = 0;
        timer->start(500);
        isSetEnableArea = true;
        isEditArea = false;
        setMouseTracking(false);
        std::vector<std::vector<bool>> mazebuf(max_map_num);              //定义并且初始化vector
        //resize mazebuf的大小，定义二维数组
        for(int i=0;i<max_map_num;i++)
        {
            mazebuf[i].resize(max_map_num);
        }
        //遍历赋值
        for(int i = 0; i < (int)mazebuf.size(); i++)
        {
           for (int j = 0; j < (int)mazebuf[0].size();j++)
            {
                mazebuf[i][j] = true;
            }
        }
        for(int i = 0; i < max_map_num; i++)
        {
            for(int j = 0; j < max_map_num; j++)
            {
                if(pnpoly(enableArea.index, enableArea.x, enableArea.y, i*10, j*10))
                {
                    mazebuf[j][i] = false;
                }
            }
        }
        this->maze = mazebuf;
        update();                                                   //更新
    }
}

void myLabel::slot_waringShow()
{
    QMessageBox::StandardButton click = QMessageBox::question(NULL, "Tip", "导入数据会清除现有任务数据，是否继续？");
    if(click == QMessageBox::Yes)
    {
        slot_load();
    }
    else
    {
        return;
    }
}

void myLabel::slot_load()                                           //加载数据槽函数
{
    QFile file("data.json");
    QJsonValue jsonVal;
    QJsonArray jsonArr;
    QJsonObject jsonObj;
    QJsonDocument parse;
    QJsonParseError eor;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(file.isOpen())
    {
        QByteArray byteArr = file.readAll();
        file.close();
        parse = QJsonDocument::fromJson(byteArr, &eor);
        if(eor.error == QJsonParseError::NoError)
        {
            emit signal_clearTask();                                                                //清除旧数据
            jsonArr = parse.array();
            UWBTaskIndex = jsonArr.size() - 1;                                                      //-1除去第一个开始坐标点
            jsonVal = jsonArr.at(0);
            jsonObj = jsonVal.toObject();
            start = APoint(jsonObj.take("x").toInt(), jsonObj.take("y").toInt());
            for(int i = 0; i < UWBTaskIndex; i++)
            {
                jsonVal = jsonArr.at(i+1);
                jsonObj = jsonVal.toObject();
                int x = jsonObj.take("x").toInt();
                int y = jsonObj.take("y").toInt();
                UWBtask[i].UWBTaskName = jsonObj.take("name").toString();
                UWBtask[i].UWBTaskCode = jsonObj.take("code").toInt();
                UWBtask[i].UWBTaskLevel = jsonObj.take("level").toString();
                UWBtask[i].x = x;
                UWBtask[i].y = y;
                int taskContantIndex = jsonObj.take("contantIndex").toInt();
                double jomega = jsonObj.take("omega").toDouble();
                APoint buf(x/10, y/10, jomega, taskContantIndex);
                end.push_back(buf);
            }
            model = 4;
            emit signal_load();
        }
        else
        {
            emit signal_textAppend("读取失败，数据文件格式错误!");
            return;
        }
    }
    else
    {
        emit signal_textAppend("数据文件丢失!");
        return;
    }
}

void myLabel::slot_save()                                                           //保存数据槽函数
{
    QFile file("data.json");                                                        //保存任务数据
    QJsonArray jsonArr;
    QJsonObject jsonObj;
    QJsonObject startObj;
    QJsonDocument jsonDoc;

    startObj.insert("x", start.x);
    startObj.insert("y", start.y);
    jsonArr.insert(0, startObj);
    for(int i = 0; i < UWBTaskIndex; i++)
    {
        jsonObj.insert("name", UWBtask[i].UWBTaskName);
        jsonObj.insert("code", UWBtask[i].UWBTaskCode);
        jsonObj.insert("level", UWBtask[i].UWBTaskLevel);
        jsonObj.insert("contantIndex", end.at(i).taskContantIndex);
        jsonObj.insert("x", UWBtask[i].x);
        jsonObj.insert("y", UWBtask[i].y);
        jsonObj.insert("omega", end.at(i).omega);
        jsonArr.insert(i+1, jsonObj);
    }
    jsonDoc.setArray(jsonArr);
    QByteArray byteArr = jsonDoc.toJson(QJsonDocument::Compact);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(byteArr);
    file.close();

    QFile areaFile("area.json");                                                        //保存区域数据
    QJsonArray jsonAreaArr;
    QJsonObject jsonAreaObj;
    QJsonDocument jsonAreaDoc;
    int i = 0;
    for(i = 0; i < enableArea.index; i++)
    {
        jsonAreaObj.insert("x", enableArea.x[i]);
        jsonAreaObj.insert("y", enableArea.y[i]);
        jsonAreaArr.insert(i, jsonAreaObj);
    }
    jsonAreaDoc.setArray(jsonAreaArr);
    QByteArray byteAreaArr = jsonAreaDoc.toJson(QJsonDocument::Compact);
    areaFile.open(QIODevice::WriteOnly | QIODevice::Text);
    areaFile.write(byteAreaArr);
    areaFile.close();

    QFile landFile("land.json");                                                        //保存登陆点数据
    QJsonArray jsonLandArr;
    QJsonObject jsonLandObj;
    QJsonDocument jsonLandDoc;
    for(int i = 0; i < logInPoint.size(); i++)
    {
        jsonLandObj.insert("x", logInPoint.at(i).x);
        jsonLandObj.insert("y", logInPoint.at(i).y);
        jsonLandObj.insert("vertical", logInPoint.at(i).vertical);
        jsonLandArr.insert(i, jsonLandObj);
    }
    jsonLandDoc.setArray(jsonLandArr);
    QByteArray byteLandArr = jsonLandDoc.toJson(QJsonDocument::Compact);
    landFile.open(QIODevice::WriteOnly | QIODevice::Text);
    landFile.write(byteLandArr);
    landFile.close();
    emit signal_textAppend("保存成功!");
}

void myLabel::loadLandPoint()
{
    QFile file("land.json");
    QJsonValue jsonVal;
    QJsonArray jsonArr;
    QJsonObject jsonObj;
    QJsonDocument parse;
    QJsonParseError eor;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(file.isOpen())
    {
        QByteArray byteArr = file.readAll();
        file.close();
        parse = QJsonDocument::fromJson(byteArr, &eor);
        if(eor.error == QJsonParseError::NoError)
        {
            jsonArr = parse.array();
            int landPointSize = jsonArr.size();
            for(int i = 0; i < landPointSize; i++)
            {
                jsonVal = jsonArr.at(i);
                jsonObj = jsonVal.toObject();
                int x = jsonObj.take("x").toInt();
                int y = jsonObj.take("y").toInt();
                bool vert = jsonObj.take("vertical").toBool();
                APoint buf(x, y, vert);
                logInPoint.push_back(buf);
            }
            model = 4;
        }
        else
        {
            emit signal_textAppend("读取失败，数据文件格式错误!");
            return;
        }
    }
    else
    {
        emit signal_textAppend("数据文件丢失!");
        return;
    }
}

void myLabel::loadEnableArea()
{
    QFile file("area.json");
    QJsonValue jsonVal;
    QJsonArray jsonArr;
    QJsonObject jsonObj;
    QJsonDocument parse;
    QJsonParseError eor;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(file.isOpen())
    {
        QByteArray byteArr = file.readAll();
        file.close();
        parse = QJsonDocument::fromJson(byteArr, &eor);
        if(eor.error == QJsonParseError::NoError)
        {
            jsonArr = parse.array();
            enableArea.index = jsonArr.size();
            for(int i = 0; i < enableArea.index; i++)
            {
                jsonVal = jsonArr.at(i);
                jsonObj = jsonVal.toObject();
                int x = jsonObj.take("x").toInt();
                int y = jsonObj.take("y").toInt();
                enableArea.x[i] = x;
                enableArea.y[i] = y;
                QPoint buf(x, y);
                areaPoint.push_back(buf);
            }
            model = 1;
            isSetEnableArea = true;
            initMaze();
        }
        else
        {
            emit signal_textAppend("读取失败，数据文件格式错误!");
            return;
        }
    }
    else
    {
        emit signal_textAppend("数据文件丢失!");
        return;
    }
}

void myLabel::clearNode()
{
    //route = 0;
    landAfterIndex = 0;
    landAfter.clear();
    node_index = 1;                                         //节点索引，0为起点，从第一个目标点开始
    node.clear();                                           //初始化节点向量
    node.push_back(QPoint());
    work_finish = false;
    isCreatePath = false;
    notAllLanded = true;

//    node.clear();
//    node.push_back(QPoint());
}

//void myLabel::loadAreaA()
//{
//    QFile file("areaA.json");
//    QJsonValue jsonVal;
//    QJsonArray jsonArr;
//    QJsonObject jsonObj;
//    QJsonDocument parse;
//    QJsonParseError eor;
//    file.open(QIODevice::ReadOnly | QIODevice::Text);
//    if(file.isOpen())`
//    {
//        QByteArray byteArr = file.readAll();
//        file.close();
//        parse = QJsonDocument::fromJson(byteArr, &eor);
//        if(eor.error == QJsonParseError::NoError)
//        {
//            jsonArr = parse.array();
//            enableArea.index = jsonArr.size();
//            for(int i = 0; i < enableArea.index; i++)
//            {
//                jsonVal = jsonArr.at(i);
//                jsonObj = jsonVal.toObject();
//                int x = jsonObj.take("x").toInt();
//                int y = jsonObj.take("y").toInt();
//                QPoint buf(x, y);
//                areaA.push_back(buf);
//            }
//        }
//        else
//        {
//            emit signal_textAppend("读取失败，数据文件格式错误!");
//            return;
//        }
//    }
//    else
//    {
//        emit signal_textAppend("数据文件丢失!");
//        return;
//    }
//}

//bool myLabel::inA(const uint &_x, const uint &_y)
//{
//    bool xContain, yContain;
//    if(_x > areaA.at(0).x() && _x < areaA.at(1).x())
//        xContain = true;
//    if(_y > areaA.at(0).y() && _y < areaA.at(1).y())
//        yContain = true;
//    if(xContain && yContain)
//        return true;
//    else
//        return false;
//}
//接口
void myLabel::setStopCalc(const bool &statu)
{
    stopCalc = statu;
}

bool myLabel::getStopCalc() const
{
    return stopCalc;
}

std::vector<QPoint> myLabel::get_vector_node()
{
//    for(const auto &p : node)
//    {
//        qDebug() << p.x() << ", " << p.y();
//    }
    return node;

//模拟版本
//    _vector_node = node;
//    for(auto &p : temp)
//    {
//        qDebug() << '(' << p.x() << ", " << p.y() << ')' << " size : " << temp.size();
//    }
}

void myLabel::setArriveBeforeRun(const bool &res)
{
    arriveBeforeRun = res;
}

bool myLabel::getArriveBeforeRun()
{
    return arriveBeforeRun;
}

void myLabel::addlandAfter(const int &_landAfter)
{
    //qDebug() << _landAfter;
    landAfter.push_back(_landAfter);
}

void myLabel::setInArea(bool _inArea)
{
    inArea = _inArea;
}

void myLabel::setIsSetPoint(bool _isSetPoint)
{
    isSetPoint = _isSetPoint;                                                                  //接口
}

void myLabel::set_omega(double _omega)
{
    omega = _omega;
}

void myLabel::set_taskContantIndex(int _contIndex)
{
    contIndex = _contIndex;
}

bool myLabel::getInArea()
{
    return inArea;
}

AAPOINT* myLabel::sendPoint()                                                                    //传输数据(使用版本)
{
    return &point;
}

//AAPOINT* myLabel::sendPoint()//模拟手动路径版本
//{
//    tempPoint.x[0] = point.x[0];
//    tempPoint.y[0] = point.y[0];
//    for(int i = 1; i < point.index; i++)            //n个点n-1个间隙
//    {
//        int ii = (i - 1)*4;
//        double dx1 = (point.x[i] + point.x[i-1]) / 2;
//        double dy1 = (point.y[i] + point.y[i-1]) / 2;
//        double dx2 = (point.x[i] + dx1) / 2;
//        double dy2 = (point.y[i] + dy1) / 2;
//        double dx3 = (dx1 + point.x[i-1]) / 2;
//        double dy3 = (dy1 + point.y[i-1]) / 2;


//        tempPoint.x[ii + 1] = dx3;
//        tempPoint.y[ii + 1] = dy3;
//        tempPoint.x[ii + 2] = dx1;
//        tempPoint.y[ii + 2] = dy1;
//        tempPoint.x[ii + 3] = dx2;
//        tempPoint.y[ii + 3] = dy2;
//        tempPoint.x[4*i] = point.x[i];
//        tempPoint.y[4*i] = point.y[i];
//    }
//    tempPoint.index = 4*point.index - 3;
//    for(size_t i = 0; i < 4*point.index - 3; i++)
//    {
//        qDebug() << '(' << tempPoint.x[i] << ", " << tempPoint.y[i] << ')';
//    }
//    return &tempPoint;
//}
void myLabel::setLandingDis(const uint &dis)
{
    landingDis = dis;
}

void myLabel::setArriveDis(const uint &dis)
{
    arriveDis = dis;
}

void myLabel::setLanding(const bool &flag)
{
    landing = flag;
}
void myLabel::setLandingPoint(const bool &flag)
{
    isLandingPoint = flag;
}

void myLabel::setIsVertical(const bool &res)
{
    isVertical = res;
}

void myLabel::setlandIndex(const uchar &idx)
{
    landIndex = idx;
}

int myLabel::setNowIndex(int _test)
{
    nowIndex = _test;
    return end.at(nowIndex).taskContantIndex;
}

size_t myLabel::taskSize() const
{
    return end.size();
}

void myLabel::setRatio(const double &_ratio)
{
    if(_ratio)
    {
        this->ratio = _ratio;
    }
    else
    {
        ratio = 1;
    }
}
