#include "astar.h"
#include<QFile>
#include<QTextStream>
#include<QDebug>

#define PRECISION 0.0001

ASTAR::ASTAR()
{
    //minheap_openlist = new MinHeap<APoint*>();
    openList.clear();
    closeList.clear();
}

void ASTAR::InitAstar(std::vector<std::vector<bool>>& _maze)
{
    maze = _maze;
    QFile  myfile("maze.txt");//创建一个输出文件的文档
    if (myfile.open(QFile::WriteOnly|QFile::Truncate))//注意WriteOnly是往文本中写入的时候用，ReadOnly是在读文本中内容的时候用，Truncate表示将原来文件中的内容清空
    {
        //读取之前setPlainText的内容，或直接输出字符串内容QObject::tr()
        QTextStream out(&myfile);
        for(int i = 0; i < max_map_num; i++)
        {
            for(int j = 0; j < max_map_num; j++)
            {
                out<<maze[i][j];
            }
            out << endl;
        }
        out << maze.size() << ',' << maze[0].size();
    }
}

int ASTAR::calcG(APoint* temp_start, APoint* point)
{
    int extraG = (abs(point->x - temp_start->x) + abs(point->y - temp_start->y)) == 1 ? kCost1 : kCost2;
    int parentG = point->parent == NULL ? 0 : point->parent->G; //如果是初始节点，则其父节点是空
     return parentG + extraG;
//    if(parentG != extraG && parentG != 0)
//    {
//        extraG += 5;                                           //惩罚，使路径更平滑
//    }
}

int ASTAR::calcH(APoint* point, APoint* end)
{
    //http://theory.stanford.edu/~amitp/GameProgramming/    原文地址
    int dx = abs(point->x - end->x);
    int dy = abs(point->y - end->y);
    return kCost1 * (dx + dy) + (kCost2 - 2 * kCost1) * fmin(dx, dy);//切比雪夫距离
    //计算欧几里得距离H
    //return (int)(abs(end->x - point->x)+abs(end->y - point->y)) * kCost1;
//    int dx = pow(end->x - point->x, 2);
//    int dy = pow(end->y - point->y, 2);
//    return static_cast<int>(sqrt(dx + dy) * kCost1);
}

int ASTAR::calcF(APoint* point)
{
    return point->G + point->H;
}

APoint* ASTAR::getLeastFpoint()
{
    if (!openList.empty())
    {
        auto resPoint = openList.front();
        for (auto& point : openList)
            if (point->F < resPoint->F)
                resPoint = point;
        return resPoint;
    }
    return NULL;
}

APoint* ASTAR::findPath(APoint& startPoint, APoint& endPoint, bool isIgnoreCorner)
{
    openList.push_back(new APoint(startPoint.x, startPoint.y)); //置入起点,拷贝开辟一个节点，内外隔离
    while (!openList.empty())
    {
        auto curPoint = getLeastFpoint(); //找到F值最小的点
        openList.remove(curPoint); //从开启列表中删除
        closeList.push_back(curPoint); //放到关闭列表
        //找到当前周围八个格中可以通过的格子
        auto surroundPoints = getSurroundPoints(curPoint, isIgnoreCorner);
        for (auto& target : surroundPoints)
        {
            //对某一个格子，如果它不在开启列表中，加入到开启列表，设置当前格为其父节点，计算F G H
            if (!isInList(openList, target))
            {
                target->parent = curPoint;

                target->G = calcG(curPoint, target);
                target->H = calcH(target, &endPoint);
                target->F = calcF(target);

                openList.push_back(target);
            }
            //对某一个格子，它在开启列表中，计算G值, 如果比原来的大, 就什么都不做, 否则设置它的父节点为当前点,并更新G和F
            else
            {
                int tempG = calcG(curPoint, target);
                if (tempG < target->G)
                {
                    target->parent = curPoint;

                    target->G = tempG;
                    target->F = calcF(target);
                }
            }
            APoint* resPoint = isInList(openList, &endPoint);
            if (resPoint)
                return resPoint; //返回列表里的节点指针，不要用原来传入的endpoint指针，因为发生了深拷贝
        }
    }
    return NULL;
}

