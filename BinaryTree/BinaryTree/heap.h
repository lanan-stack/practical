#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>
//˳���ʵ��
typedef int HeapDataType;
typedef struct Heap {
    HeapDataType* arr;//����
    int size;//ʵ�ʸ���
    int capacity;//����
}Heap;

void swap(int* x, int* y);//����
void HeapInit(Heap*hp);//��ʼ��
void HeapDestroy(Heap* hp);//����
void HeapPush(Heap* hp, HeapDataType x);//����
void HeapAdjustUp(Heap* hp, int child);//���ϵ����㷨��childΪҪ������λ��
void HeapAdjustDown(Heap*hp,int parent);//���µ����㷨��parentΪҪ������λ��
bool HeapEmpty(Heap* hp);//�п�
void HeapPop(Heap*hp);//ɾ��