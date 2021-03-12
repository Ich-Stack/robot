#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")                       //解决中文乱码问题
#endif
#include "cs.h"
//#include<iostream>
//#include<QThread>
#include <QDebug>

const int ITERATIONS = 200;                 //迭代次数
const double ALPHA = 0.98;					//退火速度
const double INITIAL_TEMPERATURE = 2000.0;	//初始温度
const double MIN_TEMPERATURE = 1e-5;		//最低温度

CS::CS(QObject *parent) : QObject(parent)
{
    memset(_dis0, 0, sizeof(int)*MAX_CITY_NUM);
}

void CS::Init_path(int (&_codeBuf)[MAX_CITY_NUM])                               //初始化路径
{
    _path.dis = 1e6;
    for(int i = 0; i < MAX_CITY_NUM; i++)                                       //任务代号数据传输
    {
        _path.code[i] = _codeBuf[i];
    }
    for (int i = 0; i < nodeNum; ++i)                                           //初始化城市
    {
        _path.route[i] = i;
    }
}
void CS::TSP_SA()
{
    double t = INITIAL_TEMPERATURE;
    srand(time(NULL));
    PATH cur = _path;
    PATH next = _path;
    bool bChange;
    if(_UWBTaskIndex < 2)
    {
        _path.dis = _dis0[0];
        return;
    }
    while (t > MIN_TEMPERATURE)                                             //温度
    {
        bChange = false;
        for (int i = 0; i < ITERATIONS; ++i)                                //迭代次数
        {
            next = generate(cur);                                           //产生新路径
            int df = next.dis - cur.dis;
            if (df <= 0)                                                    //接受更短的路径
            {
                cur = next;
                bChange = true;
            }
            else
            {
                double rndp = rand() / (RAND_MAX + 1.0);                    //概率接受更差的路径
                double eps = exp(-df / t);
                if (eps > rndp&& eps < 1)
                {
                    cur = next;
                    bChange = true;
                }
            }
        }

        if (cur.dis < _path.dis)                                            //接受更优的路径
        {
            _path = cur;
            qDebug() << "dis : " << cur.dis;
        }

        t *= ALPHA;                                                         //退火速度
        if (!bChange)                                                       //两次最优路径相同或者到达迭代次数退出循环
            times++;
        else
            times = 0;

        if (times == 2)
        {
            break;
        }
    }
}
PATH CS::generate(PATH p)
{
    int x = 0, y = 0;
    while (x == y)                                                          //产生随机数，随机交换两个城市，产生新的路径
    {
        x = (int)(nodeNum * (rand() / (RAND_MAX + 1.0)));
        y = (int)(nodeNum * (rand() / (RAND_MAX + 1.0)));
    }
    int tmpRoute;
    int tmpCode;
    PATH gen = p;
    tmpRoute = gen.route[x];                                    //随机交换城市产生新解
    gen.route[x] = gen.route[y];
    gen.route[y] = tmpRoute;

    tmpCode = gen.code[x];                                      //交换城市对应的任务代号
    gen.code[x] = gen.code[y];
    gen.code[y] = tmpCode;

    gen.dis = 0;
    for (int i = 0; i < nodeNum - 1; i++)
    {
        int WBuf = W[gen.route[i]][gen.route[i + 1]];
        if(WBuf < 0)
        {
            WBuf -= static_cast<int>(WBuf - (WBuf / 10 * i));
        }
        gen.dis += WBuf;           //每个任务之间做路径优化做寻找最短路径
    }
    gen.dis += _dis0[gen.route[0]];                             //任务间最短路径加上设备当前坐标与第一个目标点的距离

    return gen;
}

