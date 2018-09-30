// ConsoleApplication1.cpp : �������̨Ӧ�ó������ڵ㡣
//
//-----------------------------------------------------------------------------------------------------------------------------------------
//���̰߳�ȫ��https://blog.csdn.net/morewindows/article/details/7392749	����ɱ���̣߳�
//-----------------------------------------------------------------------------------------------------------------------------------------
//����
//CRITICAL_SECTION			�߳�ӵ��Ȩ��ͬһ�߳̿��ظ����룬���ü������ۼӡ��������������ת��������Ч�ʡ�
//mutex								�߳�ӵ��Ȩ���ں˶��󣬿ɿ���̻��⡣�����������ԣ�ֻ��mutex���������ԡ�ԭ���߳�ӵ��Ȩ��
//													�ں˶��󣬵�ռ�ô�mutex���߳���release֮���������ϵͳ������߳��Ƿ�ռ��mutex����
//													�ã�event��signal���߳�ӵ��Ȩ���޷��������ԡ�
//��Ϊ��ռ���߳̿����룬���ܱ�֤��ͬ�̵߳�����ִ�У����Բ���ʵ��ͬ����
//-----------------------------------------------------------------------------------------------------------------------------------------
//ͬ��
//event									�ֶ���λ�¼����Զ���λ�¼���WaitForSingleObject)��
//Semaphore						.......
//-----------------------------------------------------------------------------------------------------------------------------------------
//���⡢ͬ��				ͬ�����첽			�������
//2���̣߳����̣�֮�����Լ��ϵ�����̲߳���ִ��ʱ��������Դ������߳�Э����ʹ���߳�֮����������������Լ��ϵ��
//��1��������໥��Լ��һ��ϵͳ�еĶ���̱߳�ȻҪ����ĳ��ϵͳ��Դ���繲��CPU������I / O�豸����ν����໥��Լ��Դ��
//������Դ������ӡ��������õ����ӣ��߳�A��ʹ�ô�ӡ��ʱ�������̶߳�Ҫ�ȴ���
//��2����ֱ���໥��Լ��������Լ��Ҫ����Ϊ�߳�֮��ĺ����������߳�A���������ṩ���߳�B����һ��������ô�߳�B����
//��A�������ʹ�֮ǰ������������״̬��
//		����໥��Լ���Գ�Ϊ���⣬ֱ���໥��Լ���Գ�Ϊͬ�������ڻ������������⣬�߳�A���߳�B�������ĳ����Դ������
//֮��ͻ����˳�����⡪��Ҫô�߳�A�ȴ��߳�B������ϣ�Ҫô�߳�B�ȴ��̲߳�����ϣ�����ʵ�����̵߳�ͬ���ˡ����ͬ��
//�������⣬������ʵ��һ�������
//		���˲��䣺ͬ������ʵ�ֻ��⣻
//		�����ǲ�ͬ�߳�ִ����ͬ���루eg���ٽ������ؼ��Σ�(����ȫ��-2018/9/29)��ͬ���ǲ�ͬ�߳�ִ�в�ͬ���롣
//		ͬ���첽��⣺ͬ����CPUͬʱִֻ��һ����ջ֡���첽��CPUͬʱִ�ж����ջ֡��
//-----------------------------------------------------------------------------------------------------------------------------------------
//PV������https://blog.csdn.net/bcbobo21cn/article/details/71154339
//
//
//-----------------------------------------------------------------------------------------------------------------------------------------
#include "stdafx.h"


//�����߳�ͬ����������
#include <stdio.h>
#include <process.h>
#include <windows.h>


long g_nNum; //ȫ����Դ,�������ڶ�ȡ���߳���
unsigned int __stdcall WriteFun(void *pPM);
unsigned int __stdcall ReadFun(void *pPM);
const int THREAD_NUM = 10; //���̸߳���

HANDLE g_event;//�����д����
CRITICAL_SECTION g_sc;//������߼����Ļ���

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

	//��֤���߳���ȫ�����н���
	WaitForSingleObject(handle_read, INFINITY);
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
	getchar();
	return 0;
}
//��д������˷�����ʵ��
//���ģ�����д���⣬ͨ��event���
//�ؼ���ʵ�ֶԶ��߼����Ļ���
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