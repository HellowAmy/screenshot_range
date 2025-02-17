
#ifndef TOPT_H
#define TOPT_H

namespace bhtools {


// 可选返回值
template<typename T>
struct Topt
{
    // 未使用状态
    constexpr Topt() : _use(false) {}

    // 使用并传递参数
    template <typename... Targ>
    constexpr Topt(Targ&&... arg) : _use(true), _val(std::forward<Targ>(arg)...) {}

    // 重载-判断有效
    operator bool() { return use(); }

    // 重载-取值
    T& operator *() { return value(); }

    // 使用状态
    bool use() { return _use; } 

    // 返回值引用
    T& value() { return _val; }

    // 重置为初始状态
    void reset() { _use = false; _val.~T();  } 

    // 设置返回值
    template <typename... Targ>
    void set(Targ&&... arg) 
    { 
        if(_use) { reset(); }
        _val = T(std::forward<Targ>(arg)...); 
        _use = true;
    }

    bool _use;  // 使用标记
    T _val;     // 返回值
};


} // bhtools

#endif // TOPT_H
