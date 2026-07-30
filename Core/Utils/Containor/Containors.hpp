#ifndef CONTAINORS_H
#define CONTAINORS_H

#include <stack>
#include <queue>
#include <vector>
#include <memory>
#include <array>

template <typename T>
class StackPool
{
    std::vector<std::unique_ptr<T>> owner;
    std::stack<T*> pool;

public:
    StackPool()
    {}
    ~StackPool()
    {
        while (!pool.empty())
        {
            pool.pop();
        }
        owner.clear();
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
		if (ret == nullptr || pool.size() == owner.size()) return false;
		pool.push(ret);
		return true;
	}
	bool Pop(T*& get)
	{
        if (pool.empty()) return false;
		get = pool.top();
        pool.pop();
        return true;
	}

	size_t GetPoolSize() { return owner.size(); }
	bool isEmpty() { return pool.empty(); }
};

template <typename T>
class PoolUsingKey // 내부 원소 개수를 반드시 2^n (n != 0) 으로 맞추시오.
{
    uint32_t bitMask;
    std::vector<std::unique_ptr<T>> owner;   
    std::vector<T*> pool;   

public:
    PoolUsingKey(): bitMask(0)
    {}
    ~PoolUsingKey()
    {
        pool.clear();
        owner.clear();
    }

    // bool Reserve()
    // {
    //     if (owner.size() == poolSize) return false;
    //     std::unique_ptr<T>ptr = std::make_unique<T>();
    // }

    bool AddElement(std::unique_ptr<T>&& element)
    {
        if (element == nullptr) return false;
        T* ptr = element.get();
        owner.push_back(std::move(element));
        pool.push_back(ptr);
        size_t currentSize = pool.size();
        if (currentSize > 0 && (currentSize & (currentSize - 1)) == 0) bitMask = currentSize - 1; // 비트 마스크 전용 변수로 명확히 관리
        return true;
    }

    T* GetElement(uint32_t hashKey)
    {
        if (pool.empty()) return nullptr;
        uint32_t key = hashKey & bitMask;
        return pool[key]; 
    }

};

template <typename RegistryKey, typename T>
class ElementRegistry
{
    std::unordered_map<RegistryKey, std::unique_ptr<T>> objMap;    
    std::vector<T*> objs;

public:
    ElementRegistry(){}
    ~ElementRegistry()
    {
        objs.clear();
        objMap.clear();
    }

    bool AddElement(const RegistryKey& key, std::unique_ptr<T>&& obj)
    {
        T* objPTR = obj.get();
        auto [iter, success] = objMap.emplace(key, std::move(obj));
        if(!success) return success;
        objs.push_back(objPTR);
        return true;
    }

    T* Find(const RegistryKey& key)
    {
        return objMap.find(key).get();
    }

    bool Delete(const RegistryKey& key)
    {
        T* delThis = Find(key);
        if (delThis == nullptr) return false;
        for (auto it  = objs.begin(); it != objs.end(); it++)
        {
            T* obj = *it;
            if (delThis == obj)
            {
                objs.erase(it);
                break;
            }
        }
        objMap.erase(key);
        return true;
    }

    std::vector<T*>& GetObjects() {return objs;}
};
#endif