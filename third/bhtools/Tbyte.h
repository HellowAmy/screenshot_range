
#ifndef TBYTE_H
#define TBYTE_H

#include <string>
#include <algorithm>
#include <tuple>


// 使用 Linux GCC 字节序处理
#ifdef __linux__
    #include <endian.h>
#endif


namespace bhtools {


// 检测系统定义的字节序
#ifndef BIG_ENDIAN
    #define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
    #define LITTLE_ENDIAN 1234
#endif
#ifndef NET_ENDIAN
    #define NET_ENDIAN BIG_ENDIAN
#endif
#ifndef BYTE_ORDER
    #if defined(__BYTE_ORDER)
        #define BYTE_ORDER  __BYTE_ORDER
    #elif defined(__BYTE_ORDER__)
        #define BYTE_ORDER  __BYTE_ORDER__
    #else
        #warning "Err Code Endian"
    #endif
#endif


// 定义字节序判断
#ifndef BHEND_IS_BIG
    #define BHEND_IS_BIG (BYTE_ORDER == BIG_ENDIAN)
#endif
#ifndef BHEND_IS_LIT
    #define BHEND_IS_LIT (BYTE_ORDER == LITTLE_ENDIAN)
#endif


// 查看当前字节序
#if BHEND_IS_BIG
    #define BHEND_NOW
#endif
#if BHEND_IS_LIT
    #define BHEND_NOW
#endif

//
//
//
//

// 最高位为1的值-主要用于高位右移判断-
static const uint8_t  _BH_ONE_HIGH_INT08_   = (1UL << 7);
static const uint16_t _BH_ONE_HIGH_INT16_   = (1UL << 15);
static const uint32_t _BH_ONE_HIGH_INT32_   = (1UL << 31);
static const uint64_t _BH_ONE_HIGH_INT64_   = (1UL << 63);
static const uint8_t  _BH_ONE_LOW_          = (1UL << 0);

// 16进制1248位为1的对比值-主要用于判断是否为1的位数
static const uint8_t  _BH_ONE_HEX_1_        = (1UL << 0);
static const uint8_t  _BH_ONE_HEX_2_        = (1UL << 1);
static const uint8_t  _BH_ONE_HEX_4_        = (1UL << 2);
static const uint8_t  _BH_ONE_HEX_8_        = (1UL << 3);

//
//
//
//

// 字节序操作
struct Tendian
{
    // 字节序转换
    template<typename T>
    inline static T swap_endian(T val)
    {
        size_t len = sizeof(val);
        std::string mem((char*)&val,len);
        for(size_t i=0;i<len;i++)
        {
            if(i >= len / 2) { break; }
            char tm = mem[i];
            mem[i] = mem[len -i -1];
            mem[len -i -1] = tm;
        }
        return *(T*)mem.c_str();
    };

    // 转为主机序-传入为一定是网络大端
    template<typename T>
    inline static T to_host(T val)
    {
        #if IS_BIG_ENDIAN
            return val;
        #else
            return swap_endian(val);
        #endif
    };

    // 转为网络序-传入不确定
    template<typename T>
    inline static T to_net(T val)
    {
        #if IS_BIG_ENDIAN
            return val;
        #else
            return swap_endian(val);
        #endif
    };
};

//
//
//
//

// 字节与进制转换
struct Tbyte
{
    // 2进制转2进制字符串
    template<typename T>
    inline static std::string b2_s2(T val)
    {
        std::string mem(size_byte(val),'0');
        for(size_t i=0;i<mem.size();i++)
        {
            if(val & _BH_ONE_LOW_) { mem[mem.size() -i -1] = '1'; }
            val >>= 1;
        }
        return mem;
    }

    // 2进制转2进制字符串-返回真实的字节序排布
    template<typename T>
    inline static std::string b2_s2_true(T val)
    {
        std::string ret;
        std::string mem((char*)&val,sizeof(val));
        for(size_t i=0;i<sizeof(val);i++)
        { ret += b2_s2<char>(mem[i]); }
        return ret;
    }


    // 2进制转8进制字符串
    template<typename T>
    inline static std::string b2_s8(T val)
    {
        size_t count = 0;
        size_t sum = 0;
        std::string ret;
        
        for(size_t i=0;i<size_byte(val);i++)
        {
            if(val & _BH_ONE_LOW_) { sum += calc_exp(2,count); }
            val >>= 1;
            count++;
            if(count == 3) { ret += std::to_string(sum); count = 0; sum = 0; }
        }
        ret += std::to_string(sum);
        std::reverse(ret.begin(),ret.end());
        return ret;
    }

    // 2进制转16进制字符串
    template<typename T>
    inline static std::string b2_s16(T val)
    {
        size_t count = 0;
        size_t sum = 0;
        std::string ret;
        
        for(size_t i=0;i<size_byte(val);i++)
        {
            if(val & _BH_ONE_LOW_) { sum += calc_exp(2,count); }
            val >>= 1;
            count++;
            if(count == 4) { ret += num_hex(sum); count = 0; sum = 0; }
        }
        std::reverse(ret.begin(),ret.end());
        return ret;
    }

