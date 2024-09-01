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
	// 获取锁
	std::unique_lock<std::mutex> lock(taskQueMtx);

	// 线程的通信：等待任务队列有空位置
	
	//while (taskQue.size() == taskQueMaxThreshold)
	//{
	//	notFull.wait(lock);   // 进入等待状态， 释放lock
	//}

	// 这种写法等同于上面的for循环
	// notFull.wait(lock, [&]()->bool {return taskQue.size() < taskQueMaxThreshold; });
	
	// 用户提交任务，最长不能阻塞超过1s，否则判断提交任务失败并返回
	bool waitResult = false;
	waitResult = notFull.wait_for(lock, 
						std::chrono::seconds(1),
						[&]()->bool {return taskQue.size() < taskQueMaxThreshold; });
	if (!waitResult)
	{
		// notFull等待1s，条件依然没有满足，即提交任务失败
		std::cerr << "Task queue is full, submit task failed!\n";
		return;
	}

	// 任务队列中有空位，就把任务放入任务队列
	taskQue.emplace(sp);
	taskSize++;

	// 放了新任务，此时任务队列肯定不为空，not_Empty通知
	notEmpty.notify_all();

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

	while (true)
	{
		// 获取锁
		std::unique_lock<std::mutex> lock(taskQueMtx);

		std::cout << "tid: " << std::this_thread::get_id() << " Try to get task...\n";

		// 等待notEmpty条件，即条件队列非空才能执行任务，此时没有任务可以一直等，不用返回
		notEmpty.wait(lock, [&]()->bool {return taskQue.size() > 0; });
		
		std::cout << "tid: " << std::this_thread::get_id() << " Get task success!\n";

		// 从任务队列中取一个任务
		auto task = taskQue.front();
		taskQue.pop();
		taskSize--;

		// 如果任务队列中还有其他任务，需要通知其他线程来执行任务
		if (taskQue.size() > 0)
		{
			notEmpty.notify_all(); // 通知其他线程任务队列非空
		}
		
		// 取出任务后，任务队列肯定不满，需要通知，可以继续提交任务
		notFull.notify_all();
		
		// 获取完任务就应该释放锁
		lock.unlock();
		
		// 当前线程执行这个任务
		if (task != nullptr)
		{
			task->runTask();
		}
	}

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
