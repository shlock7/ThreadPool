#include <iostream>
#include <chrono>
#include <thread>
#include "threadpool.h"


int main()
{
	ThreadPool pool;
	pool.startThreadPool();

	std::this_thread::sleep_for(std::chrono::seconds(5));

	return 0;
}
