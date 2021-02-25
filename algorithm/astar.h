#ifndef ASTAR_H
#define ASTAR_H

/*
//A*算法对象类
*/
#include<iostream>
#include<vector>
#include<list>
#include"minheap.h"
#include<QPoint>

const int kCost1 = 10; //直移一格消耗
const int kCost2 = 14; //斜移一格消耗

struct APoint
{
    int x, y; //点坐标，这里为了方便按照C++的数组来计算，x代表横排，y代表竖列
    int F, G, H; //F=G+H
    int taskContantIndex;
    double omega;
    APoint* parent; //parent的坐标，这里没有用指针，从而简化代码
    APoint(int _x, int _y, double _omega, int _taskContantIndex) :x(_x), y(_y), omega(_omega), taskContantIndex(_taskContantIndex), F(0), G(0), H(0), parent(NULL){}//变量初始化
    APoint(int _x, int _y) :x(_x), y(_y), omega(1), taskContantIndex(0), F(0), G(0), H(0), parent(NULL){}//变量初始化
    APoint():x(0), y(0), F(0), G(0), H(0), omega(1), taskContantIndex(0), parent(NULL){}
    //explicit operator QPoint(){ return QPoint(x, y); }
    bool operator ==(const APoint &rp) const
    {
        return !((x ^ rp.x) || (y ^ rp.y));
//        bool result;
//        result = this->x == rp.x && this->y == rp.y;
//        return result;
    }
    QPoint toQPoint() const
    {
        return QPoint(x, y);
    }
};

class ASTAR
{
public:
    ASTAR();
    void InitAstar(std::vector<std::vector<bool>>& _maze);
    int getG(APoint& startPoint, APoint& endPoint, bool isIgnoreCorner);
    std::list<QPoint> GetPath(APoint& startPoint, APoint& endPoint, bool isIgnoreCorner);
private:
    APoint* findPath(APoint& startPoint, APoint& endPoint, bool isIgnoreCorner);
    std::vector<APoint*> getSurroundPoints(const APoint* point, bool isIgnoreCorner) const;
    bool isCanreach(const APoint* point, const APoint* target, bool isIgnoreCorner) const; //判断某点是否可以用于下一步判断
    APoint* isInList(const std::list<APoint*>& list, const APoint* point) const; //判断开启/关闭列表中是否包含某点
    APoint* getLeastFpoint(); //从开启列表中返回F值最小的节点
    //计算FGH值
    int calcG(APoint* temp_start, APoint* point);
    int calcH(APoint* point, APoint* end);
    int calcF(APoint* point);
    //弗洛伊德平滑优化
    //const std::list<APoint*> floyd(const std::list<APoint*> &path);
    const double calcSlope(const APoint &a, const APoint &b);                        //计算斜率函数
private:
    const unsigned char max_map_num = 60;
    //MinHeap<APoint*> *minheap_openlist;                                         //二元堆对象
    std::vector<std::vector<bool>> maze;
    std::list<APoint*> openList;  //开启列表
    std::list<APoint*> closeList; //关闭列表
};

#endif // ASTAR_H
