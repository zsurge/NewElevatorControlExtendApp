/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : quickSort.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年12月1日
  最近修改   :
  功能描述   : 非递归快速排序法
  函数列表   :
  修改历史   :
  1.日    期   : 2020年12月1日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "quickSort.h"
#include "malloc.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
/*最大栈长度*/
#define STACK_SIZE 8 * sizeof(unsigned int)
#define MAX_THRESH 4

/*入栈，出栈*/
#define STACK_PUSH( low, hig )    ( (top->lo = low), (top->hi = hig), top++)
#define STACK_POP( low, hig )    (top--, (low = top->lo), (hig = top->hi) )

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/*存储区间信息*/
typedef struct stack_node_t
{
    int lo;
    int hi;
}struct_node;



/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void insertSort(ElementType A[], int N);
void swap(ElementType *a, ElementType *b);
ElementType medianPivot(ElementType A[], int left, int right);
int partition(ElementType A[], int left, int right);

/*元素交换*/
void swap(ElementType *a,ElementType *b)
{
    ElementType temp = *a;
    *a = *b;
    *b = temp;
}
/*插入排序*/
void insertSort(ElementType A[],int N)
{
    /*优化后的插入排序*/
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
/*三数中值法选择基准*/
ElementType medianPivot(ElementType A[],int left,int right)
{
    int center = (left + right)/2 ;
    /*对三数进行排序*/
    if(A[left] > A[center])
        swap(&A[left],&A[center]);
    if(A[left] > A[right])
        swap(&A[left],&A[right]);
    if(A[center] > A[right])
        swap(&A[center],&A[right]);
    
    /*交换中值和倒数第二个元素*/    
    swap(&A[center],&A[right-1]);
    return A[right-1];
}

/*分区操作*/
int partition(ElementType A[],int left,int right)
{
   
    int i = left;
    int j = right-1;
    /*获取基准值*/
    ElementType pivot = medianPivot(A,left,right);
    for(;;)
    {
        /*i j分别向右和向左移动，为什么不直接先i++？*/
        while(A[++i] < pivot)
        {}
        while(A[--j] > pivot)
        {}
        
        if(i < j)
        {
            swap(&A[i],&A[j]);
        }
        /*交错时停止*/
        else
        {
            break;
        }

    }
    /*交换基准元素和i指向的元素*/
    swap(&A[i],&A[right-1]);
    return i;
    
}

/*快速排序*/
void quickSortNor(void *A, int left, int right)
{
    if(NULL == A)
        return;
    /*使用寄存器指针*/
    register ElementType *arr = (ElementType *)A;
    if ( right - left >= MAX_THRESH )
    {
        struct_node    stack[STACK_SIZE]    = { { 0 } };
        register struct_node    *top            = stack;

        /*最大区间压栈*/
        int lo = left;
        int hi = right;
        STACK_PUSH( 0, 0);
        int mid = 0;
        while ( stack < top )
        {
            /*出栈，取出一个区间进行分区操作*/
            
            mid = partition( arr, lo, hi );

            /*分情况处理，左边小于阈值*/
            if ( (mid - 1 - lo) <= MAX_THRESH)
            {
                /* 左右两个数据段的元素都小于阈值，取出栈中数据段进行划分*/
                if ( (hi - (mid+1)) <= MAX_THRESH)
                    /* 都小于阈值，从栈中取出数据段 */
                    STACK_POP (lo, hi);
                else
                    /* 只有右边大于阈值，右边继续分区*/
                    lo =  mid -1 ;
            }
            /*右边小于阈值，继续计算左边*/
            else if ((hi - (mid+1)) <= MAX_THRESH)
                hi = mid - 1;
            /*左右两边都大于阈值，且左边大于右边，左边入栈，右边继续分区*/
            else if ((mid -1 - lo) > (hi - (mid + 1)))
            {
                STACK_PUSH (lo, mid - 1);
                lo = mid + 1;
            }
            /*左右两边都大于阈值，且右边大于左边，右边入栈，左边继续分区*/
            else
            {
                STACK_PUSH (mid + 1, hi);
                hi = mid  -1;
            }
        }

    }

    /*最后再使用插入排序，对于接近有序状态的数据，插入排序速度很快*/
    insertSort(arr,right-left+1);
    
}


