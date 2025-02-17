
#ifndef FSTM_H
#define FSTM_H

#include <string>
#include <algorithm>

#include "Tbase.h"

namespace bhtools {


// 切割字符串
struct Fstm
{
    Fstm(const std::string &str) : _str(str) {}

    // 退出函数
    template<typename ...Tarr>  
    inline std::string section(){ return _str; }

    // 根据分隔符和获取部分标记切割字符串-可以对同一字符进行多次切割
    template<typename ...Tarr>
    inline std::string section(const std::string &sf,int ib,int ie,const Tarr &...arg)
    {
        if(_str == "") { return ""; }

        if(ib >= 0 && ie >= 0) 
        { 
            // 全整数切割-对比差值
            size_t ipb = find_part(_str,sf,ib);
            size_t ipe = find_part(_str,sf,ie,true);
            if(ipe > ipb) { _str = _str.substr(ipb,ipe - ipb); }
            else { _str = ""; }
        }
        else if(ib < 0 && ie < 0) 
        {
            // 负数切割-转成整数后反向查找-矫正反向下标后切割
            ib = revise_reverse_index(ib);
            ie = revise_reverse_index(ie);
            size_t ipb = find_part(_str,sf,ib,false,true);
            size_t ipe = find_part(_str,sf,ie,true,true);
            if(_str.size() >= ipe && ipe > ipb) { _str = _str.substr(_str.size() - ipe,ipe - ipb); }
            else { _str = ""; }
        }
        else if(ib < 0) 
        { 
            // 前负数切割-从矫正的前下标开始-获取前后下标的差值
            ib = revise_reverse_index(ib);
            size_t ipb = find_part(_str,sf,ib,true,true);
            size_t ipe = find_part(_str,sf,ie,true);
            if((_str.size() >= ipb) && (ipe > (_str.size() - ipb))) 
            { _str = _str.substr(_str.size() - ipb, ipe - (_str.size() - ipb)); }
            else { _str = ""; }
        }
        else if(ie < 0) 
        { 
            // 后负数切割-从前下标开始-对比矫正的后下标差值进行切割
            ie = revise_reverse_index(ie);
            size_t ipb = find_part(_str,sf,ib,false);
            size_t ipe = find_part(_str,sf,ie,false,true);
            if(_str.size() > ipe) { _str = _str.substr(ipb,_str.size() - ipe); }
            else { _str = ""; }
        }

        return section(arg...);
    }

    // 切割参数进入
    template<typename ...Tarr>
    inline std::string operator()(const Tarr &...arg)
    {   return section(arg...); }

    // 修正传入负数参数所属的部分
    inline static int revise_reverse_index(int index)
    {
        index *= -1;
        index -= 1;
        return index;
    }

    // 发现被切割符分割部分的下标-back选项为返回该部分的尾部-reverse选项为反转字符后再查找下标
    inline static size_t find_part(std::string str,const std::string &sub,size_t part,bool back = false,bool reverse = false)
    {
        if(str == "" || str.size() <= sub.size()) { return 0; }
        if(reverse) { std::reverse(str.begin(),str.end()); }
        if(back) { part += 1; }

        bool over = false;
        size_t index = 0;
        for(size_t i=0;i<part;i++)
        {
            if(over) { return 0; }
            auto btup = Ffinds::find_sub(str,sub,index);
            if(std::get<0>(btup))
            {
                index = std::get<1>(btup);
                index += sub.size();
            }
            else { index = str.size(); over = true; }
        }
        if(back && over == false && index != 0) { index -= sub.size(); }
        return index;
    }


    std::string _str;   // 存储需切割字符-处理结束时返回的结果
};



} // bhtools

#endif // FSTM_H
