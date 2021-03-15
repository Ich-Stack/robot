#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")                       //解决中文乱码问题
#endif

#include "mainwidget.h"
#include "ui_mainwidget.h"

bool Ack = false;                                               //全局变量，方便静态函数调用
int UWBindex = 0;
int nowtaski = 0;
int doworkTimes = 0;
APoint cur;
AAPOINT* _point = myLabel::sendPoint();
int WBuf[MAX_CITY_NUM][MAX_CITY_NUM];                  //权，用于规划
unsigned int G[MAX_CITY_NUM][MAX_CITY_NUM];                  //测距，用于显示

MainWidget::MainWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    //this->setAttribute(Qt::WA_DeleteOnClose);

    //路径规划
    sa = new CS();
    edit = new EDIT();
    astar = new ASTAR();
    m_client = new MQTT();
    m_spcomm = new COMM();
    dialog = new Dialog();
    choose = new CHOOSE();
    workTimer = new QTimer();
    channel = new QWebChannel();
    sendDataTimer = new QTimer();
    webView = new QWebEngineView();
    m_videowidget = new VideoWidget();    //打开视频窗口
    fixP1 = new FIXWATER("65", nullptr);
    fixP2 = new FIXWATER("66", nullptr);
    fixP3 = new FIXWATER("96", nullptr);

    memset(m_setPathPointLng, 0.0, sizeof(double) * 50);        //初始化数组
    memset(m_setPathPointLat, 0.0, sizeof(double) * 50);

    ui->btn_editTask->setNorAndPre(":/picture/btn_edittask.png");
    ui->btn_createPath->setNorAndPre(":/picture/btn_createpath.png");
    ui->btn_clearTask->setNorAndPre(":/picture/btn_cleartask.png");
    ui->btn_run->setNorAndPre(":/picture/btn_run.png", ":/picture/btn_stop.png");
    ui->btn_opencpm->setNorAndPre(":/picture/btn_connect.png");
    ui->btn_setUWBPath->setNorAndPre(":/picture/btn_setpath.png");
    ui->btn_clearPath->setNorAndPre(":/picture/btn_clearpath.png");
    ui->btn_closecom->setNorAndPre(":/picture/btn_disconnect.png");
    ui->btn_contral->setNorAndPre(":/picture/btn_contral.png");
    ui->btn_area->setNorAndPre(":/picture/btn_area.png");
    ui->btn_video->setNorAndPre(":/picture/btn_video.png");
    ui->btn_back->setNorAndPre(":/picture/btn_back.png");
    ui->btn_clearTime->setNorAndPre(":/picture/btn_clear.png");
    ui->btn_monitoring->setNorAndPre(":/picture/btn_detection.png");

    webView->setParent(ui->map_widget);
    webView->page()->setWebChannel(channel);
    webView->resize(600, 600);
    webView->load(QUrl("http://112.74.84.128/"));
    channel->registerObject("sigma", this);                                       //创建与JS连接的对象content

    ui->btn_run->setFocus();

    connect(choose->btn_auto, &QPushButton::clicked, this, &MainWidget::slot_createAuto);       //自动/手动路径连接
    connect(choose->btn_set, &QPushButton::clicked, this, &MainWidget::slot_createSet);
    connect(edit->btn_load, &QPushButton::clicked, ui->label_UWB, &myLabel::slot_waringShow);         //label读写数据
    connect(edit->btn_save, &QPushButton::clicked, ui->label_UWB, &myLabel::slot_save);
    connect(ui->label_UWB, &myLabel::signal_load, this, &MainWidget::slot_load);               //保存/导入任务连接
    connect(ui->label_UWB, &myLabel::signal_textAppend, this, &MainWidget::slot_textAppend);    //mylabel在texttotal添加内容
    connect(ui->label_UWB, &myLabel::signal_clearTask, this, &MainWidget::on_btn_clearTask_clicked);    //label清除任务数据
    connect(ui->btn_back, &MYBTN::clicked, this, &MainWidget::slot_comeBack);
    connect(this, &MainWidget::createFinish, this, &MainWidget::slot_createFinish);
    connect(ui->label_UWB, &myLabel::signal_addLandingPoint, edit, &EDIT::close);
    connect(ui->label_UWB, &myLabel::signal_landing, this, &MainWidget::slot_landing);
    connect(ui->label_UWB, &myLabel::signal_landed, this, &MainWidget::slot_landed);
    connect(ui->btn_monitoring, &MYBTN::clicked, this, &MainWidget::slot_fixWaterShow);
    connect(m_client, &MQTT::messageReceived, this, &MainWidget::slot_receiveData);

    connect(ui->label_UWB, &myLabel::isArrive, [=](){           //每到达一个人任务点生成下一个任务坐标路径
            QString str = QString("到达第%1个任务点").arg(doworkTimes+1);
            sendDataTimer->stop();
            ui->textEdit_total->append(str);
            nowtaski++;
            ui->lineEdit_nowTask->setText(ui->label_UWB->UWBtask[find_taskName(path.code[nowtaski])].UWBTaskName);                 //显示当前任务名称
            dowork(contIndex);
//            contIndex = ui->label_UWB->settest(path.route[nowtaski]);
            if(path.route[nowtaski] != 0)
            {
                ui->lineEdit_distant->setText(QString::number(G[path.route[nowtaski-1]][path.route[nowtaski]]*dialog->getLength()/600/1000, 'f', 2));
            }
            else
            {
                return;
            }
    });
    connect(ui->btn_area, &QPushButton::clicked, [=](){
        if(ui->radioButton_UWB->isChecked())                                //选择任务模式放置可行区域
        {
            if(0 == ui->label_UWB->model)
            {
                ui->label_UWB->model = 1;                         //让UWB地图进入编辑可行域模式
                ui->label_UWB->isEditArea = true;
                ui->label_UWB->setMouseTracking(true);
                ui->label_UWB->timer->stop();                     //画可行区域时停止刷新，避免闪动
                ui->textEdit_total->append("请点击地图编辑可行区域，编辑完成后请点击鼠标右键！");
            }
            else if(2 == ui->label_UWB->model)
            {
                ui->textEdit_total->append("当前处于手动设置路径模式，请点击清除路径，再选择任务模式");
            }
        }
    });

    connect(ui->label_UWB, &myLabel::isAddPoint, [=](){
        ui->label_UWB->setIsSetPoint(true);                                                           //放置任务坐标记录设备当前的位置
        ui->label_UWB->UWBtask[UWBindex].UWBTaskName = edit->LineEdit_taskname_text();                 //获取任务名称
        ui->label_UWB->UWBtask[UWBindex].UWBTaskCode = edit->LineEdit_taskcode_text().toInt();         //获取任务代号

        codeBuf[UWBindex] = edit->LineEdit_taskcode_text().toInt();
        ui->label_UWB->model = 4;
        m_isAddTaskCoor = true;
        ui->textEdit_total->append("成功放置任务坐标点!");
        edit->activateWindow();
    });

    ui->lcdNumber->display(QString("00:00:00"));    //设置LCD默认显示
    connect(timer_btnRun, &QTimer::timeout, [=](){          //定时器倒计时，每一秒lcdNumber数字+1
        //设置显示时间
        ui->lcdNumber->display(QString("%1:%2:%3").arg(runTime_h, 2, 10, QChar('0')).arg(runTime_m, 2, 10, QChar('0')).arg(runTime_s, 2, 10, QChar('0')));
        runTime_s++;
        if(runTime_s >= 60)
        {
            runTime_s = 0;
            runTime_m ++;
            if(runTime_m >= 60)
            {
                runTime_m = 0;
                runTime_h++;
            }
        }
    });
    connect(ui->btn_clearTime, &QPushButton::clicked, [=](){   //清除按键连接
        //APoint p1(50, 50);
        //rebuildPath(p1);
        ui->lcdNumber->display(QString("00:00:00"));
        runTime_s = 1;
        runTime_m = 0;
        runTime_h = 0;
        if(timeIsRun == true)
        {
            timer_btnRun->start(1000);    //计时器重新计时，防止残留
        }
        else
        {
            return;
        }
    });

    connect(timer_speedStop, &QTimer::timeout, [=](){
        if(m_isStartUp)
        {
            switch(timer_speedStopNum)
            {
                 case 0: ui->lineEdit_speed->setText(QString::number(generateRand(0.1f, 0.2f), 'f', 2)); timer_speedStopNum++; break;
                 case 1: ui->lineEdit_speed->setText(QString::number(generateRand(1.3f, 1.5f), 'f', 2)); timer_speedStopNum++; break;
                 case 2: ui->lineEdit_speed->setText(QString::number(generateRand(2.5f, 2.7f), 'f', 2)); timer_speedStopNum++; break;
                 case 3: timer_speedStop->stop(); timer_speed->start(1300); ui->lineEdit_speed->setText(QString::number(generateRand(3.9f, 4.2f), 'f', 2)); break;
            }
        }
        else
        {
            switch(timer_speedStopNum)
            {
                 case 3: ui->lineEdit_speed->setText(QString::number(generateRand(2.5f, 2.7f), 'f', 2)); timer_speedStopNum--; break;
                 case 2: ui->lineEdit_speed->setText(QString::number(generateRand(1.3f, 1.5f), 'f', 2)); timer_speedStopNum--; break;
                 case 1: ui->lineEdit_speed->setText(QString::number(generateRand(0.1f, 0.2f), 'f', 2)); timer_speedStopNum--; break;
                 case 0: timer_speedStop->stop(); ui->lineEdit_speed->setText("0"); break;
            }
        }
    });

    connect(timer_speed, &QTimer::timeout, [=](){      //timer_speed槽函数
        ui->lineEdit_speed->setText(QString::number(generateRand(4.2f, 4.5f), 'f', 2));
    });

    ui->lineEdit_speed->setReadOnly(true);              //lineEdit只读
    ui->lineEdit_currentLng_x->setReadOnly(true);
    ui->lineEdit_currentLat_y->setReadOnly(true);

    //任务模块
    QStringList list = {"任务级别", "任务代号", "任务坐标", "任务名称"};
    ui->tableWidget_info->horizontalHeader()->setStretchLastSection(true);  //拉伸
    ui->tableWidget_info->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_info->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget_info->setColumnCount(4);    //设置列数
    ui->tableWidget_info->setColumnWidth(2, 200);   //设置列宽
    ui->tableWidget_info->setColumnWidth(1, 100);
    ui->tableWidget_info->setColumnWidth(0, 100);
    ui->tableWidget_info->setHorizontalHeaderLabels(list);  //导入表头

    connect(edit, &EDIT::btn_addtask, this, &MainWidget::btn_addTask_slot);
    connect(edit, &EDIT::btn_addcoor, this, &MainWidget::btn_addTaskCoor_slot);

    //串口部分
    connect(m_spcomm->m_serialPort,SIGNAL(readyRead()),this, SLOT(readcom()));

    //UWB部分
    ui->label_UWB->installEventFilter(this);                        //安装事件过滤器
    ui->stackedWidget_map->setCurrentIndex(1);
    ui->radioButton_UWB->click();                                   //默认显示
    connect(ui->radioButton_GPS, &QRadioButton::clicked, [=](){     //GPS, UWB与地图对应
        ui->stackedWidget_map->setCurrentIndex(0);
    });
    connect(ui->radioButton_UWB, &QRadioButton::clicked, [=](){
        ui->stackedWidget_map->setCurrentIndex(1);
    });
    connect(sendDataTimer, &QTimer::timeout, [=](){
        if(ui->label_UWB->taskModel)
        {
//            if(abnormal)                                                                                    //判断设备异常直接回收设备
//            {
//                double x = ui->label_UWB->start.x*dialog->getLength()/60;
//                double y = (60 - ui->label_UWB->start.y)*dialog->getWidth()/60;
//                str = QString("@M100 N1 %1 %2\r\n").arg(x).arg(y);
//                QByteArray buf = str.toLatin1();                //Qbytearray转char*
//                char* ch = buf.data();
//                m_spcomm->writeData(ch, str.length());
//            }
//            else
//            {
                if(ui->label_UWB->node_index < _node.size())
                {
                    //ui->label_UWB->getCurrent(_node.at(ui->label_UWB->node_index).x(), _node.at(ui->label_UWB->node_index).y());
                    double x = _node.at(ui->label_UWB->node_index).x()*dialog->getLength()/600;
                    double y = (600 - _node.at(ui->label_UWB->node_index).y())*dialog->getWidth()/600;
                    QString str = QString("@M100 N1 %1 %2\r\n").arg(QString::number(x)).arg(QString::number(y));
                    QByteArray buf = str.toLatin1();                //Qbytearray转char*
                    char* ch = buf.data();
                    m_spcomm->writeData(ch, str.length());
                }
                else
                {
                    on_btn_run_clicked();
                    ui->textEdit_total->append("已完成所有任务!");
                }
//            }
        }
        else
        {
            if(Ack)
            {
                Ack = false;
                UWBindex++;
            }
            if(UWBindex < _point->index)
            {
                QString str = QString("@M100 N1 %1 %2\r\n").arg(QString::number(_point->x[UWBindex]*dialog->getLength()/600, 'f', 3)).arg(QString::number((600 - _point->y[UWBindex])*dialog->getWidth()/600, 'f', 3)); //发送规则
                QByteArray buf = str.toLatin1();                //Qbytearray转char*
                char* ch = buf.data();
                m_spcomm->writeData(ch, str.length());      //发送数据
            }
            else
            {
                QByteArray arr("@M100 S\r\n");
                char *ch = arr.data();
                m_spcomm->writeData(ch, arr.size());

                ui->textEdit_total->append("已完成!");
                on_btn_run_clicked();                          //到达最终目标点后停止发送
                finishedTask = true;
            }
        }

        //自动路径模拟版本//
//        if(ui->label_UWB->node_index < (int)_node.size())
//        {
//            ui->label_UWB->getCurrent(_node.at(ui->label_UWB->node_index).x(), _node.at(ui->label_UWB->node_index).y());
//            //qDebug() << "send data : " << _node.at(ui->label_UWB->node_index).x*10 << ", " << _node.at(ui->label_UWB->node_index).y*10;
//            //qDebug() << "sendData point : " << _node.at(ui->label_UWB->node_index).x*dialog->getLength()/600 << ',' << _node.at(ui->label_UWB->node_index).y*dialog->getLength()/600;
//            //qDebug() << QString("@M100 N1 %1 %2@M100 S\r\n").arg(QString::number(_node.at(ui->label_UWB->node_index).x*dialog->getLength()/600, 'f', 3)).arg(QString::number(_node.at(ui->label_UWB->node_index).y*dialog->getWidth()/600, 'f', 3));
//            ui->lineEdit_currentLng_x->setText(QString::number(_node.at(ui->label_UWB->node_index-1).x()));
//            ui->lineEdit_currentLat_y->setText(QString::number(_node.at(ui->label_UWB->node_index-1).y()));
//        }
//        else
//        {
//            return;
//        }
        //手动路径模拟版本//
//        AAPOINT* _point = myLabel::sendPoint();
//        static size_t iii = 0;
//        if(Ack)
//        {
//            Ack = false;
//            UWBindex++;
//        }
//        if(iii < _point->index)
//        {
////            QString str = QString("@M100 N1 %1 %2\r\n").arg(QString::number(_point->x[UWBindex]*dialog->getLength()/600, 'f', 3)).arg(QString::number((600 - _point->y[UWBindex])*dialog->getWidth()/600, 'f', 3)); //发送规则
////            QByteArray buf = str.toLatin1();                //Qbytearray转char*
////            char* ch = buf.data();
////            m_spcomm->writeData(ch, str.length());      //发送数据
//            ui->label_UWB->getCurrent(_point->x[iii], _point->y[iii]);
//            iii++;
//        }
//        else
//        {
//            on_btn_run_clicked();
//            ui->textEdit_total->append("完成巡航!");
//            //sendDataTimer->stop();                          //到达最终目标点后停止发送
//        }
    });
    connect(workTimer, &QTimer::timeout, this, [=](){                               //任务时间定时器，超出时间10s回收设备
        //abnormal = true;                                                            //abnormal为true，senddatatimer发送起点坐标
        m_isStartUp = true;
        workTimer->stop();                                                          //任务倒计时定时器关闭
        ui->label_UWB->setStopCalc(false);
        timer_speedStop->start(1000);               //减速/加速定时器开

        //ui->textEdit_total->append("任务超时 ...\n将回收设备 ...");
        //disconnect(ui->label_UWB, &myLabel::abnormal);
        //m_spcomm->disconnect();
        if(doworkTimes > ui->label_UWB->taskSize() - 1)
        {
            QByteArray arr("@M100 S\r\n");
            char *ch = arr.data();
            m_spcomm->writeData(ch, arr.length());

            QTimer::singleShot(300, [=](){m_spcomm->writeData(ch, arr.length());});

            ui->textEdit_total->append("已经顺利完成所有任务！");
            on_btn_run_clicked();
            ui->lineEdit_distant->clear();
            ui->lineEdit_nowTask->clear();
            finishedTask = true;
            //stopCalc = true;
            m_isStartUp = false;
            return;
        }
        ui->label_UWB->setArriveBeforeRun(false);
        ui->label_UWB->setEnabled(false);
        contIndex = ui->label_UWB->setNowIndex(path.route[nowtaski]);
        sendDataTimer->start(300);                                                  //发送坐标定时器开
    });
    connect(ui->label_UWB, &myLabel::abnormal, this, [=](){
        //abnormal = false;                                                           //回收设备后解除异常
        //on_btn_run_clicked();                                                       //停止运行
        ui->textEdit_total->append("已回收设备!");
    });
    //MQTT协议连接物联网
    //遥控连接
    contral = new CONTRAL();
    connect(contral, &CONTRAL::stop, [=](){
        QString str = QString("@M100 S\r\n"); //停止
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
    connect(contral, &CONTRAL::up_press, [=](){
        QString str = QString("@M100 G\r\n"); //前进
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
    connect(contral, &CONTRAL::down_press, [=](){
        QString str = QString("@M20 B\r\n"); //后退
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
    connect(contral, &CONTRAL::left_press, [=](){
        QString str = QString("@M100 L\r\n"); //左转
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
    connect(contral, &CONTRAL::right_press, [=](){
        QString str = QString("@M100 y\r\n"); //右转
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
    connect(contral, &CONTRAL::cent_press, [=](){
        QString str;
        if(contral->getCatFlags())
        {
            str = QString("@M100 A1 170 10\r\n"); //机械爪抓
        }
        else
        {
            str = QString("@M100 a1 170 10\r\n"); //机械爪放
        }
        QByteArray buf = str.toLatin1();                //Qbytearray转char*
        char* ch = buf.data();
        m_spcomm->writeData(ch, str.length());      //发送数据
    });
}

MainWidget::~MainWidget()
{
    delete ui;
    delete sa;
    delete astar;
    delete dialog;
    delete channel;
    delete contral;
    delete m_spcomm;
    delete m_client;
    delete workTimer;
    delete timer_speed;
    delete timer_btnRun;
    delete sendDataTimer;
    delete timer_speedStop;
    delete webView;
}

void MainWidget::receiveTaskCoordinate(QString Lng, QString Lat)   //接收任务坐标
{
    m_taskCoorLng = Lng;
    m_taskCoorLat = Lat;
}

void MainWidget::btn_addTaskCoor_slot()
{
    if(edit->cmb_work_index() == 4)
    {
        int click = QMessageBox::question(NULL, "Tip", "登陆点是横向，还是纵向？",  "横向", "纵向");
        if(click == 0)
        {
            ui->label_UWB->setIsVertical(false);
        }
        else
        {
            ui->label_UWB->setIsVertical(true);
        }
        ui->label_UWB->setLandingPoint(true);
        ui->label_UWB->model = 3;
        this->activateWindow();                         //效率会比edit->close高
        return;
    }
    bool taskNameEmpty = edit->LineEdit_taskname_text().isEmpty();
    bool taskCodeEmpty = edit->LineEdit_taskcode_text().isEmpty();
    if(taskNameEmpty || taskCodeEmpty)                  //判断空
    {
        ui->textEdit_total->append("请先添加任务名称、要求和代号！");
    }
    else
    {
        if(ui->radioButton_GPS->isChecked())            //GPS地图
        {
            if(m_isAddTaskCoor == false)
            {
                for(int i = 0; i < 60; i++)             //判断任务代号是否重复
                {
                    if(task[i].code == edit->LineEdit_taskcode_text().toInt())
                    {
                        ui->textEdit_total->append("任务代号重复！");
                        return;
                    }
                }
                ui->textEdit_total->append("请点击地图放置任务坐标！");  //提示
                m_taskName = edit->LineEdit_taskname_text();         //获取任务名称
                m_taskCode = edit->LineEdit_taskcode_text();         //获取任务代号

//                QString sendTaskName = QString("receiveTaskLabelName(%1)").arg(m_taskCode.toInt());     //将任务代号发送给JS
//                ui->map_widget->page()->runJavaScript(sendTaskName);
//                QString command = QString("addTaskCoorBtn()");
//                ui->map_widget->page()->runJavaScript(command);      //调用JS中的addTaskCoorBtn()函数
                m_isAddTaskCoor = true;
            }
            else if(m_isAddTaskCoor == true)                            //多次点击添加坐标提示用户
            {
                ui->textEdit_total->append("你已添加目前任务的坐标！");
            }
        }
        else if(ui->radioButton_UWB->isChecked())                       //UWB地图
        {
            if(m_isAddTaskCoor == false)
            {
                for(int i = 0; i < 60; i++)             //判断任务代号是否重复
                {
                    if(ui->label_UWB->UWBtask[i].UWBTaskCode == edit->LineEdit_taskcode_text().toInt())
                    {
                        ui->textEdit_total->append("任务代号重复！");
                        return;
                    }
                }
                ui->textEdit_total->append("请点击地图放置任务坐标！");  //提示
                if(edit->cmb_level_index() == 2)
                {
                    ui->label_UWB->set_omega(3);
                }
                else if(edit->cmb_level_index() == 1)
                {
                    ui->label_UWB->set_omega(2);
                }
                ui->label_UWB->set_taskContantIndex(edit->cmb_work_index());  //任务要求索引
                ui->label_UWB->model = 3;                           //UWB任务模式
                this->activateWindow();
            }
            else if(m_isAddTaskCoor == true)                            //多次点击添加坐标提示用户
            {
                ui->textEdit_total->append("你已添加目前任务的坐标！");
            }
        }
    }
}

void MainWidget::on_btn_clearTask_clicked()
{
    if(ui->radioButton_GPS->isChecked())
    {
        if(m_isRun == false)
        {
            ui->btn_run->click();
        }
        m_taskI = 0;            //清空tablewidget的item数量
        m_taskCode.clear();     //清空任务名称，代号，经度，纬度
        m_taskName.clear();
        m_taskCoorLng.clear();
        m_taskCoorLat.clear();
        ui->lineEdit_nowTask->clear();
        ui->lineEdit_distant->clear();
        memset(m_setPathPointLng, 0.0, sizeof(double) * 50);    //清空手动设置路径数组
        memset(m_setPathPointLat, 0.0, sizeof(double) * 50);
//        QString command = QString("remove_overlay()");           //调用JS函数，清楚覆盖物
//        ui->map_widget->page()->runJavaScript(command);
        for(int i = 0; i < 60; i++)         //清空任务名称，代号数组
        {
            task[i].name.clear();
            task[i].code = 0;
        }
        while(m_taskNumber > 0)                                 //清空tablewidget
        {
            m_taskNumber--;
            ui->tableWidget_info->removeRow(m_taskNumber);
        }
    }
    else if(ui->radioButton_UWB->isChecked())
    {
        if(m_isRun == false)
        {
            on_btn_run_clicked();
        }
        Ack = false;            //全局变量，方便静态函数调用
        xbuf = 0;
        ybuf = 0;
        m_taskI = 0;            //清空tablewidget的item数量
        nowtaski = 0;           //清空路径迭代
        _nodeNum = 0;
        xbuflast = 0;
        ybuflast = 0;
        contIndex = 0;
        route_index = 0;        //清空迭代
        doworkTimes = 0;
        m_taskNumber = 0;
        path.clear();           //清空优化后路径
        _node.clear();          //清空节点向量
        isCreatePath = false;
        finishedTask = false;
        m_isAddTaskCoor = false;
        ui->label_UWB->setStopCalc(true);
        ui->lineEdit_nowTask->clear();
        ui->lineEdit_distant->clear();
        edit->lineEdit_taskcode_clear();
        edit->lineEdit_taskname_clear();
        ui->lineEdit_oxygen->clear();
        ui->lineEdit_pH->clear();
        ui->lineEdit_temperature->clear();
        ui->lineEdit_turbidity->clear();
        ui->label_UWB->clearTaskModel();                           //清空任务名称，代号，要求 和 清空可行域数组
        while(UWBindex > 0)                                 //清空tablewidget
        {
            UWBindex--;
            ui->tableWidget_info->removeRow(UWBindex);
        }
        UWBindex = 0;
    }
}

void MainWidget::receiveSetPathPoint(QString lng ,QString lat)      //接受网页返回的手动设置路径的坐标
{
    m_setPathPointLng[m_setPathPointIndex] = lng.toDouble();
    m_setPathPointLat[m_setPathPointIndex] = lat.toDouble();
    m_setPathPointIndex++;
    qDebug() << m_setPathPointLng[m_setPathPointIndex-1] << m_setPathPointLat[m_setPathPointIndex-1];   //输出效果
}

void MainWidget::receiveShipCurrentPoint(QString lng, QString lat)
{
    ui->lineEdit_currentLng_x->setText(lng.mid(0,10));
    ui->lineEdit_currentLat_y->setText(lat.mid(0,9));
//    strLatStart = y;
//    strLngStart = x;
//    if(calculate)
//    {
//        calculateData();
//        calculate = 0;
//    }
}

void MainWidget::on_btn_createPath_clicked()        //生成路径按钮
{
//    if(ui->label_UWB->isSetEnableArea && ui->label_UWB->UWBTaskIndex != 0 && !isCreatePath)
//    {
//        int _tempG = 0;
//        int _tempDis0 = 0;
//        int nowTask_index;
//        int dis0[MAX_CITY_NUM];
//        int dis0_end[MAX_CITY_NUM];
//        QString text("路径：起点");

//        memset(dis0, 0, sizeof(int)*MAX_CITY_NUM);
//        astar->InitAstar(ui->label_UWB->maze);
//        for(int i = 0; i < ui->label_UWB->UWBTaskIndex; i++)
//        {
//            for(int j = i; j < ui->label_UWB->UWBTaskIndex; j++)
//            {
//                _tempG = astar->getG(ui->label_UWB->end[i], ui->label_UWB->end[j], false);               //A*寻路获取距离G
//                GBuf[i][j] = (int)(_tempG * pow(1.002, _tempG) * ui->label_UWB->end[j].omega);
//                GBuf[j][i] = GBuf[i][j];                                                                //对角线赋值，避免两点间寻路得到的路径不相等，且减少寻路次数，增加速度
//            }
//        }
//        for(int i = 0; i < ui->label_UWB->UWBTaskIndex; i++)                                            //获取初始坐标点到每个任务点的距离G
//        {
//            _tempDis0 = astar->getG(ui->label_UWB->start, ui->label_UWB->end[i], false);
//            dis0_end[i] = _tempDis0;
//            dis0[i] = (int)(_tempDis0 * pow(1.002, _tempDis0) * ui->label_UWB->end[i].omega);
//        }
//        sa->transportG(GBuf, ui->label_UWB->UWBTaskIndex);                                              //传输距离数组，并初始化
//        sa->setNum(ui->label_UWB->UWBTaskIndex);
//        sa->Init_path(codeBuf);                                                                         //传输任务代号，使得路径与代号联立起来
//        sa->getDis0(dis0);
//        sa->getDis0_end(dis0_end);
//        sa->TSP_SA();
//        _nodeNum = sa->get_Node_Num();
//        path = sa->get_Path(ui->label_UWB->UWBTaskIndex);                                               //获取模拟退火算法优化后的路径
//        nowTask_index = find_taskName(path.code[0]);                                                    //通过任务代号获取当前正在执行的任务
//        //------------------------------------------------------------------------------------------------------------------------------------------------
//        //调试
//        ui->label_UWB->get_Node(astar->GetPath(ui->label_UWB->start, ui->label_UWB->end[path.route[0]], false), path.route[0], true);      //A*寻路获取节点
//        for(int i = 0; i < _nodeNum - 1; i++)
//        {
//            ui->label_UWB->get_Node(astar->GetPath(ui->label_UWB->end[path.route[i]], ui->label_UWB->end[path.route[i+1]], false), path.route[i+1], true);
//        }
//        //ui->label_UWB->get_Node(astar->GetPath(ui->label_UWB->end[path.route[_nodeNum-1]], ui->label_UWB->start, false), -1, true);
//        ui->label_UWB->update();
//        //------------------------------------------------------------------------------------------------------------------------------------------------
//        ui->label_UWB->get_vector_node(_node);
//        ui->label_UWB->isCanUpdate = true;
//        ui->label_UWB->update();
//        isCreatePath = true;
//        for(int i = 0; i < _nodeNum; i++)                                                                 //text_total显示路径顺序
//        {
//            text.append(QString(" -> %1").arg(path.code[i]));
//        }
//        ui->textEdit_total->append(text);
//        contIndex = ui->label_UWB->settest(path.route[0]);                                                //当前坐标不作为第一个目标点
//        ui->lineEdit_nowTask->setText(ui->label_UWB->UWBtask[nowTask_index].UWBTaskName);                 //显示当前任务名称
//        ui->lineEdit_distant->setText(QString::number(dis0[path.route[0]]*dialog->getLength()/600/1000, 'f', 2));             //显示距离
//    }
//    else
//    {
//        return;
//    }
    choose->show();
//    std::thread t(createPath, this);                                                                        //多线程，防止运行算法时软件卡顿
//    t.detach();
}

bool MainWidget::eventFilter(QObject *watched, QEvent *event)       //UWB
{
    if(watched == ui->label_UWB && event->type() == QEvent::Paint)
    {
        magicTime();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWidget::magicTime()                                        //在Label画图
{
    QPixmap *pixmap = new QPixmap();                                         //美观
    pixmap->load(":/picture/jizhan.png");
    *pixmap = pixmap->scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    QPainter painter(ui->label_UWB);
    painter.drawPixmap(530, 525, *pixmap);
    painter.setPen(Qt::gray);
    painter.drawText(5, 595, "0");
    for(int i = 5; i > 0; i--)
    {
        painter.drawLine(i*100,0,i*100,600);
        painter.drawLine(0, i*100, 600, i*100);
        painter.drawText(i*100+1, 598, QString("%1").arg(i*100));
        painter.drawText(0, i*100-2, QString("%1").arg((6 - i)*100));
    }
    painter.setPen(QPen(QColor(246, 184, 75), 7));                       //地图右上角标注
    painter.drawLine(480, 20, 520, 20);
    painter.drawText(530, 26, "目标路径");
    painter.setBrush(QColor(246, 184, 75));
    painter.drawEllipse(QPoint(500, 60), 6, 6);
    painter.drawText(530, 66, "任务坐标");
    painter.setPen(QPen(QColor(102, 114, 251), 7));
    painter.drawLine(480, 40, 520, 40);
    painter.drawText(530, 46, "已走路径");
    painter.setBrush(QColor(102, 114, 251));
    painter.drawEllipse(QPoint(500, 83), 6, 6);
    painter.drawText(530, 89, "设备坐标");
}

void MainWidget::on_btn_opencpm_clicked()
{
    QSerialPortInfo _info;                                  //储存info用于发送，连接串口
    QString serialNumber = NULL;

    dialog->exec();                                         //模态窗口，阻滞程序，避免用户还没输入完就判断
    if(dialog->getIsClick())
    {
        dialog->setIsClick(false);
        if(m_spcomm->isOpen)
        {
            ui->textEdit_total->append("设备已连接!");
            return;
        }
        else
        {
            foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())        //遍历所有串口
            {
                serialNumber = info.serialNumber();
                _info = info;
//                qDebug() << "Name : " << info.portName();                                   //打印串口信息
//                qDebug() << "Description : " << info.description();
//                qDebug() << "Manufacturer: " << info.manufacturer();
//                qDebug() << "Serial Number: " << info.serialNumber();
//                qDebug() << "System Location: " << info.systemLocation();
                if(serialNumber == "0001")                              //连接特定的串口
                {
                    m_spcomm->setBaudRate(9600);                        //波特率9600
                    m_spcomm->_setPort(_info);                          //设置端口，连接
                    if(m_spcomm->open())
                    {
                        m_spcomm->isOpen = true;
                        uint arriveDis = dialog->getArriveDis();
                        uint landingDis = dialog->getLandingDis();
                        ui->label_UWB->setLandingDis(landingDis);       //设置登陆点判断距离
                        ui->label_UWB->setArriveDis(arriveDis);         //设置目标点到达距离
                        ui->label_UWB->setRatio(dialog->getRatio());    //设置补偿倍数
                        ui->label_UWB->timer->start(500);
                        ui->label_UWB->initTimeout->start(3000);        //初始化3s的定时器
                        ui->textEdit_total->append("成功连接设备!");
                        setStateColor(0, 255, 0);                       //改变状态指示灯颜色
                        m_client->sub("/mqtt/pub");
                        ui->label_UWB->getCurrent(300, 300);
                        return;
                    }
                }
            }
            ui->textEdit_total->append("端口信息错误！\n无法连接设备!");
            return;
        }
    }
    else
    {
        return;
    }
}

void MainWidget::on_btn_closecom_clicked()          //关闭串口
{
    if(m_spcomm->isOpen)
    {
        m_spcomm->close();                   //关闭串口
        m_spcomm->isOpen = false;            //串口标志设为false
        dialog->setIsClick(false);           //dialog类中isclick标志设为false
        ui->label_UWB->setIsSetPoint(false);
        ui->label_UWB->timer->stop();        //停用定时器
        sendDataTimer->stop();
        ui->textEdit_total->append("设备已断开连接!");
        setStateColor(255, 0, 0);
    }
}

void MainWidget::readcom()
{
    static QString sumData;
    QString tempData = QString(m_spcomm->m_serialPort->readAll());         //读取串口数据
    sumData.append(tempData);
    //std::regex coorReg(R"(W \d+(\.\d+) \d+(\.\d+) B)");
    //std::smatch match;
    QRegularExpression coorReg(R"(W \d+(\.\d+) \d+(\.\d+) B)");
    QRegularExpressionMatch coorMatch = coorReg.match(sumData);

    //std::string strData = sumData.toStdString();
    if(coorMatch.hasMatch())
    {
        sumData.clear();
        enstrData = coorMatch.captured();
        disposeData();
    }

//    QRegularExpression cmdReg(R"(@F [A-Za-z]?)");
//    QRegularExpressionMatch cmdMatch = cmdReg.match(sumData);
//    if(cmdMatch.hasMatch())
//    {
//        workTimer->stop();
//    }

    QRegularExpression waterReg(R"(V \d+(\.\d+) \d+(\.\d+) \d+(\.\d+) \d+(\.\d+) B)");
    QRegularExpressionMatch waterMatch = waterReg.match(sumData);

    if(waterMatch.hasMatch())
    {
        sumData.clear();
        enstrData = waterMatch.captured();
        waterDispose();
    }
}

void MainWidget::waterDispose()
{
    enstrData.remove(0, 2);
    QStringList list = enstrData.split(' ');
    QString temperature = list.at(0) + "℃";
    QString turbidity = list.at(1) + "TU";
    QString oxygen = list.at(2) + "mg/L";
    QString pH = list.at(3);
    ui->lineEdit_temperature->setText(temperature);
    ui->lineEdit_turbidity->setText(turbidity);
    ui->lineEdit_oxygen->setText(oxygen);
    ui->lineEdit_pH->setText(pH);
    enstrData.clear();
}

void MainWidget::disposeData()//--------------------------------------------------------------------------------------------修改
{
    enstrData.remove(0, 2);                                         //移除从第0个字符往后两位的字符串
    QStringList list = enstrData.split(' ');                        //按照空格切割字符串
    QString x = list.at(0);
    QString y = list.at(1);

    double _x = x.toDouble()*600/dialog->getLength();
    double _y = y.toDouble()*600/dialog->getWidth();

    //int ix = static_cast<int>((_x + 10) / 20) * 20;
    //int iy = static_cast<int>((_y + 10) / 20) * 20;
    //cur = APoint(ix, iy);                                         //避障后，重新规划路径记录的起点

    ui->lineEdit_currentLng_x->setText(QString::number(_x, 'f', 1));
    ui->lineEdit_currentLat_y->setText(QString::number(_y, 'f', 1));

    ui->label_UWB->getCurrent(_x, 600 - _y);                        //按照比例尺发送坐标给mylabel
    enstrData.clear();
}

void MainWidget::on_btn_setUWBPath_clicked()
{
    if(ui->label_UWB->isSetEnableArea)
    {
        on_btn_clearTask_clicked();
        ui->label_UWB->clearArea();
        //ui->textEdit_total->append("当前处于任务模式，请点击清楚任务，再切换到手动设置路径模式");
    }
//    else
//    {
        if(ui->label_UWB->model)
        {
            //ui->textEdit_total->append("请先点击清除路径按钮；");
            return;
        }
        else
        {
            ui->label_UWB->model = 2;
            ui->label_UWB->taskModel = false;
            //ui->label_UWB->setInArea(true);
            ui->textEdit_total->append("你已选择手动设置路径模式，请点击地图设置路径！");
        }
//    }
}

void MainWidget::on_btn_clearPath_clicked()             //清空UWB地图和数据
{
    if(2 == ui->label_UWB->model)                       //手动设置路径模式下
    {
        Ack = false;
        xbuf = 0;
        ybuf = 0;
        UWBindex = 0;
        ui->label_UWB->model = 0;
        ui->label_UWB->clearSetPath();
        ui->label_UWB->clear();
        ui->lineEdit_currentLng_x->clear();
        ui->lineEdit_currentLat_y->clear();
        ui->label_UWB->update();
        sendDataTimer->stop();
        ui->label_UWB->initTimeout->start(3000);        //初始化3s
        ui->label_UWB->setInArea(false);
    }
    else
    {
        if(m_isRun == false)
        {
            on_btn_run_clicked();
        }
        //m_taskI = 0;            //清空tablewidget的item数量
        xbuflast = 0;
        ybuflast = 0;
        contIndex = 0;
        nowtaski = 0;           //清空路径迭代
        _nodeNum = 0;
        route_index = 0;        //清空迭代
        doworkTimes = 0;
        path.clear();           //清空优化后路径
        _node.clear();          //清空节点向量
        isCreatePath = false;
        finishedTask = false;
        m_isAddTaskCoor = false;
        ui->label_UWB->setStopCalc(true);        //默认状态不计算
        ui->lineEdit_nowTask->clear();
        ui->lineEdit_distant->clear();
        edit->lineEdit_taskcode_clear();
        edit->lineEdit_taskname_clear();

        ui->label_UWB->isCanUpdate = false;                        //清除自动生成的路径
        ui->label_UWB->clearNode();
    }
}

void MainWidget::on_btn_run_clicked()
{
    if(finishedTask)
    {
        ui->textEdit_total->append("所有任务已经完成!");
        return;
    }
    if(m_spcomm->isOpen)
    {
        timer_speedStop->start(1100);               //速度显示计时器
        if(true == m_isRun)
        {
            ui->btn_run->keyPress();
            timer_btnRun->start(1000);              //使用时间定时器
            m_isStartUp = true;
            timeIsRun = true;
            m_isRun = false;
            ui->label_UWB->setStopCalc(false);                   //开始运行使能计算
            bool arriveBeforeRun = ui->label_UWB->getArriveBeforeRun();
            if(ui->radioButton_UWB->isChecked())// && !arriveBeforeRun)    //判断是否选中UWB定位方式
            {
                ui->label_UWB->setEnabled(false);               //开始运行，不使能label
                QByteArray arr("@M100 A\r\n");
                char *ch = arr.data();
                m_spcomm->writeData(ch, arr.length());
                workTimer->start(10000);
                //sendDataTimer->start(300);                  //打开发送坐标的定时器1s发一次数据
            }
        }
        else if(false == m_isRun)
        {
            QByteArray arr("@M100 S\r\n");                      //停止运行发送停止指令
            char *ch = arr.data();
            m_spcomm->writeData(ch, arr.length());

            ui->btn_run->keyRelease();
            timer_speedStop->start(1000);
            timer_speed->stop();
            workTimer->stop();
            timer_btnRun->stop();
            m_isStartUp = false;
            timeIsRun = false;
            m_isRun = true;
            ui->label_UWB->setStopCalc(true);                                //停止运行不计算，只显示坐标
            if(ui->radioButton_UWB->isChecked())                 //判断是否选中UWB定位方式
            {
                ui->label_UWB->setEnabled(true);
                sendDataTimer->stop();                      //停止发送坐标
            }
        }
    }
    else
    {
        ui->textEdit_total->append("设备没有连接成功！");
        return;
    }
}

int MainWidget::find_taskName(int code)
{
    for(int i = 0; i < MAX_CITY_NUM; i++)
    {
        if(ui->label_UWB->UWBtask[i].UWBTaskCode == code)
        {
            return i;
        }
    }
    return false;
}

void MainWidget::btn_addTask_slot()
{
    if(m_isAddTaskCoor == false)                //检查是否添加坐标
    {
        ui->textEdit_total->append(QString("未添加任务坐标，请先添加任务坐标!"));
    }
    else
    {
        if(ui->radioButton_GPS->isChecked())
        {
            task[m_taskNumber].name = m_taskName;               //将任务的名称和代号储存在task结构体中
            task[m_taskNumber].code = m_taskCode.toInt();
            m_taskNumber++;
            ui->tableWidget_info->setRowCount(m_taskNumber);  //添加任务列表行数
            //把添加的任务添加进首页的tableWidget
            m_taskLevel = edit->cmb_level_text();
            for( ; m_taskI < m_taskNumber; m_taskI++)   //tablewidget显示
            {
                int index = 0;
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(m_taskLevel));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(m_taskCode));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(QString("(%1, %2)").arg(m_taskCoorLng).arg(m_taskCoorLat)));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(m_taskName));
                ui->tableWidget_info->item(m_taskI, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);  //设置文本居中
                ui->tableWidget_info->item(m_taskI, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget_info->item(m_taskI, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                if(edit->cmb_level_index() == 2)   //紧急为红色
                {
                    ui->tableWidget_info->item(m_taskI, 0)->setTextColor(QColor(Qt::red));
                }
                else if(edit->cmb_level_index() == 1)  //重要为黄色
                {
                    ui->tableWidget_info->item(m_taskI, 0)->setTextColor(QColor(Qt::darkYellow));
                }
                m_taskCoorLng.clear();      //每添加一次清空坐标，方便下次判断
                m_taskCoorLat.clear();
                m_isAddTaskCoor = false;
            }
        }
        else if(ui->radioButton_UWB->isChecked())
        {
            UWBindex++;
            m_isAddTaskCoor = false;
            edit->lineEdit_taskcode_clear();
            edit->lineEdit_taskname_clear();
            ui->tableWidget_info->setRowCount(UWBindex);    //添加任务列表行数
            ui->label_UWB->UWBtask[UWBindex-1].UWBTaskLevel = edit->cmb_level_text();
            for( ; m_taskI < UWBindex; m_taskI++)           //tablewidget显示
            {
                int index = 0;
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(ui->label_UWB->UWBtask[UWBindex-1].UWBTaskLevel));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(QString("%1").arg(ui->label_UWB->UWBtask[UWBindex-1].UWBTaskCode)));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(QString("(%1, %2)").arg(ui->label_UWB->UWBtask[UWBindex-1].x).arg(ui->label_UWB->UWBtask[UWBindex-1].y)));
                ui->tableWidget_info->setItem(m_taskI, index++, new QTableWidgetItem(ui->label_UWB->UWBtask[UWBindex-1].UWBTaskName));
                ui->tableWidget_info->item(m_taskI, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);  //设置文本居中
                ui->tableWidget_info->item(m_taskI, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                ui->tableWidget_info->item(m_taskI, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                int levelIndex = edit->cmb_level_index();
                if(levelIndex == 2)       //紧急为红色
                {
                    ui->tableWidget_info->item(m_taskI, 0)->setTextColor(QColor(Qt::red));
                }
                else if(levelIndex == 1)  //重要为黄色
                {
                    ui->tableWidget_info->item(m_taskI, 0)->setTextColor(QColor(Qt::darkYellow));
                }
            }
            edit->close();
        }
    }
}

void MainWidget::dowork(int _contIndex)
{
    m_isStartUp = false;
    doworkTimes++;
    if(0 == _contIndex)
    {
        m_isStartUp = true;
        ui->textEdit_total->append("巡航任务已完成!");
        sendDataTimer->start(300);
    }
    else
    {
        QByteArray arr;
        //sendDataTimer->stop();
        if(1 == _contIndex)
        {
            arr = "@M100 V\r\n";                   //水质
            ui->textEdit_total->append("正在执行水质检测任务 ...");
            char *ch = arr.data();
            m_spcomm->writeData(ch, arr.length());         //发送任务指令
            timer_speed->stop();                        //速度定时器关
            timer_speedStop->start(1000);               //减速/加速定时器开
            ui->label_UWB->setStopCalc(true);           //停止计算
            workTimer->start(5000);
            return;
        }
        else if (2 == _contIndex)
        {
            arr = "@M100 A\r\n";                   //抓取
            ui->textEdit_total->append("正在执行机械爪抓取任务 ...");
        }
        else if (3 == _contIndex)
        {
            arr = "@M100 a\r\n";                   //救援
            ui->textEdit_total->append("正在执行救援任务 ...");
        }
        char *ch = arr.data();
        m_spcomm->writeData(ch, arr.length());         //发送任务指令
        timer_speed->stop();                        //速度定时器关
        timer_speedStop->start(1000);               //减速/加速定时器开
        ui->label_UWB->setStopCalc(true);           //停止计算
        workTimer->start(10000);                    //任务倒计时10s定时器开
        return;
    }

//    //模拟版本
//    QTimer::singleShot(5000, this, [=](){           //做任务延时5s，再发下一个坐标
//        sendDataTimer->start(300);                  //做完任务重新开始0.3s发送坐标
//        m_isStartUp = true;
//        if(1 == _contIndex)
//        {
//            ui->lineEdit_temperature->setText("15℃");
//            ui->lineEdit_turbidity->setText("0.47");
//            ui->lineEdit_oxygen->setText("8mg/L");
//            ui->lineEdit_pH->setText("6.8");
//        }
//        ui->textEdit_total->append("任务完成!");
//        timer_speedStop->start(1100);
//        doworkTimes++;
        if(doworkTimes > ui->label_UWB->taskSize() - 1)
        {
            QByteArray arr("@M100 S\r\n");
            char *ch = arr.data();
            m_spcomm->writeData(ch, arr.length());

            QTimer::singleShot(300, [=](){m_spcomm->writeData(ch, arr.length());});

            ui->textEdit_total->append("已经顺利完成所有任务！");
            on_btn_run_clicked();
            finishedTask = true;
            m_isStartUp = false;
            ui->lineEdit_distant->clear();
            ui->lineEdit_nowTask->clear();
            return;
        }
        ui->label_UWB->setArriveBeforeRun(false);
        ui->label_UWB->setEnabled(false);
        contIndex = ui->label_UWB->setNowIndex(path.route[nowtaski]);
//    });
}

double MainWidget::generateRand(float min, float max)           //产生浮点数随机数函数
{
    static bool seedStatus;
    if (!seedStatus)
    {
        qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        seedStatus = true;
    }
    if(min>max)
    {
        double temp=min;
        min=max;
        max=temp;
    }
    double diff = fabs(max-min);
    double m1=(double)(qrand()%100)/100;
    double retval=min+m1*diff;
    return retval;
}

void MainWidget::closeEvent(QCloseEvent *)                     //关闭事件
{
    delete edit;
    delete choose;
    delete m_videowidget;
}

void MainWidget::setAckTrue()               //接口
{
    Ack = true;
}

void MainWidget::on_radioButton_GPS_clicked()                       //没错切换地图清空数据
{
    this->on_btn_clearTask_clicked();
}

void MainWidget::on_radioButton_UWB_clicked()
{
    this->on_btn_clearTask_clicked();
}

void MainWidget::on_btn_editTask_clicked()
{
    if(ui->label_UWB->isSetEnableArea)
    {
        edit->show();
    }
    else
    {
        ui->textEdit_total->append("请先设置可行区域!");
    }
}

void MainWidget::on_btn_contral_clicked()                               //打开遥控
{
    QTimer::singleShot(200, this, [=](){
        contral->setFocus();
        m_videowidget->activateWindow();
        contral->show();
    });
}

void MainWidget::on_btn_video_clicked()
{
    QTimer::singleShot(200, this, [=](){
        if(m_videowidget->openVideo())
        {
            ui->textEdit_total->append("成功连接视频");
        }
        else
        {
            ui->textEdit_total->append("无法连接到视频");
        }
        m_videowidget->show();
    });
}

void MainWidget::rebuildPath(APoint &curpoint)
{
//    sendDataTimer->stop();  //停止
//    _nodeNum = 0;
//    route_index = 0;        //清空迭代
//    path.clear();           //清空优化后路径
//    _node.clear();          //清空节点向量
//    ui->label_UWB->isCanUpdate = false;
//    isCreatePath = false;
//    ui->label_UWB->clearNode();
//    //清除原先路径
//    int _tempG = 0;
//    int _tempDis0 = 0;
//    int nowTask_index;
//    int dis0[MAX_CITY_NUM];
//    int dis0_end[MAX_CITY_NUM];
//    QString text("路径：");

//    ui->textEdit_total->append("遇到障碍物，重新生成路径 ...");
//    memset(dis0, 0, sizeof(int)*MAX_CITY_NUM);
//    //astar->InitAstar(ui->label_UWB->maze);
//    for(int i = 0; i < ui->label_UWB->UWBTaskIndex; i++)
//    {
//        for(int j = i; j < ui->label_UWB->UWBTaskIndex; j++)
//        {
//            _tempG = astar->getG(ui->label_UWB->end[i], ui->label_UWB->end[j], false);               //A*寻路获取距离G
//            G[i][j] = (int)(_tempG * pow(1.002, _tempG) * ui->label_UWB->end[j].omega);
//            G[j][i] = G[i][j];                                                                //对角线赋值，避免两点间寻路得到的路径不相等，且减少寻路次数，增加速度
//        }
//    }
//    for(int i = 0; i < ui->label_UWB->UWBTaskIndex; i++)                                            //获取初始坐标点到每个任务点的距离G
//    {
//        _tempDis0 = astar->getG(curpoint, ui->label_UWB->end[i], false);
//        dis0_end[i] = _tempDis0;
//        dis0[i] = (int)(_tempDis0 * pow(1.002, _tempDis0) * ui->label_UWB->end[i].omega);
//    }
//    sa->transportW(G, ui->label_UWB->UWBTaskIndex);                                              //传输距离数组，并初始化
//    sa->setNum(ui->label_UWB->UWBTaskIndex);
//    sa->Init_path(codeBuf);                                                                         //传输任务代号，使得路径与代号联立起来
//    sa->getWDis0(dis0);
//    //sa->getDis0_end(dis0_end);
//    sa->TSP_SA();
//    _nodeNum = sa->get_Node_Num();
//    path = sa->get_Path(ui->label_UWB->UWBTaskIndex);                                               //获取模拟退火算法优化后的路径
//    nowTask_index = find_taskName(path.code[0]);                                                    //通过任务代号获取当前正在执行的任务
//    //------------------------------------------------------------------------------------------------------------------------------------------------
//    //调试
//    ui->label_UWB->get_Node(astar->GetPath(curpoint, ui->label_UWB->end[path.route[0]], false), path.route[0], true);      //A*寻路获取节点
//    for(int i = 0; i < _nodeNum - 1; i++)
//    {
//        ui->label_UWB->get_Node(astar->GetPath(ui->label_UWB->end[path.route[i]], ui->label_UWB->end[path.route[i+1]], false), path.route[i+1], true);
//    }
//    ui->label_UWB->get_Node(astar->GetPath(ui->label_UWB->end[path.route[_nodeNum-1]], ui->label_UWB->start, false), -1, true);
//    ui->label_UWB->update();
//    //------------------------------------------------------------------------------------------------------------------------------------------------
//    //ui->label_UWB->get_vector_node(_node);
//    _node = ui->label_UWB->get_vector_node();
//    ui->label_UWB->isCanUpdate = true;
//    ui->label_UWB->update();
//    isCreatePath = true;
//    for(int i = 0; i < _nodeNum; i++)                                                                 //text_total显示路径顺序
//    {
//        text.append(QString("%1 ->").arg(path.code[i]));
//    }
//    text.append("起点");
//    ui->textEdit_total->append(text);
//    contIndex = ui->label_UWB->settest(path.route[0]);
//    ui->lineEdit_nowTask->setText(ui->label_UWB->UWBtask[nowTask_index].UWBTaskName);                 //显示当前任务名称
//    ui->lineEdit_distant->setText(QString::number(dis0[path.route[0]]*dialog->getLength()/600/1000, 'f', 2));             //显示距离
//    sendDataTimer->start(300);                                                                          //打开定时器，发送数据
}

void MainWidget::setStateColor(const unsigned char &r, const unsigned char &g, const unsigned char &b)
{
    const QString str = QString("border-radius:10px; background-color: rgb(%1, %2, %3)").arg(r).arg(g).arg(b);
    ui->label_s1->setStyleSheet(str);
    ui->label_s2->setStyleSheet(str);
    //ui->label_s3->setStyleSheet(str);
    ui->label_s4->setStyleSheet(str);
}

void MainWidget::area_GPS()
{
    ui->label_UWB->isSetEnableArea = true;
}

void MainWidget::slot_createAuto()
{
    std::thread t(createPath, this, true);            //另外线程执行生成路径算法                                                                    //多线程，防止运行算法时软件卡顿
    t.detach();
    choose->close();
}

void MainWidget::slot_createSet()
{
    std::thread t(createPath, this, false);            //另外线程执行生成路径算法                                                                    //多线程，防止运行算法时软件卡顿
    t.detach();
    choose->close();
}

void MainWidget::slot_load()
{
    m_isAddTaskCoor = false;
    m_taskI = ui->label_UWB->UWBTaskIndex;
    UWBindex = ui->label_UWB->UWBTaskIndex;
    ui->tableWidget_info->setRowCount(UWBindex);
    for(int i = 0; i < UWBindex; i++)
    {
        int index = 0;
        codeBuf[i] = ui->label_UWB->UWBtask[i].UWBTaskCode;
        ui->tableWidget_info->setItem(i, index++, new QTableWidgetItem(ui->label_UWB->UWBtask[i].UWBTaskLevel));
        ui->tableWidget_info->setItem(i, index++, new QTableWidgetItem(QString("%1").arg(ui->label_UWB->UWBtask[i].UWBTaskCode)));
        ui->tableWidget_info->setItem(i, index++, new QTableWidgetItem(QString("(%1, %2)").arg(ui->label_UWB->UWBtask[i].x).arg(ui->label_UWB->UWBtask[i].y)));
        ui->tableWidget_info->setItem(i, index++, new QTableWidgetItem(ui->label_UWB->UWBtask[i].UWBTaskName));
        ui->tableWidget_info->item(i, 0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);  //设置文本居中
        ui->tableWidget_info->item(i, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidget_info->item(i, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        if(ui->label_UWB->UWBtask[i].UWBTaskLevel == "紧急")   //紧急为红色
        {
            ui->tableWidget_info->item(i, 0)->setTextColor(QColor(Qt::red));
        }
        else if(ui->label_UWB->UWBtask[i].UWBTaskLevel == "重要")  //重要为黄色
        {
            ui->tableWidget_info->item(i, 0)->setTextColor(QColor(Qt::darkYellow));
        }
    }
    ui->textEdit_total->append("导入成功!");
}

void createPath(MainWidget *e, const bool &aut)
{
    if(e->ui->label_UWB->isSetEnableArea && e->ui->label_UWB->UWBTaskIndex != 0 && !e->isCreatePath)
    {
        int _tempG = 0;
        int _tempDis0 = 0;
        int back_tempG = 0;
        int nowTask_index;
        int dis0[MAX_CITY_NUM];
        int Wdis0[MAX_CITY_NUM];
        int uwbTaskIndex = e->ui->label_UWB->UWBTaskIndex;
        QString text("路径：起点");

        memset(dis0, 0, sizeof(int)*MAX_CITY_NUM);
        e->astar->InitAstar(e->ui->label_UWB->maze);
        for(int i = 0; i < uwbTaskIndex; i++)
        {
            for(int j = i; j < uwbTaskIndex; j++)
            {
                _tempG = e->astar->getG(e->ui->label_UWB->end.at(i), e->ui->label_UWB->end.at(j), false);               //A*寻路获取距离G
                back_tempG = e->astar->getG(e->ui->label_UWB->end.at(j), e->ui->label_UWB->end.at(i), false);
                WBuf[i][j] = e->calcW(_tempG, e->ui->label_UWB->end[j].omega);
                WBuf[j][i] = e->calcW(back_tempG, e->ui->label_UWB->end[i].omega);
                G[i][j] = _tempG;
                G[j][i] = G[i][j];                                                       //对角线赋值，避免两点间寻路得到的路径不相等，且减少寻路次数，增加速度
            }
        }
        for(int i = 0; i < uwbTaskIndex; i++)                                            //获取初始坐标点到每个任务点的距离G
        {
            _tempDis0 = e->astar->getG(e->ui->label_UWB->start, e->ui->label_UWB->end[i], false);
            dis0[i] = _tempDis0;
            Wdis0[i] = e->calcW(_tempDis0, e->ui->label_UWB->end[i].omega);
        }
        e->sa->transportW(WBuf, uwbTaskIndex);                                              //传输距离数组，并初始化
        e->sa->setNum(uwbTaskIndex);
        e->sa->Init_path(e->codeBuf);                                                                         //传输任务代号，使得路径与代号联立起来
        e->sa->getWDis0(Wdis0);
        if(aut)                                                                                               //自动路径规划，执行优化算法
        {
            e->sa->TSP_SA();
        }
        e->_nodeNum = e->sa->get_Node_Num();
        e->path = e->sa->get_Path(uwbTaskIndex);                                               //获取模拟退火算法优化后的路径
        nowTask_index = e->find_taskName(e->path.code[0]);                                                    //通过任务代号获取当前正在执行的任务
        //------------------------------------------------------------------------------------------------------------------------------------------------
        //调试
//        e->ui->label_UWB->get_Node(e->astar->GetPath(e->ui->label_UWB->start, e->ui->label_UWB->end.at(e->path.route[0]), false), e->path.route[0]);      //A*寻路获取节点
//        for(int i = 0; i < e->_nodeNum - 1; i++)
//        {
//            e->ui->label_UWB->get_Node(e->astar->GetPath(e->ui->label_UWB->end.at(e->path.route[i]), e->ui->label_UWB->end.at(e->path.route[i+1]), false), e->path.route[i+1]);
//        }
        e->searchPath();
        //e->test();
        //------------------------------------------------------------------------------------------------------------------------------------------------
        for(int i = 0; i < e->_nodeNum; i++)                                                                 //text_total显示路径顺序
        {
            //qDebug() << e->path.route[i] << "times :" << i;
            text.append(QString(" -> %1").arg(e->path.code[i]));
        }
        e->ui->lineEdit_nowTask->setText(e->ui->label_UWB->UWBtask[nowTask_index].UWBTaskName);                 //显示当前任务名称
        e->ui->lineEdit_distant->setText(QString::number(dis0[e->path.route[0]]*e->dialog->getLength()/600/1000, 'f', 2));             //显示距离
        emit e->createFinish(text);
    }
    else
    {
        return;
    }
}

void MainWidget::slot_createFinish(const QString text)                                                          //路径优化完成槽函数
{
    isCreatePath = true;
    ui->label_UWB->isCanUpdate = true;
    contIndex = ui->label_UWB->setNowIndex(path.route[0]);
    _node = ui->label_UWB->get_vector_node();
    ui->textEdit_total->append(text);
    ui->label_UWB->update();
}

void MainWidget::slot_textAppend(const QString &str)
{
    ui->textEdit_total->append(str);
}

int MainWidget::calcW(const int &G, const int &omega)        //最大长度4500，紧急omega==0.1, 重要omega==0.5
{
    if(0 == G)
    {
        return 0;
    }
    else if(3 == omega)                                           //紧急 (G - 4500) * 0.1
    {
        return static_cast<int>(G / 10 - 450);               //必为负数
    }
    else if(2 == omega)                                      //重要 (4500 - G) * 0.5
    {
        //return static_cast<int>(2250 - G / 2);               //必为正数
        return static_cast<int>(G / 2);
    }
    else
    {
        return G;
    }
}

//void MainWidget::searchPath()
//{
//    uchar route = path.route[0];
//    APoint start = ui->label_UWB->start;                                        //相对起点
//    APoint end = ui->label_UWB->end.at(route);                                  //相对终点
//    QPoint qStart = start.toQPoint()*10;                                        //转QPoint用于与多边形相交判断
//    QPoint qEnd = end.toQPoint()*10;
//    bool intersect = ui->label_UWB->intersect(qStart, qEnd);
//    size_t landPointSize = ui->label_UWB->logInPoint.size();
//    if(intersect && landPointSize)                                              //相交并且设有登陆点
//    {
//        APoint minLand = getMinLand(start)/10;                                  //找离相对起点最近的登陆点
//        ui->label_UWB->get_Node(astar->GetPath(start, minLand, false), -2);     //相对起点去登陆台
//        ui->label_UWB->get_Node(astar->GetPath(minLand, end, false), route);
//        ui->label_UWB->addlandAfter(route);
//    }
//    else
//    {
//        ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);      //A*寻路获取节点
//    }

//    for(int i = 0; i < _nodeNum - 1; i++)
//    {
//        route = path.route[i+1];
//        start = ui->label_UWB->end.at(path.route[i]);
//        end = ui->label_UWB->end.at(path.route[i+1]);
//        qStart = start.toQPoint()*10;
//        qEnd = end.toQPoint()*10;
//        intersect = ui->label_UWB->intersect(qStart, qEnd);
//        if(intersect && landPointSize)
//        {
//            APoint minLand = getMinLand(start)/10;
//            ui->label_UWB->get_Node(astar->GetPath(start, minLand, false), route);
//            ui->label_UWB->get_Node(astar->GetPath(minLand, end, false), route);
//            ui->label_UWB->addlandAfter(route);
//        }
//        else
//        {
//            ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
//        }
//    }
//}

//void MainWidget::test()
//{
//    uchar area = 0;
//    uchar route = path.route[0];
//    //size_t taskSize = ui->label_UWB->end.size();
//    APoint start = ui->label_UWB->start;                                        //相对起点
//    APoint end = ui->label_UWB->end.at(route);                                  //相对终点
//    APoint loginP = ui->label_UWB->logInPoint.front();
//    if(end.inArea != area)
//    {
//        ui->label_UWB->get_Node(astar->GetPath(start, loginP, false), -2);
//        ui->label_UWB->get_Node(astar->GetPath(loginP, end, false), route);
//        area = end.inArea;
//    }
//    else
//    {
//        ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
//    }

//    for(size_t i = 0; i < _nodeNum - 1; i++)
//    {
//        if(end.inArea != area)
//        {
//            ui->label_UWB->get_Node(astar->GetPath(start, loginP, false), -2);
//            ui->label_UWB->get_Node(astar->GetPath(loginP, end, false), route);
//            area = end.inArea;
//        }
//        else
//        {
//            ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
//        }
//    }
//}

void MainWidget::searchPath()
{
    uchar idx = 0;
    uchar route = path.route[0];
    size_t logPointSize = ui->label_UWB->logInPoint.size();
    APoint start = ui->label_UWB->start;                                        //相对起点
    APoint end = ui->label_UWB->end.at(route);                                  //相对终点
    std::vector<APointPuls> logPoint = searchPathSupplement();

    if(logPointSize == 0)
    {
        ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);      //A*寻路获取节点

        for(int i = 0; i < _nodeNum - 1; i++)
        {
            start = ui->label_UWB->end.at(path.route[i]);
            end = ui->label_UWB->end.at(path.route[i+1]);
            route = path.route[i+1];
            ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
        }
    }
    else
    {
        std::sort(logPoint.begin(), logPoint.end(), &sort_APointPlus);
        if(logPoint.front().route == route)
        {
            APoint login = logPoint.front().logPoint/10;
            ui->label_UWB->get_Node(astar->GetPath(start, login, false), -2);      //A*寻路获取节点
            ui->label_UWB->get_Node(astar->GetPath(login, end, false), route);
            ui->label_UWB->addlandAfter(route);
            idx++;
        }
        else
        {
            ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);      //A*寻路获取节点
        }

        for(int i = 0; i < _nodeNum - 1; i++)
        {
            start = ui->label_UWB->end.at(path.route[i]);
            end = ui->label_UWB->end.at(path.route[i+1]);
            route = path.route[i+1];
            if(idx >= logPointSize)
            {
                ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
            }
            else if(route == logPoint.at(idx).route)
            {
                APoint login = logPoint.at(idx).logPoint/10;
                ui->label_UWB->get_Node(astar->GetPath(start, login, false), -2);
                ui->label_UWB->get_Node(astar->GetPath(login, end, false), route);
                ui->label_UWB->addlandAfter(route);
                if(idx < logPointSize)
                {
                    idx++;
                }
            }
            else
            {
                ui->label_UWB->get_Node(astar->GetPath(start, end, false), route);
            }
        }
    }
}

std::vector<APointPuls> MainWidget::searchPathSupplement()
{
    uchar routeBuf = 100;
    uchar route = path.route[0];
    APoint start = ui->label_UWB->start;                                        //相对起点
    APoint end = ui->label_UWB->end.at(route);                                  //相对终点
    QPoint qStart = start.toQPoint()*10;                                        //转QPoint用于与多边形相交判断
    QPoint qEnd = end.toQPoint()*10;
    std::vector<APointPuls> res;
    bool vertical = calcVector(start, end);
    bool intersect = ui->label_UWB->intersect(qStart, qEnd);
    uchar num = getContainNum(start, end, vertical);
    APoint minLog;
    APointPuls temp(route, minLog);

    if(intersect || num > 1)                                                    //两点连线与区域相交或者两点间有两个登陆点，直接保存登陆点与终点
    {
        minLog = getMinLand(start, end, vertical);
        temp = APointPuls(route, minLog);
        res.push_back(temp);
    }
    else if(num == 1)
    {
        minLog = getMinLand(start, end, vertical);
        routeBuf = route;                                                             //-1代表起点
    }

    for(int i = 0; i < _nodeNum - 1; i++)
    {
        start = ui->label_UWB->end.at(path.route[i]);
        end = ui->label_UWB->end.at(path.route[i+1]);
        route = path.route[i+1];
        qStart = start.toQPoint()*10;
        qEnd = end.toQPoint()*10;
        vertical = calcVector(start, end);
        intersect = ui->label_UWB->intersect(qStart, qEnd);
        num = getContainNum(start, end, vertical);

        if(intersect || num > 1)                                                    //两点连线与区域相交或者两点间有两个登陆点，直接保存登陆点与终点
        {
            minLog = getMinLand(start, end, vertical);
            temp = APointPuls(route, minLog);
            res.push_back(temp);
        }
        else if(num == 1)
        {
            minLog = getMinLand(start, end, vertical);
            routeBuf = route;
        }
    }
    const uchar logSize = ui->label_UWB->logInPoint.size();
    if(res.size() < logSize)
    {
        temp = APointPuls(routeBuf, minLog);
        res.push_back(temp);
    }
    return res;
}

uchar MainWidget::getContainNum(const APoint &start, const APoint &end, const bool &vert)            //计算两点间登陆点个数函数
{
    //qDebug() << start.x << ", " << start.y;
    //qDebug() << end.x << ", " << end.y;
    uchar times = 0;
    int tolDis = 0;
    int posDis = 0;
    std::vector<APoint> &logPoint = ui->label_UWB->logInPoint;
    //qDebug() << logPoint.front().x << ", " << logPoint.front().y;
    if(vert)
    {
        for(const auto &p : logPoint)
        {
            tolDis = end.y - start.y;
            posDis = p.y/10 - start.y;
            if(p.vertical)
            {
                if(tolDis > 0)
                {
                    if(posDis < tolDis)
                        times++;
                }
                else
                {
                    if(posDis > tolDis)
                        times++;
                }
            }
        }
    }
    else
    {
        for(const auto &p : logPoint)
        {
            tolDis = end.x - start.x;
            posDis = end.x - p.x/10;
            if(!p.vertical)
            {
                if(tolDis > 0)
                {
                    if(posDis < tolDis)
                        times++;
                }
                else
                {
                    if(posDis > tolDis)
                        times++;
                }
            }
        }
    }
    return times;
}

bool MainWidget::calcVector(const APoint &start, const APoint &end)
{
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int res = (dy / dialog->getRatio()) - dx;
    if(res <= 0)
    {
        return false;           //y距离大于x距离，判断为垂直走向
    }
    else
    {
        return true;           //x距离大于y距离，判断为水平走向
    }
}

APoint MainWidget::getMinLand(const APoint &start, const APoint &end, const bool &vert)
{
//    uint minDis = 1e5;
//    APoint res;
//    for(int i = 0; i < ui->label_UWB->logInPoint.size(); i++)
//    {
//        uint x = abs(ui->label_UWB->logInPoint.at(i).x - cur.x);
//        uint y = abs(ui->label_UWB->logInPoint.at(i).y - cur.y);
//        uint total = x + y;
//        if(total < minDis)
//        {
//            minDis = total;
//            res = ui->label_UWB->logInPoint.at(i);
//            ui->label_UWB->setlandIndex(i);
//        }
//    }
//    return res;
    uint x = 0;
    uint y = 0;
    uint minDis = 1e5;
    APoint res;
    int tolDis = 0;
    int posDis = 0;
    std::vector<APoint> &logPoint = ui->label_UWB->logInPoint;
    for(const auto &p : logPoint)
    {
        if(vert)
        {
            tolDis = end.y - start.y;
            posDis = p.y/10 - start.y;
            if(p.vertical)
            {
                if(tolDis > 0)
                {
                    if(posDis < tolDis)
                    {
                        x = abs(start.x - p.x/10);
                        y = abs(start.y - p.y/10);
                        uint total = x + y;
                        if(total <= minDis)
                        {
                            minDis = total;
                            res = p;
                        }
                    }
                }
                else
                {
                    if(posDis > tolDis)
                    {
                        x = abs(start.x - p.x/10);
                        y = abs(start.y - p.y/10);
                        uint total = x + y;
                        if(total <= minDis)
                        {
                            minDis = total;
                            res = p;
                        }
                    }
                }
            }
        }
        else
        {
            tolDis = end.x - start.x;
            posDis = p.x/10 - start.x;
            if(!p.vertical)
            {
                if(tolDis > 0)
                {
                    if(posDis < tolDis)
                    {
                        x = abs(start.x - p.x/10);
                        y = abs(start.y - p.y/10);
                        uint total = x + y;
                        if(total <= minDis)
                        {
                            minDis = total;
                            res = p;
                        }
                    }
                }
                else
                {
                    if(posDis > tolDis)
                    {
                        x = abs(start.x - p.x/10);
                        y = abs(start.y - p.y/10);
                        uint total = x + y;
                        //qDebug() << "min :" << minDis << "tol :"<< total << p.x << ", " << p.y;
                        if(total <= minDis)
                        {
                            minDis = total;
                            res = p;
                        }
                    }
                }
            }
        }
    }
    return res;
}

//APoint MainWidget::getMinLand(const APoint &cur)
//{
//    uint minDis = 1e5;
//    APoint res;
//    for(int i = 0; i < ui->label_UWB->logInPoint.size(); i++)
//    {
//        uint x = abs(ui->label_UWB->logInPoint.at(i).x - cur.x);
//        uint y = abs(ui->label_UWB->logInPoint.at(i).y - cur.y);
//        uint total = x + y;
//        if(total < minDis)
//        {
//            minDis = total;
//            res = ui->label_UWB->logInPoint.at(i);
//            ui->label_UWB->setlandIndex(i);
//        }
//    }
//    return res;
//}

void MainWidget::slot_landing()                                         //准备登陆槽函数
{
    sendDataTimer->stop();
    ui->lineEdit_nowTask->setText("登陆");
    QByteArray arr("@M100 g\r\n");
    char *ch = arr.data();
    m_spcomm->writeData(ch, arr.length());
}

void MainWidget::slot_landed()                                          //登陆完成槽函数
{
    ui->label_UWB->setLanding(true);
    sendDataTimer->start(300);
}

void MainWidget::slot_comeBack()
{
    ui->textEdit_total->append("功能正在开发中...");
//    m_spcomm->isOpen = true;
//    cur = APoint(30 ,30);
//    if(m_spcomm->isOpen)
//    {
//        sendDataTimer->stop();
//        on_btn_clearPath_clicked();                                                                     //清除现有路径
//        ui->lineEdit_nowTask->setText("回收设备");
//        ui->label_UWB->get_Node(astar->GetPath(cur, ui->label_UWB->allStart, true), -1, true);          //寻找返回全局起点的路径
//        //sendDataTimer->start(300);
//    }
//    else
//    {
//        ui->textEdit_total->append("没有连接设备");
//        return;
//    }
}
//void MainWidget::singShot(const size_t &sec)
//{
////    QTimer::singleShot(sec, this, [](){qDebug() << "<>???";});
//}

void MainWidget::slot_fixWaterShow()
{
    if(fixWaterShowing)
    {
        fixWaterShowing = false;
        fixP1->setParent(ui->label_UWB);
        fixP1->move(100, 100);
        fixP1->show();
        fixP2->setParent(ui->label_UWB);
        fixP2->move(400, 100);
        fixP2->show();
        fixP3->setParent(ui->label_UWB);
        fixP3->move(100, 300);
        fixP3->show();
    }
    else
    {
        fixWaterShowing = true;
        fixP1->hide();
        fixP2->hide();
        fixP3->hide();
    }
}

void MainWidget::slot_receiveData(const QByteArray &message, const QMqttTopicName &topic)
{
    QList<QByteArray> list = message.split(' ');
    const int waterId = list.front().toInt();
    const QByteArray temperature = list.at(1);
    const QByteArray turbidity = list.at(2);
    const QByteArray pH = list.at(3);
    switch (waterId)
    {
    case 65:
        fixP1->setTemperatureValue(temperature);
        fixP1->setTurbidityValue(turbidity);
        fixP1->setpHValue(pH);
        break;
    case 66:
        fixP2->setTemperatureValue(temperature);
        fixP2->setTurbidityValue(turbidity);
        fixP2->setpHValue(pH);
        break;
    case 96:
        fixP3->setTemperatureValue(temperature);
        fixP3->setTurbidityValue(turbidity);
        fixP3->setpHValue(pH);
        break;
    default:
        break;
    }
}

bool sort_APointPlus(const APointPuls &a, const APointPuls &b)
{
    return a.route < b.route;
}
