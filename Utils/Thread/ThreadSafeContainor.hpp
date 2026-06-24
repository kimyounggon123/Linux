#ifndef THREADSAFECONTAINOR_H
#define THREADSAFECONTAINOR_H

#include <stack>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <vector>
#include <memory>


#define INFINITE 0xffffffff

template <typename T>
class ThreadSafeContainor
{
	std::vector<T> safe_containor;
	std::mutex cont_mtx;
	std::condition_variable cont_cv;
	unsigned long timeout_ms;

public:
	ThreadSafeContainor(unsigned long timeout = INFINITE) : timeout_ms(timeout) {}
	~ThreadSafeContainor() = default;

	// 복사 금지
	ThreadSafeContainor(const ThreadSafeContainor&) = delete;
	ThreadSafeContainor& operator=(const ThreadSafeContainor&) = delete;

	bool Push(const T& input) 
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.push_back(input);
		cont_cv.notify_one();
		return true;
	}

	// move 스타일
	bool Push(T&& input)
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.push_back(std::move(input));
		cont_cv.notify_one();
		return true;
	}

	bool Pop(T& output)
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		output = std::move(safe_containor.back());
		safe_containor.pop_back();
		return true;
	}

	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		return safe_containor.size() == 0;
	}

	bool Swap(std::vector<T>& toSwap)
	{
		if (safe_containor.size() == 0) return false;
		std::lock_guard<std::mutex> lock(cont_mtx);
		std::swap(safe_containor, toSwap);
		return true;
	}

	void PushFrontRange(std::vector<T>&& remainData, size_t sendCounts)
	{
		if (remainData.size() <= sendCounts) return;

		// temp vector 생성
		std::vector<T> temp;
		temp.reserve((remainData.size() - sendCounts) + safe_containor.size());

		// temp에 remainData를 일단 insert
		temp.insert(
			temp.end(),
			std::make_move_iterator(remainData.begin() + sendCounts),
			std::make_move_iterator(remainData.end())
		);

		// 이제 현재 담겨있는 데이터를 담음
		std::lock_guard<std::mutex> lock(cont_mtx);
		temp.insert(
			temp.end(),
			std::make_move_iterator(safe_containor.begin()),
			std::make_move_iterator(safe_containor.end())
    	);
		std::swap(safe_containor, temp);
	}

	void PushBackVector(std::vector<T>&& tailDatalist)
	{
		if (tailDatalist.empty()) return;

		std::lock_guard<std::mutex> lock(cont_mtx);
		
		safe_containor.reserve(safe_containor.size() + tailDatalist.size());

		safe_containor.insert(
			safe_containor.end(), 
			std::make_move_iterator(tailDatalist.begin()), 
			std::make_move_iterator(tailDatalist.end())
		);
	
		tailDatalist.clear(); 
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.clear();	
	}
};




template <typename T>
class ThreadSafeStack
{
	std::stack<T> safe_stack;
	std::mutex stack_mtx;
	std::condition_variable stack_cv;
	unsigned long timeout_ms;

public:
	ThreadSafeStack(unsigned long timeout = INFINITE) : timeout_ms(timeout) {}
	~ThreadSafeStack() = default;

	// 복사 스타일
	bool push(const T& input) 
	{
		std::lock_guard<std::mutex> lock(stack_mtx);
		safe_stack.push(input);
		stack_cv.notify_one();
		return true;
	}
	

	// move 스타일
	bool push(T&& input)
	{
		std::lock_guard<std::mutex> lock(stack_mtx);
		safe_stack.push(std::move(input));
		stack_cv.notify_one();
		return true;
	}

	bool pop(T& output) {
		std::unique_lock<std::mutex> lock(stack_mtx);
		if (timeout_ms == INFINITE)
		{
			stack_cv.wait(lock, [this] { return !safe_stack.empty(); });
		}
		else {
			if (!stack_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !safe_stack.empty(); }))
			{
				return false; // 타임아웃
			}
		}

		output = std::move(safe_stack.top());
		safe_stack.pop();
		return true;
	}

	bool pop_nowait(T& output)
	{
		output = std::move(safe_stack.top());
		safe_stack.pop();
		return true;
	}


	bool isEmpty() {
		std::lock_guard<std::mutex> lock(stack_mtx);
		return safe_stack.empty();
	}

	size_t size() {
		std::lock_guard<std::mutex> lock(stack_mtx);
		return safe_stack.size();
	}

	void setTimems(unsigned long timems)
	{
		timeout_ms = timems;
	}

	void Clear()
	{
		while (!isEmpty())
		{
			T output;
			pop_nowait(output);
		}
	}

	void Swap(std::stack<T>& toSwap)
	{
		std::lock_guard<std::mutex> lock(stack_mtx);
		std::swap(safe_stack, toSwap);
	}
};


