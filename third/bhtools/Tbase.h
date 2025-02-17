
#ifndef TBASE_H
#define TBASE_H

#include <string>
#include <type_traits>
#include <sstream>
#include <tuple>

#include "Topt.h"

namespace bhtools {


// 默认C++ sstream类型转字符串-可兼容C++ cout的输入输出
struct Tsstream_string
{
    template<typename T>
    std::string operator()(const T &val) { std::stringstream ss; ss<<val; return ss.str(); }
};

//
//
//
//

// 各类型转字符串函数
template<typename T>
inline static std::string Tto_string(const T &val) 
{ return std::to_string(val); }

inline static std::string Tto_string(const char *val) 
{ return val; }

inline static std::string Tto_string(const std::string &val) 
{ return val; }

template<typename T>
inline static std::string Tto_string(T* val) 
{ std::stringstream ss; ss<<val; return ss.str(); }

inline static std::string Tto_string(void *val) 
{ std::stringstream ss; ss<<val; return ss.str(); }

//
//
//
//

// 字符串转各类型
template<typename T>
bool Tfrom_string(const std::string &str,T &ret)
{
    bool suc = true;
    try {
        if(std::is_same<T,double>::value) 
        { ret = std::stod(str); }
        else if(std::is_same<T,float>::value) 
        { ret = std::stof(str); }
        else if(std::is_same<T,int>::value || std::is_same<T,unsigned int>::value) 
        { ret = std::stoi(str); }
        else if(std::is_same<T,long>::value || std::is_same<T,unsigned long>::value) 
        { ret = std::stol(str); }
        else if(std::is_same<T,long long>::value || std::is_same<T,unsigned long long>::value) 
        { ret = std::stoll(str); }
        else { suc = false; }
    } catch(...) { suc = false; }
    return suc;
}

template<typename T>
T Tfrom_string(const std::string &str)
{ T ret; std::istringstream ss(str); ss>>ret; return ret; }

template<typename T>
Topt<T> Tfrom_string_opt(const std::string &str)
{ 
    T ret; 
    if(Tfrom_string(str,ret)) { return Topt<T>(ret); } 
    return Topt<T>();
}

//
//
//
//

// 处理字符串位置
struct Ffinds
{
    // 发现子串位置
    // 返回值 < 是否成功 - 成功的下标 >
    inline static std::tuple<bool,size_t> 
    find_sub(const std::string &str,const std::string &sub,size_t offset = 0)
    {
        auto it = str.find(sub,offset);
        if(it != std::string::npos)
        { return std::make_tuple(true,it); }
        return std::make_tuple(false,0);
    }

    // 发现子串范围位置-标记位置是最开始和最末尾
    // 传参数 < 字符串 - 开始标记 - 末尾标记 - 偏移位置 >
    // 返回值 < 是否成功 - 开始标记的首位 - 末尾标记的末尾 - 标记中间的字符串 >
    inline static std::tuple<bool,size_t,size_t,std::string> 
    find_range(const std::string &str,const std::string &fb,const std::string &fe,size_t offset = 0)
    {
        auto rb = find_sub(str,fb,offset); 
        auto re = find_sub(str,fe,offset); 
        if(std::get<0>(rb) && std::get<0>(re))
        { 
            size_t ib = std::get<1>(rb);
            size_t ie = std::get<1>(re);
            return std::make_tuple(true,ib,ie + fe.size() -1,section_range(str,ib + fb.size(),ie)); 
        }
        return std::make_tuple(false,0,0,"");
    }

    // 切割字符串-传入开始和末尾下标-获取两个下标间的字符串-开区间和闭区间
    inline static std::string section_range(const std::string &str,size_t ib,size_t ie)
    {
        std::string ret;
        if(ie > ib && ie <= str.size())
        {
            for(size_t i=ib;i<ie;i++)
            {
                ret.push_back(str[i]);
            }
        }
        return ret;
    }
};




} // bhtools


#endif // TBASE_H
