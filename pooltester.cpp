#include <iostream>
#include <chrono>
#include <thread>
#include "threadpool.h"


class MyTask : public Task
{
public:
	void runTask()
	{
		std::cout << "tid: " << std::this_thread::get_id() << " begin!\n";

		std::this_thread::sleep_for(std::chrono::seconds(2));

		std::cout << "tid: " << std::this_thread::get_id() << " end!\n";
	}
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
