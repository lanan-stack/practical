#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef int DCLDataType;


typedef struct DCListNode {
	DCLDataType data;//数据
	struct DCListNode* prev;//指向前面的结点
	struct DCListNode* next;//指向后面的结点
	int list_length;
}DCListNode;



void DCListInit1(DCListNode** dcl);//初始化1
void DCListDestory(DCListNode**DCL);//销毁
void DCLPrint(DCListNode*DCL);//打印
void DCLPushFront(DCListNode*DCL,DCLDataType x);//头插
void DCLPushBack(DCListNode* DCL, DCLDataType x);//尾插
void DCLPopFront(DCListNode* DCL);//头删
void DCLGetElement(DCListNode*DCL,int i );//获取指定位置的节点
void DCLInsert(DCListNode* DCL, int i, DCLDataType x);//在第i个节点后插入节点
void DCLDelete(DCListNode* DCL,int i);//删除指定位置的节点