void CS::transportW(int (&_W)[MAX_CITY_NUM][MAX_CITY_NUM], int index)
{
    _UWBTaskIndex = index;
    QFile  myfile("W.txt");                                     //创建一个输出文件的文档
    for(int i = 0; i < MAX_CITY_NUM; i++)
    {
        for(int j = 0; j < MAX_CITY_NUM; j++)
        {
            this->W[i][j] = _W[i][j];
        }
    }
    //输出在G.txt文件中
    if (myfile.open(QFile::WriteOnly | QFile::Truncate))//注意WriteOnly是往文本中写入的时候用，ReadOnly是在读文本中内容的时候用，Truncate表示将原来文件中的内容清空
    {
        //读取之前setPlainText的内容，或直接输出字符串内容QObject::tr()
        QTextStream out(&myfile);
        for(int i = 0; i < index; i++)
        {
            for(int j = 0; j < index; j++)
            {
                out << W[i][j] << ' ';
            }
            out << endl;
        }
    }
}

PATH CS::get_Path(int num)
{
//    int temp_dis = 0;
//    int temp_code = 0;
//    int temp_route = 0;
//    for(int i = 0; i < num; i++)                                    //从小到大排序，成本越低的任务优先做
//    {
//        for(int j = i; j < num; j++)
//        {
//            if(_path.dis_alone[i] > _path.dis_alone[j])
//            {
//                temp_dis = _path.dis_alone[i];
//                _path.dis_alone[i] = _path.dis_alone[j];
//                _path.dis_alone[j] = temp_dis;

//                temp_route = _path.route[i];
//                _path.route[i] = _path.route[j];
//                _path.route[j] = temp_route;

//                temp_code = _path.code[i];
//                _path.code[i] = _path.code[j];
//                _path.code[j] = temp_code;
//            }
//        }
//    }

//    for(int i = 0; i < num; i++)
//    {
//        std::cout << "route:" << _path.route[i] << " code:" << _path.code[i] << " dis_alone:" << _path.dis_alone[i] << " dis:" << _path.dis << std::endl;
//    }

    return this->_path;
}

//void TSP_Test(CS *c)
//{
//    double t = INITIAL_TEMPERATURE;
//    srand(time(NULL));
//    PATH cur = c->_path;
//    PATH next = c->_path;
//    bool bChange;
//    if(c->_UWBTaskIndex < 2)
//    {
//        c->_path.dis = c->_dis0[0]*2;
//        return;
//    }
//    while (t > MIN_TEMPERATURE)                                             //温度
//    {
//        bChange = false;
//        for (int i = 0; i < ITERATIONS; ++i)                                //迭代次数
//        {
//            next = c->generate(cur);                                           //产生新路径
//            double df = next.dis - cur.dis;
//            if (df <= 0)                                                    //接受更短的路径
//            {
//                cur = next;
//                bChange = true;
//            }
//            else
//            {
//                double rndp = rand() / (RAND_MAX + 1.0);                    //概率接受更差的路径
//                double eps = exp(-df / t);
//                if (eps > rndp&& eps < 1)
//                {
//                    cur = next;
//                    bChange = true;
//                }
//            }
//        }

//        if (cur.dis < c->_path.dis)                                            //接受更优的路径
//            c->_path = cur;

//        t *= ALPHA;                                                         //退火速度
//        if (!bChange)                                                       //两次最优路径相同或者到达迭代次数退出循环
//            c->times++;
//        else
//            c->times = 0;

//        if (c->times == 2)
//        {
//            emit c->finishTSP();                                               //迭代次数完成或者达到特定条件触发信号并退出循环
//            break;
//        }
//    }
//    emit c->finishTSP();
//}

//接口
void CS::getWDis0(int (&dis0)[MAX_CITY_NUM])
{
    for(int i = 0; i < MAX_CITY_NUM; i++)
    {
        _dis0[i] = dis0[i];
    }
}

//void CS::getDis0_end(int (&dis0_end)[MAX_CITY_NUM])
//{
//    for(int i = 0; i < MAX_CITY_NUM; i++)
//    {
//        _dis0_end[i] = dis0_end[i];
//    }
//}

void CS::setNum(int _num)
{
    this->nodeNum = _num;
}

int CS::get_Node_Num()
{
    return this->nodeNum;
}
