
#ifndef TUNPACK_H
#define TUNPACK_H

#include <iostream>
#include <functional>
#include <mutex>

#include "Tbyte.h"

namespace bhtools {

// 拆包参数-分隔符
struct Tunfd_delimit
{
    std::string _delimit = "\n";
};

// 拆包参数-包长度-仅提供字节长度
struct Tunfd_length
{
    uint16_t _length = 0;
};

// 
// 
// 
// 

// 生成包数据-分隔符
template<typename T = Tunfd_delimit>
struct Tunfmake_delimit
{
    // 传入数据返回包体
    static std::string make(const std::string &ctx)
    {
        T arg; 
        std::string pack = ctx + arg._delimit;
        return pack;
    }
};

// 生成包数据-包长度
template<typename T = Tunfd_length>
struct Tunfmake_length 
{
    // 传入数据返回包体
    static std::string make(const std::string &ctx)
    {
        T arg;
        arg._length = ctx.size();
        arg._length = Tendian::to_net(arg._length);
        std::string pack = std::string((char*)&arg._length,sizeof(arg._length)) + ctx;
        return pack;
    }
};

// 
// 
// 
// 

// 拆包函数-分隔符
template<typename T = Tunfd_delimit>
struct Tunf_delimit
{
    // 拆包函数
    void unpack(const std::string &ctx,bool ths = true)
    {
        std::unique_lock<std::mutex> lock(_mux); // 上锁

        // 不合理退出
        if(_arg._delimit.size() == 0) { return; }

        std::string all = _save + ctx;
        size_t go = _save.size();
        size_t sidel = _arg._delimit.size();

        // 循环拆包-如果拆包状态未退出下次从0下标开始
        if(_into) { go = 0; } 
        while(go < all.size())
        {
            // 检查到分隔符
            if(all[go] == _arg._delimit[0])
            {

                // 进入拆包状态
                _into = true; 
                if((all.size() - go) >= sidel)
                {
                    // 检查拆包分隔符是否为字符串
                    bool match = true;
                    if(_arg._delimit.size() > 1)
                    {
                        for(size_t x=1;x<sidel;x++)
                        {
                            if(all[go+x] != _arg._delimit[x])
                            {
                                match = false;
                                break;
                            }
                        }
                    }

                    // 匹配成功-触发回调并重置状态
                    if(match)
                    {
                        if(_fn_cb) { _fn_cb(std::string(all.begin(),all.begin() + go)); }

                        all = std::string(all.begin() + go + sidel,all.end());
                        go = 0;
                    }
                    _into = false;
                }
            }
            go++;
        }
        _save = all;
    }

    // 注册拆包成功的回调函数
    void read_cb(std::function<void(const std::string &)> fn)
    { _fn_cb = fn; }


    T _arg;                 // 拆包函数参数
    bool _into = false;     // 拆包状态
    std::string _save;      // 保存上次数据
    std::mutex _mux;        // 线程锁
    std::function<void(const std::string &)> _fn_cb = nullptr;      // 回调函数
};

// 拆包函数-包长度
template<typename T = Tunfd_length>
struct Tunf_length
{
    // 拆包函数
    void unpack(const std::string &ctx,bool ths = true)
    {
        std::unique_lock<std::mutex> lock(_mux); // 上锁

        std::string all = _save + ctx;
        size_t len_head =  sizeof(_arg._length);

        while(true)
        {
            // 包头字节不足退出
            if(all.size() < len_head) { break; }

            // 数据长度不足退出
            size_t len_data = all.size() - len_head;
            std::string head = std::string(all.begin(),all.begin() + len_head);
            _arg._length = *(decltype(_arg._length)*)head.c_str();
            _arg._length = Tendian::to_host(_arg._length);
            if(len_data < _arg._length) { break; }

            // 数据达到或超出包范围-回调包体数据
            std::string data = std::string(all.begin() + len_head,all.begin() + len_head + _arg._length);
            if(_fn_cb) { _fn_cb(data); }  

            all = std::string(all.begin() + len_head + _arg._length,all.end());
        }

        _save = all;
    }

    // 注册拆包成功的回调函数
    void read_cb(std::function<void(const std::string &)> fn)       
    { _fn_cb = fn; }


    T _arg;                 // 拆包函数参数
    std::string _save;      // 保存上次数据
    std::mutex _mux;        // 线程锁    
    std::function<void(const std::string &)> _fn_cb = nullptr;      // 回调函数
};

// 
// 
// 
// 

// 数据拆包组合
template<template<typename> class Tfn,template<typename> class Tmake,typename Ttype>
struct Tunpack
{
    // 拆包函数
    void unpack(const std::string &ctx,bool ths = true)
    { _fn.unpack(ctx,ths); }

    // 注册拆包成功的回调函数
    void read_cb(std::function<void(const std::string &)> fn)       
    { _fn.read_cb(fn); }

    // 生成包数据
    std::string make(const std::string &ctx)
    { return Tmake<Ttype>::make(ctx); }

    Tfn<Ttype> _fn;     // 拆包类型
};

//
//
//
//

// 快捷默认使用
typedef Tunpack<Tunf_length,Tunfmake_length,Tunfd_length> Tunpack_len;
typedef Tunpack<Tunf_delimit,Tunfmake_delimit,Tunfd_delimit> Tunpack_del;


} // bhtools

#endif // TUNPACK_H
