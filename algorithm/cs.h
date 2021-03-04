#ifndef CS_H
#define CS_H

#include <QObject>
#include<math.h>
#include<QFile>
#include<QTextStream>

#define MAX_CITY_NUM 60

//typedef struct{
//    int code;
//    double Lng;
//    double Lat;
//}TASK;

struct PATH{
    int code[MAX_CITY_NUM];
    int route[MAX_CITY_NUM];
    int dis_alone[MAX_CITY_NUM];
    int dis;
    PATH() : dis(0)
    {
        memset(code, 0, sizeof(int)*MAX_CITY_NUM);
        memset(route, 0, sizeof(int)*MAX_CITY_NUM);
        memset(dis_alone, 0, sizeof(int)*MAX_CITY_NUM);
    }
    void clear()
    {
        dis = 0;
        memset(code, 0, sizeof(int)*MAX_CITY_NUM);
        memset(route, 0, sizeof(int)*MAX_CITY_NUM);
        memset(dis_alone, 0, sizeof(int)*MAX_CITY_NUM);
    }
};

class CS : public QObject
{
    Q_OBJECT
public:
    explicit CS(QObject *parent = nullptr);

    int get_Node_Num();
    void TSP_SA();
    void Init_City();
    void setNum(int _num);
    void getWDis0(int (&dis0)[MAX_CITY_NUM]);
    //void getDis0_end(int (&dis0_end)[MAX_CITY_NUM]);
    void Init_path(int (&_codeBuf)[MAX_CITY_NUM]);
    void transportW(int (&_W)[MAX_CITY_NUM][MAX_CITY_NUM], int index);
    PATH get_Path(int num);
//    friend void TSP_Test(CS *c);
private:
    PATH generate(PATH p);
private:
    PATH _path;
    int times = 0;
    int nodeNum = 0;
    int _UWBTaskIndex = 0;
    int _dis0[MAX_CITY_NUM];
    //int _dis0_end[MAX_CITY_NUM];
    int W[MAX_CITY_NUM][MAX_CITY_NUM];
signals:
    //void finishTSP();
};

//void TSP_Test(CS *c);

#endif // SA1_H