template<typename T>
class ThreadSafeQueue {
	std::queue<T> safe_queue;
	std::mutex queue_mtx;
	std::condition_variable queue_cv;
	unsigned long timeout_ms;

public:
	ThreadSafeQueue(unsigned long timeout_ms = INFINITE) : timeout_ms(timeout_ms) {}
	~ThreadSafeQueue() = default;

	// 복사 버전
	bool enqueue(const T& input) {
		std::lock_guard<std::mutex> lock(queue_mtx);
		safe_queue.push(input);
		queue_cv.notify_one();
		return true;
	}

	// move 버전
	bool enqueue(T&& input) {
		std::lock_guard<std::mutex> lock(queue_mtx);
		safe_queue.push(std::move(input));
		queue_cv.notify_one();
		return true;
	}

	bool dequeue(T& output) {
		std::unique_lock<std::mutex> lock(queue_mtx);

		if (timeout_ms == INFINITE)
			queue_cv.wait(lock, [this] { return !safe_queue.empty(); });

		else if (!queue_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
			[this] { return !safe_queue.empty(); }))
			return false;

		output = std::move(safe_queue.front());   // move
		safe_queue.pop();
		return true;
	}

	bool dequeue_nowait(T& output)
	{
		output = std::move(safe_queue.front());   // move
		safe_queue.pop();
		return true;
	}

	bool isEmpty()
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		return safe_queue.empty();
	}
	size_t size()
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		return safe_queue.size();
	}

	void setTimems(unsigned long timems)
	{
		timeout_ms = timems;
	}

	void Clear()
	{
		while (!isEmpty())
		{
			T output;
			pop_nowait(output);
		}
	}

	void Swap(std::queue<T>& toSwap)
	{
		std::lock_guard<std::mutex> lock(queue_mtx);
		std::swap(safe_queue, toSwap);
	}
};


template <typename T>
using LockStack = ThreadSafeStack<T>;
template <typename T>
using LockQueue = ThreadSafeQueue<T>;

template <typename T>
class ThreadSafePool
{
	std::vector<std::unique_ptr<T>> owner;
	ThreadSafeStack<T*> pool;

public:
	ThreadSafePool(unsigned int timeout_ms = INFINITE) : pool(timeout_ms)
	{}
	~ThreadSafePool()
	{
		Clear();
	}

	// initialize
	bool AddElement(std::unique_ptr<T>&& element)
	{
		if (!element) return false;
		pool.push(element.get());
		owner.push_back(std::move(element));
		return true;
	}

	bool Push(T* ret)
	{
		if (!ret) return false;
		pool.push(ret);
		return true;
	}
	bool Pop(T*& get)
	{
		return pool.pop(get);
	}

	size_t GetPoolSize() { return owner.size(); }

	bool isEmpty() { return pool.isEmpty(); }

	void Clear()
	{
		pool.Clear();
		owner.clear();
	}

};
template <typename T>
using LockPool = ThreadSafePool<T>;

/*
#include <atomic>
template <typename T>
class RingQueueElement
{
	std::atomic<bool> isUsing;
	T element;
public:
	RingQueueElement(): isUsing(false) {}
	~RingQueueElement() = default;

	T* UseThis() 
	{
		bool expected = false;
        // 다른 스레드가 먼저 채가지 않았을 때만(false일 때만) true로 변경 (CAS 연산)
        if (!isUsing.compare_exchange_strong(expected, true)) 
		{
            return nullptr; 
        }
		return &element;
	}
	void ReturnThis() { isUsing.store(false, std::memory_order_release); }

}

template <typename T>
class RingQueue
{
	std::atomic<unsigned int> head_pos;
    std::atomic<unsigned int> tail_pos;

	unsigned int size;
	unsigned int mask;
	RingQueueElement<T>* queue;


public:
	RingQueue(unsigned int size = 1024): head_pos(0), tail_pos(0), 
		size(size), mask(size - 1), 
		queue(nullptr)
	{}
	~RingQueue()
	{
		Destroy();
	}

	bool Initialize()
	{
		if (queue != nullptr) return false;
		queue = new RingQueueElement<T>[size]
		return true;
	}
	void Destroy()
	{
		if (queue == nullptr) return;
		delete[] queue;
		queue = nullptr;
	}

	T* Get()
	{

	}
};
*/
#endif