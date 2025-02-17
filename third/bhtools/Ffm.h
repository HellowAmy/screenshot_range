
#ifndef FFM_H
#define FFM_H

#include <string>
#include <vector>
#include <tuple>
#include <algorithm>

#include "Tbase.h"

namespace bhtools {


// 格式化基本规则-需要使用"{}"符号作为边界线-字符串从构造函数传入
struct Ffm_base
{
    // 构造需格式化的字符串
    Ffm_base(const std::string &str) : _str(str) { }

    // 规定替换字符的边界
    inline static std::string& strb() { static std::string s = "{"; return s; };
    inline static std::string& stre() { static std::string s = "}"; return s; };

    std::string _str;   // 格式化字符串
};


// 多位置格式化字符串-速度慢
struct Fsfm : public Ffm_base
{
    using Ffm_base::Ffm_base;

    // 传入替换参数-不限类型
    template<typename ...Tarr>
    inline std::string operator()(const Tarr &...arg)
    { return fms(arg...); }

    // 解析参数到容器-可随机位置替换
    template<typename T,typename ...Tarr>
    inline std::string fms(T && val,const Tarr &...arg)
    {
        _vec.push_back(Tto_string(std::forward<T>(val)));
        return fms(arg...);
    }

    // 格式化字符串
    template<typename ...Tarr>
    inline std::string fms()
    {
        std::string ret;
        size_t val = 0;
        size_t offset = 0;
        while(true)
        {
            auto tsub = Ffinds::find_range(_str,strb(),stre(),offset);
            if(std::get<0>(tsub))
            {
                ret += Ffinds::section_range(_str,offset,std::get<1>(tsub));
                size_t index = 0;
                bool ok = Tfrom_string(std::get<3>(tsub),index);

                if(ok && index < _vec.size()) { ret += _vec[index]; }
                else { ret += Ffinds::section_range(_str,std::get<1>(tsub),std::get<2>(tsub) +1); }

                offset = std::get<2>(tsub) +1;
            }
            else { break; }
        }
        ret += Ffinds::section_range(_str,offset,_str.size());
        return ret;
    }

    std::vector<std::string> _vec;  // 存储参数容器
};



// 快速格式化-不支持随机位置-单字符时速度提升一倍
struct Fffm : public Ffm_base
{
    using Ffm_base::Ffm_base;

    // 传入替换参数-不限类型
    template<typename ...Tarr>
    inline std::string operator()(const Tarr &...arg)
    { 
        if(strb().size() == 1 && stre().size() == 1) 
        { 
            _cfb = strb()[0];
            _cfe = stre()[0];
            return cfms(arg...); 
        }
        else { return sfms(arg...); }
    }

    // 查找并格式化字符串-单字符
    template<typename T,typename ...Tarr>
    inline std::string cfms(T && val,const Tarr &...arg)
    {
        if(proc(Tto_string(val),_cfb,_cfe)) { return cfms(arg...); }
        return cfms();
    }

    // 查找并格式化字符串-多字符
    template<typename T,typename ...Tarr>
    inline std::string sfms(T && val,const Tarr &...arg)
    {
        auto tsub = Ffinds::find_range(_str,strb(),stre(),_offset);
        if(std::get<0>(tsub))
        {
            _ret += Ffinds::section_range(_str,_offset,std::get<1>(tsub));
            _ret += Tto_string(val);
            _offset = std::get<2>(tsub) +1;
            return sfms(arg...);
        }
        return sfms();
    }

    // 终止函数
    inline std::string sfms() { return fms(); }

    // 终止函数
    inline std::string cfms() { return fms(); }

    // 退出时回收尾部字符
    inline std::string fms()
    {
        _ret += Ffinds::section_range(_str,_offset,_str.size());
        return _ret;
    }


    // 单字符时优化-速度翻倍
    inline bool proc(const std::string &val,char fb,char fe)
    {
        bool has = false;
        for(int i=_offset;i<_str.size();i++)
        {
            if(_str[i] == fb) { has = true; }
            else if(has && _str[i] == fe) { _offset = i+1; _ret += val; return true; }

            if(has == false) { _ret.push_back(_str[i]); }
        }
        return false;
    }

    char _cfb;              // 单字符优化-首部分割符
    char _cfe;              // 单字符优化-尾部分割符 
    size_t _offset = 0;     // 当前查找偏移
    std::string _ret;       // 存储结果字符串
};


} // bhtools


#endif // FFM_H