    // 2进制转10进制字符串
    template<typename T>
    inline static std::string b2_s10(T val)
    {
        size_t sum = 0;
        size_t one = 0;
        std::string mem(size_byte(val),'0');
        for(size_t i=0;i<mem.size();i++)
        {
            if(val & _BH_ONE_LOW_) { one = 1; }
            else { one = 0; }
            sum += calc_exp(2,i) * one;
            val >>= 1;
        }
        return std::to_string(sum);
    }

    // 2进制字符串转2进制
    template<typename T>
    inline static T s2_b2(const std::string &s)
    {
        T t = 0;
        for(size_t i=0;i<s.size();i++)
        {
            t <<= 1;
            if(s[i] == '1') { t |= _BH_ONE_LOW_; }
        }
        return t;
    } 

    // 8进制字符串转2进制
    template<typename T>
    inline static T s8_b2(const std::string &s)
    {
        T t = 0;
        for(size_t i=0;i<s.size();i++)
        {
            std::vector<bool> vec = char_oct(s[i]);
            for(size_t a=vec.size();a>0;a--)
            {
                t <<= 1;
                if(vec[a-1]) { t |= _BH_ONE_LOW_; }
            }
        }
        return t;
    }

    // 10进制字符串转2进制
    template<typename T>
    inline static T s10_b2(const std::string &s)
    { T ret; std::istringstream ss(s); ss>>ret; return ret; }

    // 16进制字符串转2进制
    template<typename T>
    inline static T s16_b2(const std::string &s)
    {
        T t = 0;
        for(size_t i=0;i<s.size();i++)
        {
            std::vector<bool> vec = char_hex(s[i]);
            for(size_t a=vec.size();a>0;a--)
            {
                t <<= 1;
                if(vec[a-1]) { t |= _BH_ONE_LOW_; }
            }
        }
        return t;
    } 

    // 字节转16进制位数标记
    inline static std::vector<bool> char_hex(char c)
    {
        std::vector<bool> vec {false,false,false,false};
        size_t num = hex_num(c);
        for(int i=3;i>=0;i--)
        {
            size_t b = calc_exp(2,i);
            if(num >= b) { num -= b; vec[i] = true; }
        }
        return vec;
    }

    // 字节转8进制位数标记
    inline static std::vector<bool> char_oct(char c)
    {
        std::vector<bool> vec {false,false,false};
        size_t num = oct_num(c);
        for(int i=2;i>=0;i--)
        {
            size_t b = calc_exp(2,i);
            if(num >= b) { num -= b; vec[i] = true; }
        }
        return vec;
    }

    // 数字转16进制符号
    inline static char num_hex(size_t val)
    { 
        if(val > 15) { return '0'; }
        if(val <= 9) { return val + '0'; }
        return val = val - 10 + 'a';
    }

    // 16进制符号转数字
    inline static size_t hex_num(char c)
    { 
        if(is_hex_range(c))
        {
            if(is_hex_digit(c)) { return c - '0'; }
            else if(is_hex_little(c)) { return c - 'a' + 10; }
            else if(is_hex_big(c)) { return c - 'A'  + 10; }
        }
        return 0;
    }

    // 8进制符号转数字
    inline static size_t oct_num(char c)
    { 
        if(is_oct_range(c)) { return c - '0'; }
        return 0;
    }

    // 判断为数字
    inline static bool is_hex_digit(char c)
    {
        if((c >= '0' && c <= '9')) { return true; }
        return false;
    }

    // 判断为小写字母
    inline static bool is_hex_little(char c)
    {
        if(c >= 'a' && c <= 'f') { return true; }
        return false;
    }

    // 判断为大写字母
    inline static bool is_hex_big(char c)
    {
        if(c >= 'A' && c <= 'F') { return true; }
        return false;
    }

    // 判断为16进制字符
    inline static bool is_hex_range(char c)
    {
        if(is_hex_digit(c) || is_hex_little(c) || is_hex_big(c)) { return true; }
        return false;
    }

    // 判断为8进制字符
    inline static bool is_oct_range(char c)
    {
        if((c >= '0' && c <= '7')) { return true; }
        return false;
    }

    // 转大写-单字符
    inline static char to_upper(char c)
    {
        const int w = 'a' - 'A';
        if(c >= 'a' && c <= 'z') { c -= w; }
        return c;
    }

    // 转小写-单字符
    inline static char to_lower(char c)
    {
        const int w = 'a' - 'A';
        if(c >= 'A' && c <= 'Z') { c += w; }
        return c;
    }

    // 转大写-字符串
    inline static std::string to_upper(const std::string &s)
    {
        std::string ret(s.size(),'0');
        for(int i=0;i<s.size();i++)
        { ret[i] = to_upper(s[i]); }
        return ret;
    }

    // 转小写-字符串
    inline static std::string to_lower(const std::string &s)
    {
        std::string ret(s.size(),'0');
        for(int i=0;i<s.size();i++)
        { ret[i] = to_lower(s[i]); }
        return ret;
    }

    // 返回内存的bit长度
    template<typename T>
    inline static size_t size_byte(T val)
    { return sizeof(val) * 8; }

    // 正整数的指数计算
    inline static size_t calc_exp(size_t val,size_t exp)
    { 
        if(exp == 0) { return 1; }
        if(exp == 1) { return val; }
        size_t sum = val;
        for(size_t i=0;i<exp-1;i++)
        { sum *= val; }
        return sum; 
    }

};


} // bhtools


#endif // TBYTE_H
