
#ifndef TPUBLISH_H
#define TPUBLISH_H

#include <iostream>
#include <map>
#include <set>
#include <functional>
#include <list>

namespace bhtools {


// 发布类模板消息传递
template<typename T>
struct Tpubmsg
{
    std::function<void(const T &)> _notify = nullptr;
};

// 发布类-单个类代表一种消息-只存储同订阅类型的对象
template<typename Targ,template<typename> class Tsub>
struct Tpublish
{
    // 插入对象
    void insert(Tsub<Targ> &sub)
    { _sub.emplace(&sub); }

    // 移除对象
    void remove(Tsub<Targ> &sub)
    {
        auto it = _sub.find(&sub);
        if(it != _sub.end())
        { _sub.erase(it); }
    }

    // 通知所有对象
    void notify(Targ arg)
    {
        for(auto sub:_sub)
        {
            if(sub->_notify)
            { sub->_notify(arg); }
        }
    }


    std::set<Tsub<Targ>*> _sub;    // 存储同类型订阅者
};


} // bhtools


#endif // TPUBLISH_H
