#ifndef THREADSAFECONTAINOR_H
#define THREADSAFECONTAINOR_H

#include <stack>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <vector>
#include <memory>

#include "../Containor/Containors.hpp"
#define INFINITE 0xffffffff

template <typename T>
class ThreadSafeContainor
{
	std::vector<T> safe_containor;
	std::mutex cont_mtx;
	std::condition_variable cont_cv;
	unsigned long timeout_ms;

public:
	ThreadSafeContainor(uint32_t maxSize = 1000, unsigned long timeout = INFINITE) : timeout_ms(timeout)
	{
		safe_containor.reserve(maxSize);	
	}
	~ThreadSafeContainor() 
	{
		safe_containor.clear();
	}

	// 복사 금지
	ThreadSafeContainor(const ThreadSafeContainor&) = delete;
	ThreadSafeContainor& operator=(const ThreadSafeContainor&) = delete;

	void Push(const T& input) 
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.push_back(input);
		cont_cv.notify_one();
	}

	// move 스타일
	void Push(T&& input)
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.push_back(std::move(input));
		cont_cv.notify_one();
	}

	void Pop(T& output)
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		output = std::move(safe_containor.back());
		safe_containor.pop_back();
	}

	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		return safe_containor.size() == 0;
	}

	void Swap(std::vector<T>& toSwap)
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		std::swap(safe_containor, toSwap);
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

	// param으로 받은 vector 내 데이터를 maxChunkSize만큼 여기 vector에 넣음.
	void PushChunk(std::vector<T>& dataList, size_t maxChunkSize)
	{
		if (dataList.empty() || maxChunkSize == 0) return;

		std::lock_guard<std::mutex> lock(cont_mtx);

		const size_t chunk = std::min(maxChunkSize, dataList.size());
		auto begin = dataList.end() - chunk;

		safe_containor.insert(
			safe_containor.end(),
			std::make_move_iterator(begin),
			std::make_move_iterator(dataList.end())
		);

		dataList.resize(dataList.size() - chunk);
	}
	
	
	// param으로 받은 vector에 maxChunkSize 만큼 데이터 넣음
	bool PopChunk(std::vector<T>& dataList, size_t maxChunkSize)
	{
		if (maxChunkSize == 0) return false;

		std::lock_guard<std::mutex> lock(cont_mtx);
		if (safe_containor.empty()) return false;
		const size_t chunk = std::min(maxChunkSize, safe_containor.size());
		dataList.reserve(dataList.size() + chunk);

		auto begin = safe_containor.end() - chunk;

		dataList.insert(
			dataList.end(),
			std::make_move_iterator(begin),
			std::make_move_iterator(safe_containor.end())
		);

		safe_containor.resize(safe_containor.size() - chunk);

		return true;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(cont_mtx);
		safe_containor.clear();	
	}

	const size_t GetCapacity() const {return safe_containor.capacity();}
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
	void push(const T& input) 
	{
		{
			std::lock_guard<std::mutex> lock(stack_mtx);
			safe_stack.push(input);
		}
		stack_cv.notify_one();
	}
	

	// move 스타일
	void push(T&& input)
	{
		{
			std::lock_guard<std::mutex> lock(stack_mtx);
			safe_stack.push(input);
		}
		stack_cv.notify_one();
	}

	bool pop(T& output) {
		std::unique_lock<std::mutex> lock(stack_mtx);
		if (timeout_ms == INFINITE)
		{
			stack_cv.wait(lock, [this] { return !safe_stack.empty(); });
		}
		else 
		{
			if (!stack_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !safe_stack.empty(); }))
			{
				return false; // 타임아웃
			}
		}

		output = std::move(safe_stack.top());
		safe_stack.pop();
		return true;
	}

	bool pop_chunk(std::vector<T>& output_chunk, size_t max_count = 90)
	{
		std::unique_lock<std::mutex> lock(stack_mtx);

		if (timeout_ms == INFINITE) 
		{
			stack_cv.wait(lock, [this] { return !safe_stack.empty(); });
		}
		else 
		{
			if (!stack_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
				[this] { return !safe_stack.empty(); })) {
				return false; // 타임아웃 발생 시 락 해제되며 종료
			}
		}

		// 빨리 복사
		output_chunk.reserve(max_count);
		while (!safe_stack.empty() && output_chunk.size() < max_count) 
		{
			output_chunk.push_back(std::move(safe_stack.top()));
			safe_stack.pop();
		}
		return true;
	}

	void pop_nowait(T& output)
	{
		output = std::move(safe_stack.top());
		safe_stack.pop();
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
	void enqueue(const T& input) 
	{
		{
			std::lock_guard<std::mutex> lock(queue_mtx);
			safe_queue.push(std::move(input));
		}
		queue_cv.notify_one();
	}

	// move 버전
	void enqueue(T&& input) 
	{
		{
			std::lock_guard<std::mutex> lock(queue_mtx);
			safe_queue.push(std::move(input));
		}

		queue_cv.notify_one();
	}

	bool dequeue(T& output) 
	{
		std::unique_lock<std::mutex> lock(queue_mtx);

		if (timeout_ms == INFINITE)
			queue_cv.wait(lock, [this] { return !safe_queue.empty(); });

		else 
		{
			if (!queue_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
				[this] { return !safe_queue.empty(); })) 
			{
				return false; // 타임아웃 발생 시 락 해제되며 종료
			}
		}

		output = std::move(safe_queue.front());   // move
		safe_queue.pop();
		return true;
	}

	bool dequeue_chunk(std::vector<T>& output_chunk, size_t max_count = 90)
	{
		std::unique_lock<std::mutex> lock(queue_mtx);

		if (timeout_ms == INFINITE) 
		{
			queue_cv.wait(lock, [this] { return !safe_queue.empty(); });
		}
		else 
		{
			if (!queue_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
				[this] { return !safe_queue.empty(); })) {
				return false; // 타임아웃 발생 시 락 해제되며 종료
			}
		}

		// 빨리 복사
		output_chunk.reserve(max_count);
		while (!safe_queue.empty() && output_chunk.size() < max_count) 
		{
			output_chunk.push_back(std::move(safe_queue.front()));
			safe_queue.pop();
		}
		return true;
	}

	void dequeue_nowait(T& output)
	{
		output = std::move(safe_queue.front());   // move
		safe_queue.pop();
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
		T* ptr = element.get();
		pool.push(ptr);
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

template <typename T>
class ThreadSafePoolChunkModel
{
	std::vector<std::unique_ptr<T>> owner;
	ThreadSafeContainor<T*> pool;

public:
	ThreadSafePoolChunkModel(uint32_t maxSize, unsigned int timeout_ms = INFINITE) : pool(maxSize, timeout_ms)
	{
		owner.reserve(maxSize);
	}
	~ThreadSafePoolChunkModel()
	{
		pool.Clear();
		owner.clear();
	}

	bool AddElement(std::unique_ptr<T>&& element)
	{
		if (!element || owner.size() >= owner.capacity()) return false;
		T* ptr = element.get();
		pool.Push(ptr);
		owner.push_back(std::move(element));
		return true;
	}

	void PushChunk(std::vector<T*>& tailDatalist,  size_t maxChunkSize)
	{
		pool.PushChunk(tailDatalist, maxChunkSize);
	}

	bool PopChunk(std::vector<T*>& dataList, uint32_t maxChunkSize)
	{
		return pool.PopChunk(dataList, maxChunkSize);
	}	

	const size_t GetCapacity() const {return owner.capacity();}
};

#include <atomic>
template<typename Key, typename T>
class ThreadSafeMap
{
	std::mutex mtx;
	std::unordered_map<Key, T> map;

public:
	ThreadSafeMap(){}
	~ThreadSafeMap()
	{
		map.clear();
	}

	bool Add(const Key& key, const T& obj)
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto [iter, success] = map.emplace(key, obj);
		return success;
	}

	bool Add(const Key& key, T&& obj)
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto [iter, success] = map.emplace(key, std::move(obj));
		return success;
	}

	bool Find(const Key& key, T& result)
	{
		auto it = map.find(key);
		if (it == map.cend()) return false;
		result = it->second;
        return true;
	}

	bool Delete(const Key& key)
	{
		return map.erase(key) != 0;
	}
};

