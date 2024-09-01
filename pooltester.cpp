#include <iostream>
#include <chrono>
#include <thread>
#include "threadpool.h"

/*
* 
*/
class MyTask : public Task
{
public:
	MyTask(int begin, int end)
		: begin_(begin)
		, end_(end)
	{}

	Any runTask()
	{
		std::cout << "tid: " << std::this_thread::get_id() << " begin!\n";

		int sum = 0;
		for (int i = begin_; i <= end_; i++)
		{
			sum += i;
		}

		std::cout << "tid: " << std::this_thread::get_id() << " end!\n";

		return sum;
	}

private:
	int begin_;
	int end_;
};


int main()
{
	ThreadPool pool;
	pool.startThreadPool(3);

	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());
	pool.submitTask(std::make_shared<MyTask>());

	getchar();

	return 0;
}
