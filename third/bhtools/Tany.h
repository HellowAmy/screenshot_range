
#ifndef TANY_H
#define TANY_H

#include <iostream>
#include <typeindex>
#include <memory>

#include "Topt.h"

namespace bhtools {


// 存储任何值-使用无类型指针存储数据
struct Tany
{
    // 类型保存-保存的基类指针-用于类型擦除
    struct type_base
    {
        virtual ~type_base() {}
        virtual type_base* make() = 0;
    };

    // 类型保存-存储具体值得模板类-用于生成指针
    template<typename T>
    struct type_pack : public type_base
    {
        // 传入需要保存的值
        type_pack(const T &val) : _val(val) {}

        // 将值内容分配到指针
        type_base* make() override 
        { return new type_pack<T>(_val); }

        T _val;     // 动态保存的值
    };

    // 默认无数据
    Tany() : _type(typeid(void)) {}

    // 有数据
    template<typename T>
    Tany(const T &val) : _type(typeid(T))
    { set(val); }

    // 拷贝构造只复制保存值
    Tany(const Tany &any) : _type(typeid(any._type))
    { set(any); }

    // 字符串统一转成std::string类型
    Tany(const char *val) : _type(typeid(std::string))
    { set(std::string(val)); }

    // 作用同构造
    Tany& operator=(const char *val)
    { set(std::string(val)); return *this; }

    // 作用同构造
    template<typename T>
    Tany& operator=(const T &val)
    { set(val); return *this; }

    // 作用同构造
    Tany& operator=(const Tany &any)
    { set(any); return *this; }

    // 释放保存值
    ~Tany() { reset(); }

    // 构造传入值时创建新的保存值指针
    template<typename T>
    void set(const T &val)
    {
        _type = std::type_index(typeid(T));
        _val = type_pack<T>(val).make();
    }

    // 释放当前保存指针并拷贝传入保存值
    void set(const Tany &any)
    {
        if(_val) { reset(); }
        _type = any._type;
        _val = any.copy();
    }

    // 重置状态和释放保存值
    void reset()
    {
        _type = std::type_index(typeid(void));
        if(_val) { delete _val; _val = nullptr; }
    }

    // 查询使用的类型
    template<typename T>
    bool use() { return _type == std::type_index(typeid(T)) && _val != nullptr; }

    // 直接返回保存值-需要先查询
    template<typename T>
    T& value() { return (static_cast<type_pack<T>*>(_val))->_val; }

    // 返回较为安全的可选版本
    template<typename T>
    Topt<T> value_opt()
    {
        if(use<T>) { return Topt<T>(value<T>()); }
        return Topt<T>();
    }

    // 用于拷贝构造复制对象保存值
    type_base* copy() const
    { 
        if(_val) { return _val->make(); }
        return nullptr;
    }


    std::type_index _type;      // 保存值得类型
    type_base *_val = nullptr;  // 保存值的无类型指针
};



} // bhtools

#endif // TANY_H