template <typename RegistryKey, typename T>
class ThreadElementRegistry
{	
	std::mutex mtx;
	ElementRegistry<RegistryKey, T> registry;

public:
	bool AddElement(const RegistryKey& key, std::unique_ptr<T>&& obj)
    {
        std::lock_guard<std::mutex> lock(mtx);
		return registry.AddElement(key, std::move(obj));
    }

    T* Find(const RegistryKey& key)
    {
		std::lock_guard<std::mutex> lock(mtx);
        return registry.Find(key);
    }

    bool Delete(const RegistryKey& key)
    {
        std::lock_guard<std::mutex> lock(mtx);
        return registry.Delete(key);
    }

    std::vector<T*>& GetObjects() 
	{
		std::lock_guard<std::mutex> lock(mtx);
		return registry.GetObjects();
	}
};




/*
#include <atomic>
template <typename T>
class LockFreeQueue
{
	struct Element
	{
		T value;
		std::atomic<Element*> next; 

        Element() : next(nullptr) {}
        Element(T&& val) : value(std::move(val)), next(nullptr) {}
	};

	std::atomic<Element*> head; // Dummy 노드를 가리킴
    std::atomic<Element*> tail;

	void Destroy()
	{

	}
public:
	LockFreeQueue(): isReferenced(false)
	{
		Element* dummy = new Element();
		head.store(dummy);
        tail.store(dummy);
	}
	~LockFreeQueue()
	{

	}


	void Enqueue(T&& value)
	{
		Element* newElement = new Element(std::move(value));
		while (true)
		{
			Element* currTail = tail.load(std::memory_order_acquire);
        	Element* nextNode = currTail->next.load(std::memory_order_acquire);

			// case 1. 확인한 tail == 현재 tail일 경우
			if (currTail == tail.load(std::memory_order_relaxed))
        	{
				// 1. case 1-1. 정상적인 상태로 tail이 실제로 맨 마지막 노드일 경우
				if (nextNode == nullptr)
				{
					// currtail->next를 nullptr에서 newElement로 바꾸기 시도
					// element.compare_exchange_strong(expected, desired, std::memory_order success, std::memory_order failure)
					// element가 expected과 값이 같다면, desired로 변경 후 return true
					// element가 expected과 값이 다르면, element를 변경하지 않고 대신 expected를 desired로 강제 변경.
					Element* expectedNull = nullptr;
					if (currTail->next.compare_exchange_strong(expectedNull, newElement, 
                    	std::memory_order_release, std::memory_order_relaxed))
					{
						// 현재 tail을 newElement로 바꿔치기
						tail.compare_exchange_strong(currTail, newElement, 
                        	std::memory_order_release, std::memory_order_relaxed);
						// 3, 4번 째 인자인 std::memory_order의 종류
						// memory_order_relaxed: 순서 재정렬을 하지 않고 오직 원자성만 보장. 가장 가벼움. 보통 실패 처리나 카운트 증가에 쓰임
						// memory_order_release: lock-free 에서 안전하게 값을 가져오고 데이터를 밖으로 밀어내는 역할. 데이터를 안전하게 노출시킬 때 사용
						// memory_order_acquire: 최신 데이터를 안전하게 내 스레드로 가져오는 역할. 변수 값 load나 전송 권환 획득 시 사용
						break;
					}
				}
			}

			// case 2. 확인한 tail != 현재 tail 일 경우
			else 
			{
				// 현재 tail을 진짜 tail인 nextNode로 이동
				tail.compare_exchange_strong(currTail, nextNode, 
                    std::memory_order_release, std::memory_order_relaxed);
			}
		}
	}

	bool Dequeue(T& value)
	{

	}

};
*/
#endif