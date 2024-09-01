



# Thread Pool Implementation Based on C++11



### 6.2.1. 线程池类的设计：

1. **线程列表**

```cpp
// 使用unique_ptr代替裸指针，vector析构的时候析构里面的Thread对象
std::vector<std::unique_ptr<Thread>> threads;
```

使用智能指针管理 Thread 对象，`vector`析构的时候，智能指针将它管理的 new 出来的堆内存释放掉，析构里面的Thread对象



1. **任务队列相关的成员** 

任务用队列来存储，需要考虑一个问题：**生命周期**

首先，我们肯定不能存储任务的副本`std::queue<Task>`，任务队列里每个元素都是基类，用值类型无法完成多态，所以必须用指针 `std::queue<Task *>`。而用指针就需要考虑一个问题：用户没有考虑对象的声明周期，可能创建了一个临时任务对象存储进任务队列，离开作用域后该对象被析构了线程池里的基类指针拿了一个已经析构的对象就没有意义了。用户关注的是任务执行哪些操作，我们需要考虑用户把对象交给线程池，线程池需要保证任务对象存在，这样才能通过基类指针访问这个任务并调用重写的 `run `方法。所以在任务队列中不应该用裸指针。线程池有义务帮助用户把传进来的对象的生命周期保持住，当任务的 `run` 方法执行完毕后再释放这个任务。所以我们需要使用智能指针，保持并拉长任务对象的生命周期，并且自动释放资源。

```cpp
std::queue<std::shared_ptr<Task>> taskQue;  // 任务队列
```

任务队列中任务的个数：需要考虑线程安全，但又没必要使用锁，因此使用原子类型

```cpp
#include <atomic>
std::atomic_int taskSize;					// 任务的数量
```

1. **锁和条件变量：**

任务队列的锁：

```cpp
std::mutex taskQueMtx;						// 保证任务队列的线程安全
```

两个条件变量：不空，不满。分别给用户线程和线程池中的线程用

如果任务队列不满：用户就可以提交任务到队列中

如果任务队列不空：线程池中的线程就可以从队列中取出任务并执行

```cpp
std::condition_variable notFull;			// 表示任务队列不满
std::condition_variable notEmpty;			// 表示任务队列不空
```



我们不希望对线程池对象进行拷贝构造和赋值

```cpp
ThreadPool(const ThreadPool&) = delete;
ThreadPool& operator=(const ThreadPool&) = delete;
```



**线程函数：**

每个线程启动后要执行一个线程函数。执行什么函数也是由线程池来决定的。

每个线程的线程函数执行的任务其实都是一样的：从任务队列中取出任务（如果有）并执行。线程函数访问队列所需要使用的条件变量和互斥锁都定义在了线程池类中， 并且是私有成员，线程类无法访问，所以线程函数定义在线程类中不太合适。

并且，我们在创建线程对象的时候就应该把线程函数给到线程对象



**提交任务函数（****难点：提交任务函数的返回值****）：**

- 信号量的 `wait`， `wait for` `wait until`

- - wait：传入一个参数是锁，或者两个参数：锁和条件，等到条件满足再启动，否则一直等
  - wait_for：增加了时间参数，设置了等待时间的长短，在规定时间内等条件满足
  - wait_until：增加了时间参数，设置了等待时间的终止时间。

- 设计`runTask()`函数的返回值：

怎么设计`runTask()`函数返回值可以表示任意类型？如何设计返回机制？

- -  部分场景下我们希望能够获取线程执行任务的返回值，但是不同用户提交的任务不同，期待的返回值也不同，但这时不能使用模板类型，因为模板函数跟虚函数不能写在一起。所以我们需要实现一个**任意类型 Any****。**的

因为我们代码从上往下编译的时候，编译到 `vitrual T runTask()`的时候，看见 `runTask()`是虚函数，就要给当前这个类产生一个虚函数表，把这个虚函数的地址记录在虚函数表中。如果是模板类型，此时函数还没有实例化，根本没有地址。因此不能一起使用。





### 6.2.2. 线程类



```cpp
using ThreadFunc = std::function<void()>;
```

- `**std::function<void()>**`：`std::function` 是 C++ 标准库中的一个模板类，它是一个通用的可调用对象容器。`std::function<void()>` 表示一个不接受任何参数且返回类型为 `void` 的可调用对象的类型。
- `**using**` **关键字**：`using` 关键字在这里用于定义类型别名。它允许您为现有类型定义一个新的名字。在这个例子中，`ThreadFunc` 就是 `std::function<void()>` 的一个新的名字。



实例化线程类对象：

```cpp
threads.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this)));
```

`**new Thread(std::bind(&ThreadPool::threadFunc, this))**`：

- - `new Thread(...)` 创建一个新的 `Thread` 对象实例。
  - `std::bind(&ThreadPool::threadFunc, this)` 使用 `std::bind` 将当前线程池对象的 `threadFunc` 成员函数绑定到当前对象实例（`this` 指针）。这意味着创建的 `Thread` 对象将执行 `ThreadPool` 类的 `threadFunc` 方法，并且该方法将在当前 `ThreadPool` 实例的上下文中执行。

- `**std::bind**`：`std::bind` 用于创建一个新的可调用对象，这个对象在调用时会使用预先指定的参数。在这个例子中，`&ThreadPool::threadFunc` 是一个指向成员函数的指针，`this` 指针表示当前对象实例。`std::bind` 将 `this` 捆绑到 `threadFunc` 上，这样当线程执行时，`threadFunc` 方法能够在正确的对象上下文中被执行。