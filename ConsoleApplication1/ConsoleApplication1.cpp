// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//
//-----------------------------------------------------------------------------------------------------------------------------------------
//多线程安全：https://blog.csdn.net/morewindows/article/details/7392749	（秒杀多线程）
//-----------------------------------------------------------------------------------------------------------------------------------------
//互斥
//CRITICAL_SECTION			线程拥有权，同一线程可重复进入，引用计数会累加。多核情况下配合旋转锁可提升效率。
//mutex								线程拥有权。内核对象，可跨进程互斥。”遗弃“特性，只有mutex有遗弃特性。原因：线程拥有权的
//													内核对象，当占用此mutex的线程在release之意外结束，系统会检测此线程是否占用mutex并重
//													置，event、signal无线程拥有权，无法检测此特性。
//因为已占用线程可重入，不能保证不同线程的有序执行，所以不能实现同步。
//-----------------------------------------------------------------------------------------------------------------------------------------
//同步
//event									手动置位事件和自动置位事件（WaitForSingleObject)。
//Semaphore						.......
//-----------------------------------------------------------------------------------------------------------------------------------------
//互斥、同步				同步、异步			深入理解
//2．线程（进程）之间的制约关系？当线程并发执行时，由于资源共享和线程协作，使用线程之间会存在以下两种制约关系。
//（1）．间接相互制约。一个系统中的多个线程必然要共享某种系统资源，如共享CPU，共享I / O设备，所谓间接相互制约即源于
//这种资源共享，打印机就是最好的例子，线程A在使用打印机时，其它线程都要等待。
//（2）．直接相互制约。这种制约主要是因为线程之间的合作，如有线程A将计算结果提供给线程B作进一步处理，那么线程B在线
//程A将数据送达之前都将处于阻塞状态。
//		间接相互制约可以称为互斥，直接相互制约可以称为同步，对于互斥可以这样理解，线程A和线程B互斥访问某个资源则它们
//之间就会产个顺序问题——要么线程A等待线程B操作完毕，要么线程B等待线程操作完毕，这其实就是线程的同步了。因此同步
//包括互斥，互斥其实是一种特殊的
//		个人补充：同步可以实现互斥；
//		互斥是不同线程执行相同代码（eg：临界区、关键段）(不完全是-2018/9/29)，同步是不同线程执行不同代码。
//		同步异步理解：同步是CPU同时只执行一个堆栈帧，异步是CPU同时执行多个堆栈帧。
//-----------------------------------------------------------------------------------------------------------------------------------------
//PV操作：https://blog.csdn.net/bcbobo21cn/article/details/71154339
//
//
//-----------------------------------------------------------------------------------------------------------------------------------------
#include "stdafx.h"


//经典线程同步互斥问题
#include <stdio.h>
#include <process.h>
#include <windows.h>


long g_nNum; //全局资源,保存正在度取的线程数
unsigned int __stdcall WriteFun(void *pPM);
unsigned int __stdcall ReadFun(void *pPM);
const int THREAD_NUM = 10; //子线程个数

HANDLE g_event;//管理读写互斥
CRITICAL_SECTION g_sc;//管理读者计数的互斥

int main()
{
	g_event = CreateEvent(NULL, FALSE, TRUE, NULL);
	InitializeCriticalSection(&g_sc);

	g_nNum = 0;

	HANDLE  handle_read = (HANDLE)_beginthreadex(NULL, 0, WriteFun, 0, 0, NULL);
	Sleep(500);
	HANDLE  handle[THREAD_NUM];
	int i = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, ReadFun, &i, 0, NULL);
		i++;
	}

	//保证子线程已全部运行结束
	WaitForSingleObject(handle_read, INFINITY);
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
	getchar();
	return 0;
}
//读写问题个人分析及实现
//核心：读与写互斥，通过event完成
//关键段实现对读者计数的互斥
unsigned int __stdcall WriteFun(void *pPM)
{
	//SetEvent(g_event);
	WaitForSingleObject(g_event,INFINITE);
	printf("write start\n");
	Sleep(5000);//write process
	printf("write over\n");
	SetEvent(g_event);
	return 0;
}
unsigned int __stdcall ReadFun(void *pPM)
{
	EnterCriticalSection(&g_sc);
	if (0 == g_nNum)
	{
		WaitForSingleObject(g_event, INFINITE);
	}
	g_nNum++;
	LeaveCriticalSection(&g_sc);

	printf("read thread NUM = %ld\n", g_nNum);
	Sleep(5000);//read process

	EnterCriticalSection(&g_sc);
	g_nNum--;
	if (0 == g_nNum)
	{
		SetEvent(g_event);
	}
	LeaveCriticalSection(&g_sc);
	return 0;
}