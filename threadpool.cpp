#include "threadpool.h"
#include <thread>
#include <iostream>

const int TASK_MAX_THRESHOLD = 1024;

/*
* Funct : 线程池构造函数
* Param ：无
* retVal：无
* 设置初始线程数量，任务数量，任务数量上线，线程池模式
*/
ThreadPool::ThreadPool()
	: initThreadSize(0)
	, taskSize(0)
	, taskQueMaxThreshold(TASK_MAX_THRESHOLD)
	, poolMode(PoolMode::MODE_FIXED)
{}

/*
* Funct : 线程池析构函数
* Param ：无
* retVal：无
* 没有申请空间，不需要释放
*/
ThreadPool::~ThreadPool()
{

}

/*
* Funct : 设置线程池工作模式
* Param ：需要设置的工作模式
* retVal：无
* 说  明：
*/
void ThreadPool::setMode(PoolMode mode)
{
	poolMode = mode;
}

/*
* Funct : 设置任务队列中任务数量上限
* Param ：新的任务数量上限
* retVal：无
*/
void ThreadPool::setTaskQueMaxThreshod(int threshold)
{
	taskQueMaxThreshold = threshold;
}

/*
* Funct : 向线程池中提交任务
* Param ：新的任务
* retVal：无
* 用户调用该接口，向任务队列中添加任务对象，生产任务
*/
void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{

}

/*
* Funct : 启动线程池
* Param ：线程池中初始线程个数
* retVal：无
*/
void ThreadPool::startThreadPool(int initSize)
{
	initThreadSize = initSize;  // 初始化线程数量
	
	// 创建线程对象
	for (int i = 0; i < initThreadSize; i++)
	{
		// 创建线程对象的时候将线程函数传递给Thread对象
		// std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
		threads.emplace_back(std::move(ptr)); // unique_ptr不允许拷贝构造
	}

	// 启动线程对象
	for (int i = 0; i < initThreadSize; i++)
	{
		threads[i]->startThread();  // 需要执行一个线程函数
	}
}

/*
* Funct : 线程函数
* Param ：
* retVal：无
* 线程池的所有线程从任务队列中取出任务并执行，消费任务
*/
void ThreadPool::threadFunc()
{
	std::cout << "begin threadFunc tid:" 
				<< std::this_thread::get_id()
				<< std::endl;

	std::cout << "end threadFunc tid:" 
				<< std::this_thread::get_id()
				<< std::endl;
}


/*-------------------------------- 线程方法实现 --------------------------------*/


Thread::Thread(ThreadFunc func)
	:threadFunc(func)
{}

Thread::~Thread()
{
}

void Thread::startThread()
{
	// 创建并启动一个线程来执行线程函数
	std::thread t(threadFunc);
	/*
	* 设置分离线程，出了这个函数，对象的生命周期就到了，但是线程函数还要执行
	*/
	t.detach();   // 分离线程

}
