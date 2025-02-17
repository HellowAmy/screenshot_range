
#ifndef TTIMER_H
#define TTIMER_H

#include <chrono>

#include "Tpool.h"
#include "Theap.h"

namespace bhtools {


// 多线程定时器-在子线程开启事件循环监控时间-其他子线程处理任务-至少存在2个子线程才能导致定时延时
// 可自定义定时器的精度-事件循环间隔-任务线程池数量
template<typename Tduration = std::chrono::milliseconds,size_t Tinterval = 20,size_t Tths = 2>
struct Ttimer
{
    typedef std::chrono::nanoseconds nanoseconds;
    typedef std::chrono::steady_clock steady_clock;

    // 定时任务包体
    struct task
    {
        task() { }
        task(size_t id) : _id(id) { }

        bool _never;                        // 永不停止
        size_t _id;                         // 定时器任务ID
        size_t _active;                     // 活动数规定执行次数
        Tduration _start;                   // 任务开始时间
        Tduration _delay;                   // 任务延时时间-加入多久后会被执行
        std::function<void(size_t)> _task;  // 回调函数

        bool operator>(const task &ct) { return _start > ct._start; }   // 最大堆需实现
        bool operator<(const task &ct) { return _start < ct._start; }   // 最小堆需实现
        bool operator==(const task &ct) { return _id == ct._id; }       // 需要移除指定节点时需实现
    };

    // 构造时启动事件循环
    Ttimer() { _works.push(&Ttimer::start_event_loop,this); }

    // 析构时退出事件循环
    ~Ttimer() { close_timer(); }

    // 开始定时器时间循环
    inline void start_event_loop()
    {
        while(_run)
        {
            Tduration now = time_now(); 
            process_timer_task(now);            
            std::this_thread::sleep_for(Tduration(Tinterval));
        }
        clear_task_th();
    }

    // 在事件循环中处理所有超时任务-线程池处理任务
    inline void process_timer_task(Tduration now)
    {
        while(_heap.size_node() > 0)
        {
            task rct = _heap.check_root();
            if(rct._start <= now)
            {
                rct = pop_task_th();
                if(rct._task) { _works.push(rct._task,rct._id); }

                if(rct._never == false) { rct._active--; }
                if(rct._never || rct._active > 0)
                {
                    rct._start = now + rct._delay;
                    insert_task_th(rct);
                }
            }
            else { break; }
        }
    }

    // 加入定时任务到执行队列
    inline size_t push(Tduration delay,std::function<void(size_t)> fn,size_t active = 1)
    {
        task ct;
        ct._id = _count++;
        ct._start = time_now() + delay;
        ct._delay = Tduration(delay);
        ct._active = active;
        ct._task = fn;
        if(active == 0) { ct._never = true; }
        else { ct._never = false; }

        insert_task_th(ct);
        return ct._id;
    }

    // 加入定时任务到执行队列-重载
    inline size_t push(size_t delay,std::function<void(size_t)> fn,size_t active = 1)
    { return push(std::chrono::duration_cast<Tduration>(Tduration(delay)),fn,active); }

    // 移除指定ID定时任务
    inline bool remove(size_t id) { return remove_task_th(id); }

    // 关闭定时器
    inline void close_timer() { _run = false; }

    // 重启定时器
    inline void restart_timer()
    { 
        close_timer();
        std::this_thread::sleep_for(Tduration(Tinterval * 2));
        if(_run == false) 
        { 
            _run = true;
            _works.push(&Ttimer::start_event_loop,this);
        } 
    }



    // 加入任务到堆树排队
    inline void insert_task_th(const task &ct)
    {
        std::unique_lock<std::mutex> lock(_mut);
        _heap.insert_node(ct);
    }

    // 移除指定ID定时任务
    inline bool remove_task_th(size_t id)
    {
        std::unique_lock<std::mutex> lock(_mut);
        return _heap.remove_node(task(id));
    }

    // 清空定时器任务
    inline void clear_task_th()
    {
        std::unique_lock<std::mutex> lock(_mut);
        _heap.clear_heap();
    }

    // 弹出超时的任务
    inline task pop_task_th()
    {
        std::unique_lock<std::mutex> lock(_mut);
        return _heap.pop_root();
    }

    // 获取当前时间
    static inline Tduration time_now()
    {
        nanoseconds now = steady_clock::now().time_since_epoch();
        return std::chrono::duration_cast<Tduration>(now);
    }


    bool _run = true;                       // 事件循环运行标记
    size_t _count = 0;                      // 任务数量累计-并作为新的任务ID发布
    std::mutex _mut;                        // 定时器任务锁
    Tpool<Tths> _works;                     // 定时任务线程池
    Theap<task,Theap_comp_min> _heap;       // 最小堆定时排队
};


} // bhtools


#endif // TTIMER_H
