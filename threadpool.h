#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>

// 任务抽象基类
class Task
{
public:
	//用户可以自定义任意任务类型，从Task继承，重写run方法，实现自定义任务处理
	virtual void run() = 0;
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

