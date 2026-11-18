#include"heap.h"

void swap(int* x, int* y)//���յ�ַ������
{
	/*int temp = *x;
	*x = *y;
	*y = temp;*/
	*x = *x ^ *y;
	*y = *x ^ *y;
	*x = *x ^ *y;

}

void HeapInit(Heap*hp)//��ʼ��
{
	assert(hp);
	hp->arr = (HeapDataType*)malloc(sizeof(HeapDataType) * 4);
	hp->size = 0;
	hp->capacity = 4;
	
}

void HeapDestroy(Heap* hp)//����
{
	assert(hp);
	free(hp->arr);
	hp->capacity = 0;
	hp->size = 0;
}

void HeapAdjustUp(Heap* hp, int child)//���ϵ����㷨
{
	assert(hp);
	int parent = (child - 1) / 2;
	while (child > 0)
	{
		if (hp->arr[child] < hp->arr[parent])
		{
			swap(&hp->arr[child], &hp->arr[parent]);
			child = parent;
			parent = (child - 1) / 2;
		}
		else
		{
			break;
		}
	}
}

void HeapPush(Heap* hp, HeapDataType x)//����
{
	assert(hp);
	if (hp->size == hp->capacity)
	{
		//����
		HeapDataType* temp = (HeapDataType*)realloc(hp->arr, sizeof(HeapDataType) * 2 * hp->capacity);
		if (temp == NULL)
		{
			perror("realloc failed");
			return;
		}
		//����ɹ�
		hp->arr = temp;
		hp->capacity *= 2;
	}
	//�ռ��㹻
	hp->arr[hp->size] = x;
	HeapAdjustUp(hp, hp->size);
	hp->size++;

}

void HeapPrint(Heap* hp)//��ӡ
{
	assert(hp);
	for (int i = 0; i < hp->size; i++)
	{
		printf("%d ", hp->arr[i]);
	}
	printf("\n");
}

void HeapAdjustDown(Heap* hp, int parent)//���µ����㷨
{
	assert(hp);
	int child = parent * 2 + 1;
	while (child < hp->size)
	{
		if (child + 1 < hp->size && hp->arr[child+1] < hp->arr[child])//���Һ��Ӳ����ұ߸�С
		{
			child++;
		}
		if (hp->arr[child] < hp->arr[parent])
		{
			swap(&hp->arr[child], &hp->arr[parent]);
			parent = child;
			child = parent * 2 + 1;
		}
		else
		{
			break;
		}
	}
}

bool HeapEmpty(Heap* hp)//�п�
{
	if (hp->size == 0)//��
	{
		return true;
	}
	else
	{
		return false;
	}
}

void HeapPop(Heap* hp)//ɾ���Ѷ�����
{
	assert(hp && !HeapEmpty(hp));
	swap(&hp->arr[0], &hp->arr[hp->size - 1]);
	hp->size--;
	HeapAdjustDown(hp, 0);
}

