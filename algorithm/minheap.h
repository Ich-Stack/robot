/*
 * 二叉堆
 * openlist储存
*/
#ifndef MINHEAP_H
#define MINHEAP_H
#include<iomanip>
#include<iostream>
#include<vector>
using namespace std;

template <typename T>
class MinHeap
{
public:
    MinHeap(){}
    //MinHeap(int capacity);
    ~MinHeap(){}

    // 删除最小堆中的data
    void remove(T index);
    // 将data插入到二叉堆中
    void insert(const T data);
    // 寻找元素
    // 返回data在二叉堆中的索引
    T inList(const T &data) const;
    T minPoint();
    bool empty();
    // 打印二叉堆
    //void print();
private:
    vector<T> mHeap;        // 数据

private:
    // 最小堆的向下调整算法
    void filterdown(int start, int end);
    // 最小堆的向上调整算法(从start开始向上直到0，调整堆)
    void filterup(int start);
};

template <typename T>
T MinHeap<T>::inList(const T &data) const
{
    T flags = NULL;
    for (T &p: mHeap)
    {
        if(!flags)
        {
            flags = (p == data);
        }
        else
        {
            return flags;
        }
    }
    return NULL;
}

template <typename T>
void MinHeap<T>::filterup(int start)
{
    int c = start;            // 当前节点(current)的位置
    int p = (c - 1) / 2;        // 父(parent)结点的位置
    T tmp = mHeap[c];        // 当前节点(current)的大小

    while (c > 0)
    {
        if (mHeap[p] <= tmp)
            break;
        else
        {
            mHeap[c] = mHeap[p];
            c = p;
            p = (p - 1) / 2;
        }
    }
    mHeap[c] = tmp;
}

template <typename T>
void MinHeap<T>::filterdown(int start, int end)
{
    int c = start;          // 当前(current)节点的位置
    int l = 2 * c + 1;     // 左(left)孩子的位置
    T tmp = mHeap[c];    // 当前(current)节点的大小

    while (l <= end)
    {
        // "l"是左孩子，"l+1"是右孩子
        if (l < end && mHeap[l] > mHeap[l + 1])
            l++;        // 左右两孩子中选择较小者，即mHeap[l+1]
        if (tmp <= mHeap[l])
            break;        //调整结束
        else
        {
            mHeap[c] = mHeap[l];
            c = l;
            l = 2 * l + 1;
        }
    }
    mHeap[c] = tmp;
}

template <typename T>
void MinHeap<T>::remove(T index)
{
    mHeap[index] = mHeap[mHeap.size() - 1];        // 用最后元素填补
    filterdown(index, static_cast<int>(mHeap.size() - 1));    // 从index号位置开始自上向下调整为最小堆

    return;
}

template <typename T>
void MinHeap<T>::insert(const T data)
{
    mHeap.push_back(data);        // 将"数组"插在表尾
    filterup(static_cast<int>(mHeap.size() - 1));            // 向上调整堆
}

template<typename T>
bool MinHeap<T>::empty()
{
    return mHeap.empty();
}
template<typename T>
T MinHeap<T>::minPoint()
{
    return mHeap.front();
}

#endif // MINHEAP_H
