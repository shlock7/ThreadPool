﻿#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>

// Any类型可以接收任意数据类型
class Any
{
public:
	Any() = default;
	~Any() = default;
	
	// 类成员是个unique_ptr类型，没有左值构造和引用，但是有右值拷贝构造和引用
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;				// 右值拷贝构造和引用  不写也行
	Any& operator=(Any&&) = default;

	// 这个构造函数可以让Any类型接收任何其他类型的数据
	template<typename T>
	Any(T data) : basePtr(std::make_unique<Derive<T>>(data)) {}

	// 将Any对象中存储的data成员提取出来
	template<typename T>
	T cast()
	{
		// 从base中找出它指向的Derive对象，从中取出data成员
		// 基类指针要转成派生类指针 RTTI
		Derive<T>* pd = dynamic_cast<Derive<T>*>(base.get());
		if (pd == nullptr)
		{
			throw "type mismatch!";

		}
		return pd->data;
	}

private:
	// 基类类型
	class Base
	{
	public:
		/*
		* 在继承结构中，一个基类对应的派生类对象如果是在堆上创建的，
		* delete基类指针时，派生类的析构函数就无法执行了
		*/
		virtual ~Base() = default;
	};

	// 派生类
	template<typename T>
	class Derive :public Base
	{
	public:
		Derive(T anyData) : data(anyData) {}
		T data;				// 保存任意类型的数据
	};


private:
	// 定义一个基类的指针
	std::unique_ptr<Base> basePtr;
};


// 信号量类
class Semaphore
{
public:
	Semaphore(int limit = 0) : resLimit(limit) {}
	~Semaphore() = default;

	// 获取一个信号量资源
	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);
		
		// 等待信号量有资源(resLimit>0)，否则阻塞当前线程
		cond.wait(lock, [&]()->bool {return resLimit > 0; });
		resLimit--;
	}

	// 增加一个信号量资源
	void post()
	{
		std::unique_lock<std::mutex> lock(mtx);
		resLimit++;
		cond.notify_all();    // 增加之后一定要通知
	}

private:
	int resLimit;
	std::mutex mtx;
	std::condition_variable cond;
};

// 任务抽象基类
class Task
{
public:
	//用户可以自定义任意任务类型，从Task继承，重写run方法，实现自定义任务处理
	virtual Any runTask() = 0;
};


// 线程池支持的模式
enum class PoolMode
{
	MODE_FIXED,		// 线程数量固定
	MODE_CACHED		// 线程数量可动态增长
};


// 线程类型
class Thread
{
public:
	// 线程函数对象类型
	using ThreadFunc = std::function<void()>;

	Thread(ThreadFunc func);
	~Thread();

	
	void startThread();	// 启动线程
private:
	ThreadFunc threadFunc;
};


// 线程池类型
class ThreadPool
{
public:
	ThreadPool();					// 构造
	~ThreadPool();					// 析构

	void setMode(PoolMode mode);	// 设置线程池模式

	void startThreadPool(int initSize = 4);		// 开启线程池
		
	void setTaskQueMaxThreshod(int threshold);  // 设置任务队列上限阈值

	void submitTask(std::shared_ptr<Task> sp);	// 向线程池中提交任务

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	void threadFunc();   // 定义线程函数

private:
	PoolMode poolMode;							// 当前线程池的工作模式

	//std::vector< Thread*> threads;				// 线程列表
	// 使用unique_ptr代替裸指针，vector析构的时候析构里面的Thread对象
	std::vector<std::unique_ptr<Thread>> threads;
	size_t initThreadSize;						// 初始线程数量

	std::queue<std::shared_ptr<Task>> taskQue;  // 任务队列
	std::atomic_int taskSize;					// 任务的数量
	int taskQueMaxThreshold;					// 任务数量上限阈值

	std::mutex taskQueMtx;						// 保证任务队列的线程安全
	std::condition_variable notFull;			// 表示任务队列不满
	std::condition_variable notEmpty;			// 表示任务队列不空
};


#endif // THREADPOOL_H

