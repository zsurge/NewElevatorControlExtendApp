/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : quickSort.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��12��1��
  ����޸�   :
  ��������   : �ǵݹ��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��12��1��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "quickSort.h"
#include "malloc.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/*���ջ����*/
#define STACK_SIZE 8 * sizeof(unsigned int)
#define MAX_THRESH 4

/*��ջ����ջ*/
#define STACK_PUSH( low, hig )    ( (top->lo = low), (top->hi = hig), top++)
#define STACK_POP( low, hig )    (top--, (low = top->lo), (hig = top->hi) )

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
/*�洢������Ϣ*/
typedef struct stack_node_t
{
    int lo;
    int hi;
}struct_node;



/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
void insertSort(ElementType A[], int N);
void swap(ElementType *a, ElementType *b);
ElementType medianPivot(ElementType A[], int left, int right);
int partition(ElementType A[], int left, int right);

/*Ԫ�ؽ���*/
void swap(ElementType *a,ElementType *b)
{
    ElementType temp = *a;
    *a = *b;
    *b = temp;
}
/*��������*/
void insertSort(ElementType A[],int N)
{
    /*�Ż���Ĳ�������*/
    int j = 0;
    int p = 0;
    int temp = 0;
    for(p = 1;p<N;p++)
    {
        temp = A[p];
        for(j = p;j>0 && A[j-1] > temp;j--)
        {
            A[j] = A[j-1];
        }
        A[j] = temp;
    }

}
/*������ֵ��ѡ���׼*/
ElementType medianPivot(ElementType A[],int left,int right)
{
    int center = (left + right)/2 ;
    /*��������������*/
    if(A[left] > A[center])
        swap(&A[left],&A[center]);
    if(A[left] > A[right])
        swap(&A[left],&A[right]);
    if(A[center] > A[right])
        swap(&A[center],&A[right]);
    
    /*������ֵ�͵����ڶ���Ԫ��*/    
    swap(&A[center],&A[right-1]);
    return A[right-1];
}

/*��������*/
int partition(ElementType A[],int left,int right)
{
   
    int i = left;
    int j = right-1;
    /*��ȡ��׼ֵ*/
    ElementType pivot = medianPivot(A,left,right);
    for(;;)
    {
        /*i j�ֱ����Һ������ƶ���Ϊʲô��ֱ����i++��*/
        while(A[++i] < pivot)
        {}
        while(A[--j] > pivot)
        {}
        
        if(i < j)
        {
            swap(&A[i],&A[j]);
        }
        /*����ʱֹͣ*/
        else
        {
            break;
        }

    }
    /*������׼Ԫ�غ�iָ���Ԫ��*/
    swap(&A[i],&A[right-1]);
    return i;
    
}

/*��������*/
void quickSortNor(void *A, int left, int right)
{
    if(NULL == A)
        return;
    /*ʹ�üĴ���ָ��*/
    register ElementType *arr = (ElementType *)A;
    if ( right - left >= MAX_THRESH )
    {
        struct_node    stack[STACK_SIZE]    = { { 0 } };
        register struct_node    *top            = stack;

        /*�������ѹջ*/
        int lo = left;
        int hi = right;
        STACK_PUSH( 0, 0);
        int mid = 0;
        while ( stack < top )
        {
            /*��ջ��ȡ��һ��������з�������*/
            
            mid = partition( arr, lo, hi );

            /*������������С����ֵ*/
            if ( (mid - 1 - lo) <= MAX_THRESH)
            {
                /* �����������ݶε�Ԫ�ض�С����ֵ��ȡ��ջ�����ݶν��л���*/
                if ( (hi - (mid+1)) <= MAX_THRESH)
                    /* ��С����ֵ����ջ��ȡ�����ݶ� */
                    STACK_POP (lo, hi);
                else
                    /* ֻ���ұߴ�����ֵ���ұ߼�������*/
                    lo =  mid -1 ;
            }
            /*�ұ�С����ֵ�������������*/
            else if ((hi - (mid+1)) <= MAX_THRESH)
                hi = mid - 1;
            /*�������߶�������ֵ������ߴ����ұߣ������ջ���ұ߼�������*/
            else if ((mid -1 - lo) > (hi - (mid + 1)))
            {
                STACK_PUSH (lo, mid - 1);
                lo = mid + 1;
            }
            /*�������߶�������ֵ�����ұߴ�����ߣ��ұ���ջ����߼�������*/
            else
            {
                STACK_PUSH (mid + 1, hi);
                hi = mid  -1;
            }
        }

    }

    /*�����ʹ�ò������򣬶��ڽӽ�����״̬�����ݣ����������ٶȺܿ�*/
    insertSort(arr,right-left+1);
    
}


