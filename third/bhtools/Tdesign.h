
#ifndef TDESIGN_H
#define TDESIGN_H

#include <tuple>


namespace bhtools {


// 执行模板职责链判断函数-可返回处理函数类型-带返回值
template<typename Treturn,typename Tclass,size_t now>
struct Twork_action_ret
{
    // 执行类对象的 accept 判断函数-如果为真则代表接收处理并结束
    // 返回值为 std::pair 类型-第一个类型是 bool 决定是否继续处理-第二个类型为外部处理函数自定义类型
    template<typename ...Targ>
    static std::pair<bool,Treturn> action(Tclass obj,Targ ...arg) 
    {
        auto it = std::get<now -1>(obj);
        auto pair = it.accept(arg...);
        if(pair.first) { return pair; }
        return Twork_action_ret<Treturn,Tclass,now -1>::action(obj,arg...);
    }
};

// 退出模板职责链-带返回值
template<typename Treturn,typename Tclass>
struct Twork_action_ret<Treturn,Tclass,0>
{
    template<typename ...Targ>
    static std::pair<bool,Treturn> action(Tclass obj,Targ ...arg) { return {}; }
};

// 处理函数无返回值的特殊处理-返回布尔值表示是否接收处理
template<typename Tclass,size_t now>
struct Twork_action
{
    template<typename ...Targ>
    static bool action(Tclass obj,Targ ...arg) 
    {
        auto it = std::get<now -1>(obj);
        if(it.accept(arg...)) { return true; }
        return Twork_action<Tclass,now -1>::action(obj,arg...);
    }
};
 
// 退出模板职责链
template<typename Tclass>
struct Twork_action<Tclass,0>
{
    template<typename ...Targ>
    static bool action(Tclass obj,Targ ...arg) { return false; }
};
 
// 模板职责链-可以通过实现鸭子函数来顺序处理函数
template<typename ...Tclass>
struct Twork_chain
{
    // 开始执行职责链-传入具体处理函数的参数-处理顺序为函数对象的倒序执行-确保第一个执行的函数放在最后-带返回值
    template<typename Treturn,typename ...Targ>
    std::pair<bool,Treturn> start(Targ ...arg)
    { 
        return Twork_action_ret<
                Treturn,
                std::tuple<Tclass...>,
                std::tuple_size<std::tuple<Tclass...>>::value
            >::action(_objs,arg...); 
    }

    // 开始执行职责链-无处理函数返回值
    template<typename ...Targ>
    bool start(Targ ...arg)
    { 
        return Twork_action<
                std::tuple<Tclass...>,
                std::tuple_size<std::tuple<Tclass...>>::value
            >::action(_objs,arg...); 
    }

    std::tuple<Tclass...> _objs;    // 存储类对象模板-非类指针而是类本身
};

//
//
//
//



} // bhtools



#endif // TDESIGN_H