std::list<QPoint> ASTAR::GetPath(APoint &startPoint, APoint &endPoint, bool isIgnoreCorner)
{
    //qDebug() << "start point : " << '(' << startPoint.x << ", " << startPoint.y << ')';
    APoint* result = findPath(startPoint, endPoint, isIgnoreCorner);
    APoint* tempPoint = result;                                                             //保存斜率改变时的节点
    APoint* buf = NULL;                                                                     //记录方向改变时的上一个点，防止与可行区域相交
    double slopeParent = 0;                                                                    //保存斜率改变时的斜率
    std::list<QPoint> path;//(1, result->operator QPoint());
    //返回路径，如果没找到路径，返回空链表
    path.push_front(result->operator QPoint());                                                                //加入第一个节点，因为计算从第二个节点开始
    slopeParent = calcSlope(*result, *(result->parent));                                    //计算第一第二个节点的斜率，作为初始斜率
//    while(result)
//    {
    while (result->parent)
    {
        buf = result;
        result = result->parent;                                                                    //指向下一个节点，遍历所有节点
        const double slope = calcSlope(*tempPoint, *result);                                                 //计算上次斜率改变的节点与当前遍历节点的斜率
        if(abs(slope - slopeParent) < 1e-6)                                                                    //比较斜率，判断是否在一条直线上，如果是则判断下一个节点，否则奖当前节点加入列表
        {
            continue;
        }
        else
        {
            path.push_front(buf->operator QPoint());
            tempPoint = result;                                                                     //斜率改变时，记录当前斜率
            slopeParent = slope;                                                                    //斜率改变时，记录当前节点
        }
        //path.push_front(result->operator QPoint());
        //result = result->parent;
    }
    path.push_front(result->operator QPoint());                                                                        //将最后一个节点加入列表中
    // 清空临时开闭列表，防止重复执行GetPath导致结果异常
    openList.clear();
    closeList.clear();

    return path;
}

APoint* ASTAR::isInList(const std::list<APoint*>& list, const APoint* point) const
{
    //判断某个节点是否在列表中，这里不能比较指针，因为每次加入列表是新开辟的节点，只能比较坐标
    for (auto p : list)
        if (p->x == point->x && p->y == point->y)
            return p;
    return NULL;
}

bool ASTAR::isCanreach(const APoint* point, const APoint* target, bool isIgnoreCorner) const
{
    if (target->x<0 || (unsigned long)target->x>maze.size() - 1
        || target->y<0 || (unsigned long)target->y>maze[0].size() - 1
        || maze[target->y][target->x] == true
        || (target->x == point->x && target->y == point->y)
        || isInList(closeList, target)) //如果点与当前节点重合、超出地图、是障碍物、或者在关闭列表中，返回false
        return false;
    else
    {
        if (abs(point->x - target->x) + abs(point->y - target->y) == 1) //非斜角可以
            return true;
        else
        {
            //斜对角要判断是否绊住
            if (maze[point->x][target->y] == false && maze[target->x][point->y] == false)
                return true;
            else
                return isIgnoreCorner;
        }
    }
}

std::vector<APoint*> ASTAR::getSurroundPoints(const APoint* point, bool isIgnoreCorner) const
{
    std::vector<APoint*> surroundPoints;
    surroundPoints.clear();

    for (int x = point->x - 1; x <= point->x + 1; x++)
        for (int y = point->y - 1; y <= point->y + 1; y++)
            if (isCanreach(point, new APoint(x, y), isIgnoreCorner))
                surroundPoints.push_back(new APoint(x, y));

    return surroundPoints;
}

int ASTAR::getG(APoint& startPoint, APoint& endPoint, bool isIgnoreCorner)
{
    openList.clear();                                       //清空列表，不然只能寻找一次路径
    closeList.clear();
    openList.push_back(new APoint(startPoint.x, startPoint.y)); //置入起点,拷贝开辟一个节点，内外隔离
    while (!openList.empty())
    {
        auto curPoint = getLeastFpoint(); //找到F值最小的点
        openList.remove(curPoint); //从开启列表中删除
        closeList.push_back(curPoint); //放到关闭列表
        //找到当前周围八个格中可以通过的格子
        auto surroundPoints = getSurroundPoints(curPoint, isIgnoreCorner);
        for (auto& target : surroundPoints)
        {
            //对某一个格子，如果它不在开启列表中，加入到开启列表，设置当前格为其父节点，计算F G H
            if (!isInList(openList, target))
            {
                target->parent = curPoint;
                target->G = calcG(curPoint, target);
                target->H = calcH(target, &endPoint);
                target->F = calcF(target);
                openList.push_back(target);
            }
            //对某一个格子，它在开启列表中，计算G值, 如果比原来的大, 就什么都不做, 否则设置它的父节点为当前点,并更新G和F
            else
            {
                int tempG = calcG(curPoint, target);
                if (tempG < target->G)
                {
                    target->parent = curPoint;
                    target->G = tempG;
                    target->F = calcF(target);
                }
            }
            APoint* resPoint = isInList(openList, &endPoint);
            if (resPoint)
            {
                return target->G;
            }
        }
    }
    return NULL;
}

const double ASTAR::calcSlope(const APoint &a, const APoint &b)                                //计算斜率函数
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    if(dx)                                                                                 //分母为0返回0
    {
        return dy / dx;
    }
    else
    {
        return 0;
    }
}

//const std::list<APoint*> ASTAR::floyd(const std::list<APoint*> &path)
//{
//    if(path.empty())
//    {
//        return path;
//    }
//    else
//    {
//        int len = path.size();
//        if(len > 2)
//        {

//        }
//    }
//}
