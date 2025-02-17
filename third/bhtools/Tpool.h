
#ifndef TPOOL_H
#define TPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>


namespace bhtools {


// 线程池-可获取任务函数的返回值
template<size_t Tnum = 4>
struct Tpool
{
    // 加入并运行线程任务
    Tpool() 
    {
        for(size_t i=0;i<Tnum;i++) 
        { 
            _workers.emplace_back([=](){
                while(true)
                {
                    std::function<void()> task = nullptr;
                    {
                        std::unique_lock<std::mutex> lock(_mut);                // 独占锁--获取队列任务
                        while (_tasks.empty() && _run) { _cond.wait(lock); }    // 判断假唤醒--退出且队列为空
                        if(_run == false && _tasks.empty()) { return; }         // 等待队列任务完成并退出任务
                        task = std::move(_tasks.front()); _tasks.pop();         // 取任务并弹出队列
                    }
                    if(task) { task(); }
                }
            }); 
        }
    }

    //释放线程池
    ~Tpool() 
    {
        { std::unique_lock<std::mutex> lock(_mut); _run = false; }      // 关闭运行标记
        _cond.notify_all();                                             // 唤醒所有线程准备退出
        for(std::thread &worker: _workers) { worker.join(); }           // 等待所有线程完成任务后释放
    }

    // 加入任务
    template<typename Tfunc, typename... Targs>
    auto push(Tfunc&& func, Targs&&... args) -> std::future<typename std::result_of<Tfunc(Targs...)>::type>
    {
        using ret_type = typename std::result_of<Tfunc(Targs...)>::type;                // 分析任务函数返回类型
        auto pack = std::bind(std::forward<Tfunc>(func), std::forward<Targs>(args)...); // 打包任务函数
        auto task = std::make_shared<std::packaged_task<ret_type()>>(pack);             // 生成可执行的任务函数指针
        auto res = task->get_future();                                                  // 提前获取执行结果声明-从传入任务的线程延迟获取结果
        {
            std::unique_lock<std::mutex> lock(_mut);                // 锁住并准备将任务插入队列
            std::function<void()> func = [task](){ (*task)(); };    // 包装成统一可执行的任务函数
            if(_run) { _tasks.emplace(func); }                      // 加入任务到队列等待执行
        }
        _cond.notify_one(); // 通知随机一个线程去执行任务
        return res;
    }


    bool _run = true;                           // 运行标记
    std::mutex _mut;                            // 线程池锁
    std::condition_variable _cond;              // 条件变量
    std::vector<std::thread> _workers;          // 线程容器
    std::queue<std::function<void()>> _tasks;   // 任务队列
};


// 全局线程池
static Tpool<> *_sp_pool_ = new Tpool<>;


} // bhtools


#endif // TPOOL_H
