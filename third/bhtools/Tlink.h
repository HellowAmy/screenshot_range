
#ifndef TLINK_H
#define TLINK_H

#include <iostream>

#include <functional>


namespace bhtools {


// 数据链接视图
template<typename T,template<typename> class Tm>
struct Tvlink
{
    // 绑定通知
    Tvlink(Tm<T> &m) 
    { 
        _m = &m;
        _m->_fn_notify = [this](bool self) {
            if(_fn_data_change) { _fn_data_change(data(),!self); }
        }; 
    }

    // 设置数据
    void set_data(const T &val) 
    { _m->set_notify_data(val,true,false); }

    // 设置数据-不通知
    void set_nonotify_data(const T &val) 
    { _m->set_notify_data(val,false,false); }

    // 返回数据
    T data() { return _m->data(); }


    std::function<void(T,bool)> _fn_data_change = nullptr;  // 数据更改通知-外部回调

    Tm<T> *_m = nullptr;   // 模型指针
};


// 数据链接模型
template<typename T>
struct Tmlink
{
    Tmlink(const T &val) { set_notify_data(val); }

    // 设置数据
    void set_data(const T &val) { set_notify_data(val); }

    // 设置数据-不通知
    void set_nonotify_data(const T &val) 
    { set_notify_data(val,false,true); }

    // 返回数据
    T data() { return _data; }

    // 更新数据并通知视图 
    // [ notify == false    : 同步数据但不通知 ]
    // [ notify == true     : 通知且同步数据 ]
    void set_notify_data(const T &val,bool notify = true,bool self = true)
    {
        _data = val;
        if(notify && _fn_data_change) { _fn_data_change(val,self); }
        if(notify && _fn_notify) { _fn_notify(self); }
    }


    std::function<void(bool)> _fn_notify = nullptr;         // 通知函数-内部回调
    std::function<void(T,bool)> _fn_data_change = nullptr;  // 数据更改通知-外部回调

    T _data;    // 存储源数据
};



} // bhtools


#endif // TLINK_H
