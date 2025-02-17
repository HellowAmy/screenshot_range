
#ifndef TCONT_H
#define TCONT_H

#include <queue>
#include <stack>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>

namespace bhtools {


// 自旋锁
struct Fspinlock
{   
    // 上锁-循环获取标记直到成功
    void lock() { while(_flg.test_and_set(std::memory_order_acquire)) {} }

    // 解锁-清空标记
    void unlock() { _flg.clear(std::memory_order_release); }

    std::atomic_flag _flg = ATOMIC_FLAG_INIT;      // 原子互斥标记
};

//
//
//
//

// 析构退出-自动退出上锁状态
template<typename Tlock>
struct Texit_lock
{
    // 构造时上锁
    Texit_lock(Tlock *lock) { _lock = lock; if(_lock){ _lock->lock(); } }

    // 析构时解锁
    ~Texit_lock() { if(_lock){ _lock->unlock(); } }

    Tlock *_lock;    // 上锁指针-避免不能赋值的情况
};

//
//
//
//

// 队列-线程安全
template<typename T,typename Tlock = Fspinlock>
class Tqueue 
{
public:
    size_t size()               { Texit_lock<Tlock> e(&_lock);  return _org.size(); }
    size_t empty()              { Texit_lock<Tlock> e(&_lock);  return _org.empty(); }
    void pop()                  { Texit_lock<Tlock> e(&_lock);  _org.pop(); }
    void push(const T &val)     { Texit_lock<Tlock> e(&_lock);  _org.push(val); }
    T front()                   { Texit_lock<Tlock> e(&_lock);  return _org.front(); }
    T back()                    { Texit_lock<Tlock> e(&_lock);  return _org.back(); }
    
protected:
    Tlock _lock;            // 上锁对象
    std::queue<T> _org;     // 原始数据
};

//
//
//
//

// 堆栈-线程安全
template<typename T,typename Tlock = Fspinlock>
class Tstack
{
public:
    size_t size()               { Texit_lock<Tlock> e(&_lock);  return _org.size(); }
    size_t empty()              { Texit_lock<Tlock> e(&_lock);  return _org.empty(); }
    void pop()                  { Texit_lock<Tlock> e(&_lock);  _org.pop(); }
    void push(const T &val)     { Texit_lock<Tlock> e(&_lock);  _org.push(val); }
    T top()                     { Texit_lock<Tlock> e(&_lock);  return _org.top(); }
    
protected:
    Tlock _lock;            // 上锁对象
    std::stack<T> _org;     // 原始数据
};

//
//
//
//

// 数组-线程安全
template<typename T,typename Tlock = Fspinlock>
class Tvector
{
public:
    typedef typename std::vector<T>::iterator iterator;

public:
    void clear()                    { Texit_lock<Tlock> e(&_lock);  _org.clear(); }
    size_t size()                   { Texit_lock<Tlock> e(&_lock);  return _org.size(); }
    size_t empty()                  { Texit_lock<Tlock> e(&_lock);  return _org.empty(); }
    iterator begin()                { Texit_lock<Tlock> e(&_lock);  return _org.begin(); }
    iterator end()                  { Texit_lock<Tlock> e(&_lock);  return _org.end(); }
    void pop_back()                 { Texit_lock<Tlock> e(&_lock);  _org.pop_back(); }
    void push_back(const T &val)    { Texit_lock<Tlock> e(&_lock);  _org.push_back(val); };

    iterator insert(iterator pos,const T &val)   
    { Texit_lock<Tlock> e(&_lock); return _org.insert(pos,val); }

    iterator erase(iterator pos)   
    { Texit_lock<Tlock> e(&_lock); return _org.erase(pos); }

    T& operator[](size_t index)
    { Texit_lock<Tlock> e(&_lock); return _org[index]; }

protected:
    Tlock _lock;            // 上锁对象
    std::vector<T> _org;    // 原始数据
};

//
//
//
//

// 索引-线程安全
template<typename Tkey,typename Tval,typename Tlock = Fspinlock>
class Tmap
{   
public:
    typedef typename std::map<Tkey,Tval>::iterator iterator;

public:
    void clear()                { Texit_lock<Tlock> e(&_lock);  _org.clear(); }
    size_t size()               { Texit_lock<Tlock> e(&_lock);  return _org.size(); }
    size_t empty()              { Texit_lock<Tlock> e(&_lock);  return _org.empty(); }
    iterator begin()            { Texit_lock<Tlock> e(&_lock);  return _org.begin(); }
    iterator end()              { Texit_lock<Tlock> e(&_lock);  return _org.end(); }
    
    std::pair<iterator, bool> insert(std::pair<Tkey,Tval> val)  
    { Texit_lock<Tlock> e(&_lock); return _org.insert(val); }

    iterator erase(iterator it)  
    { Texit_lock<Tlock> e(&_lock); return _org.erase(it); }

    iterator find(const Tkey &val)  
    { Texit_lock<Tlock> e(&_lock); return _org.find(val); }

    template <class... Args>  
    std::pair<iterator,bool> emplace (Args&&... args)
    { Texit_lock<Tlock> e(&_lock); return _org.emplace(args...); }

    Tval& operator[](const Tkey &key)
    { Texit_lock<Tlock> e(&_lock); return _org[key]; }
    
protected:
    Tlock _lock;                // 上锁对象
    std::map<Tkey,Tval> _org;   // 原始数据
};



}; // bhtools



#endif // TCONT_H
