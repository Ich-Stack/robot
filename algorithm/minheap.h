/*
 * �����
 * openlist����
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

    // ɾ����С���е�data
    void remove(T index);
    // ��data���뵽�������
    void insert(const T data);
    // Ѱ��Ԫ��
    // ����data�ڶ�����е�����
    T inList(const T &data) const;
    T minPoint();
    bool empty();
    // ��ӡ�����
    //void print();
private:
    vector<T> mHeap;        // ����

private:
    // ��С�ѵ����µ����㷨
    void filterdown(int start, int end);
    // ��С�ѵ����ϵ����㷨(��start��ʼ����ֱ��0��������)
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
    int c = start;            // ��ǰ�ڵ�(current)��λ��
    int p = (c - 1) / 2;        // ��(parent)����λ��
    T tmp = mHeap[c];        // ��ǰ�ڵ�(current)�Ĵ�С

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
    int c = start;          // ��ǰ(current)�ڵ��λ��
    int l = 2 * c + 1;     // ��(left)���ӵ�λ��
    T tmp = mHeap[c];    // ��ǰ(current)�ڵ�Ĵ�С

    while (l <= end)
    {
        // "l"�����ӣ�"l+1"���Һ���
        if (l < end && mHeap[l] > mHeap[l + 1])
            l++;        // ������������ѡ���С�ߣ���mHeap[l+1]
        if (tmp <= mHeap[l])
            break;        //��������
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
    mHeap[index] = mHeap[mHeap.size() - 1];        // �����Ԫ���
    filterdown(index, static_cast<int>(mHeap.size() - 1));    // ��index��λ�ÿ�ʼ�������µ���Ϊ��С��

    return;
}

template <typename T>
void MinHeap<T>::insert(const T data)
{
    mHeap.push_back(data);        // ��"����"���ڱ�β
    filterup(static_cast<int>(mHeap.size() - 1));            // ���ϵ�����
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
