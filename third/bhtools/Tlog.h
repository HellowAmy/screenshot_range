
#ifndef TLOG_H
#define TLOG_H

#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <thread>
#include <queue>
#include <mutex>
#include <memory>

#include "Ftime.h"
#include "Tbase.h"


namespace bhtools {


// 不同等级日志
namespace bhenum {
    
    enum class level4
    {
        e_deb,
        e_inf,
        e_war,
        e_err,
    };
    enum class level8
    {
        e_all,
        e_tra,
        e_deb,
        e_inf,
        e_war,
        e_err,
        e_fat,
        e_off,
    };
}

//
//
//
//

// 日志等级判断
template<typename Tlevel>
struct Tlog_level 
{
    Tlog_level() { }

    Tlog_level(Tlevel el) { set_level(el); }

    inline Tlog_level& set_level(Tlevel el)  { _level = el; return *this; }

    inline void status(bool ok) { _ok = ok; }

    inline bool pass() { return _ok; }

    inline void comp(const Tlog_level &el) 
    { if(el._level >= _level) { _ok = true; } else{ _ok = false; } }

    inline Tlog_level& operator<<(Tlevel el)
    { return set_level(el); };

    bool _ok = false;   // 是否满足等级
    Tlevel _level;      // 设置等级
};


// 日志缓冲区
struct Tlog_buf 
{ 
    template<typename T>
    inline void push(const T &val) 
    { _str += Tto_string(val); }

    inline void clear() 
    { _str.clear(); }

    inline std::string value() const
    { return _str; }

    std::string _str;   // 缓存内容
};


// 日志结束类标记
struct Tlog_end { };


// 打印固定格式的时间
struct Tlog_time 
{
    inline static std::string get_time()
    {
        Ftimes::data d = Ftimes::to_data(Ftimes::time_now());
        d.hou += 8;
        return Ftimes::format_time(d,"%4Y-%2M-%2D.%2H:%2T:%2S.%3L");
    };
};


// 日志基本类-用于创建各种输出类型的模板-不提供打印只提供写入与缓存
template<typename Tlevel,typename Tbuf,typename Tend,typename Tout>
struct Tlog_base
{
    // 设置日志等级-不显示低等级内容
    inline void set_level(Tlevel el) { _el = el; } 

    // 触发换行输出
    inline void out_end() { _out.out(_buf); }

    // 传入等级
    inline Tlog_base& operator<<(Tlevel el)
    { _el.comp(el); return *this; };

    // 打印内容
    inline Tlog_base& operator<<(Tend end)
    { if(_el.pass()){ out_end(); _buf.clear(); _el.status(false); } return *this; };

    // 传入换行
    template<typename T>
    inline Tlog_base& operator<<(const T &log)
    { if(_el.pass()){ _buf.push(log); } return *this; };


    Tlevel _el;     // 日志等级划分枚举
    Tbuf _buf;      // 日志写入缓存内容
    Tout _out;      // 日志输出类
};


// 异步文件日志
// 比对普遍文件日志在一千万输出时有十分之一的速度提升
template<typename Tout,typename Tbuf,size_t Ttime = 1000>
struct Tlog_asyn
{
    Tlog_asyn() { _th = std::make_shared<std::thread>(&Tlog_asyn::work_write,this); }

    ~Tlog_asyn() { _run = false; _th->join(); }

    // 获取到日志内容并加入队列
    void out(const Tlog_buf &buf) { push_queue(buf); }    

    // 将队列内日志写入日志输出类
    void work_write()
    {
        while(_run)
        {
            {
                std::unique_lock<std::mutex> lock(_mut);
                while (_que.empty() == false)
                {
                    _out.out(_que.front());
                    _que.pop(); 
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(Ttime));
        }
    }

    // 加入到队列
    inline void push_queue(const Tbuf &txt)
    { std::unique_lock<std::mutex> lock(_mut); _que.push(txt); } 


    bool _run = true;                   // 写入线程运行标记
    Tout _out;                          // 日志输出类
    std::mutex _mut;                    // 队列锁
    std::queue<Tbuf> _que;              // 写入日志队列
    std::shared_ptr<std::thread> _th;   // 写入线程
};


//  空打印-用于测试 << 操作的耗时
struct Tlog_null 
{
    void out(const Tlog_buf &buf) { }    
};


// 命令行打印
struct Tlog_cmd 
{
    void out(const Tlog_buf &buf) { std::cout<<buf.value()<<std::endl; }    
};


// 文件打印
// 打印一千万行时换行符比换行函数速度提升近一倍-但换行符是可能会导致无法及时刷新缓冲区丢失数据
struct Tlog_file
{    
    void out(const Tlog_buf &buf) { _fs<<buf.value()<<"\n"; update_file(); } 

    //初始化日志
    inline bool reopen(const std::string &file,bool app = true)
    {
        _file = file;
        if(app) { _mode = std::ios::app; } 
        else { _mode = std::ios::out; } 

        if(_fs.is_open()) { _fs.close(); }
        _fs.open(_file,_mode);
        return _fs.is_open();
    }
    
    // 设置循环最大文件数-默认无限
    inline void set_limit(size_t max) { _limit_max = max; } 

    // 设置单个文件最大长度-默认64M
    inline void set_length(size_t len) { _len_max = len; }  

    // 设置每行写入时是否刷新文件
    inline void set_flush(bool flush) { _flush = flush; }  

    // 超出最大文件限制后更新文件名
    bool update_file() 
    {
        if(_flush) { _fs.flush(); }
        if(_len_max < _fs.tellg())
        {
            if(_limit_max == 0) { return write_unlimited(); }
            else { return write_limit(); } 
        }
        return true;
    }

    // 无限制日志
    bool write_unlimited()
    {
        _fs.close();
        for(int i=_limit_now;;i++)
        {
            std::string new_file = std::to_string(i) + "_" + _file;
            if(exist_file(new_file) == false)
            { 
                rename(_file.c_str(),new_file.c_str()); 
                _limit_now++;
                break;
            }
        }
        _fs.open(_file,_mode);
        return _fs.is_open();
    }

    // 限制日志数量
    bool write_limit() 
    {
        _fs.close();
        std::string new_file = std::to_string(_limit_now) + "_" + _file;
        rename(_file.c_str(),new_file.c_str());
        _limit_now++;
        if(_limit_now > _limit_max) { _limit_now = 1; }
        
        _fs.open(_file,_mode);
        return _fs.is_open();
    }

    // 判断文件是否存在
    static bool exist_file(const std::string &filename)
    { std::ifstream f(filename); return f.is_open(); }


    bool _flush = true;             // 刷入缓冲区
    size_t _limit_max = 0;          // 日志文件限制数量
    size_t _limit_now = 1;          // 当前写入日志
    size_t _len_max = (1 << 26);    // 最大长度--64M
    std::string _file;              // 文件名
    std::fstream _fs;               // 文件对象
    std::ios_base::openmode _mode;  // 文件打开模式
};

// 空打印日志 等级4
struct Tlog_null4 : public Tlog_base <Tlog_level<bhenum::level4>,Tlog_buf,Tlog_end,Tlog_null> 
{
    Tlog_null4() { set_level(Tlog_level<bhenum::level4>(bhenum::level4::e_deb)); }
};


// 命令行打印日志 等级4
struct Tlog_cmd4 : public Tlog_base <Tlog_level<bhenum::level4>,Tlog_buf,Tlog_end,Tlog_cmd> 
{
    Tlog_cmd4() { set_level(Tlog_level<bhenum::level4>(bhenum::level4::e_deb)); }
};


// 命令行打印日志 等级8
struct Tlog_cmd8 : public Tlog_base <Tlog_level<bhenum::level8>,Tlog_buf,Tlog_end,Tlog_cmd> 
{
    Tlog_cmd8() { set_level(Tlog_level<bhenum::level8>(bhenum::level8::e_all)); }
};


// 文件打印日志 等级4
struct Tlog_file4 : public Tlog_base <Tlog_level<bhenum::level4>,Tlog_buf,Tlog_end,Tlog_file> 
{
    Tlog_file4(const std::string &file = "Tflog.log") 
    { 
        set_level(Tlog_level<bhenum::level4>(bhenum::level4::e_deb)); 
        _out.reopen(file); 
        _out.set_flush(true);
    }
    void flush() { _out._fs.flush(); }
};

// 文件打印日志-异步 等级4
struct Tlog_asyn_file4 : public Tlog_base <Tlog_level<bhenum::level4>,Tlog_buf,Tlog_end,Tlog_asyn<Tlog_file,Tlog_buf,1000>> 
{
    Tlog_asyn_file4(const std::string &file = "Taflog.log") 
    { 
        set_level(Tlog_level<bhenum::level4>(bhenum::level4::e_deb)); 
        _out._out.reopen(file); 
        _out._out.set_flush(true);
    }
    void flush() { _out._out._fs.flush(); }
};

//
//
//
//

// 标准容器打印
template<typename T>
std::string Tlog_con(const T& con,size_t len = 1,const std::string &flg = " ",const std::string &prev = "| ")
{
    std::string ret = "\n";
    ret += prev + "size: " + std::to_string(con.size());
    ret += "\n" + prev;
    size_t count = 0;
    for(const auto &a:con)
    {
        if(len != 0 && count >= len) { count = 0; ret += "\n" + prev; }
        ret += Tto_string(a) + flg;
        count++;
    }
    ret += "\n";
    return ret;
}

//
//
//
//

// 日志格式化内容
#define BHLOG_FORMAT(tips,el,...)                           \
    el<<tips "["<<__FILE__<<":<"<<__LINE__<<">] <<<< "      \
    <<__VA_ARGS__


// 格式化VSCode快捷导航格式
#define BHLOG_FORMAT_VSC(tips,el,...)               \
    el<<tips "<<<< "                                \
    <<__VA_ARGS__                                   \
    <<" >>>>["<<__FILE__<<":"<<__LINE__             \
    <<"][" +bhtools::Tlog_time::get_time()+"] "     \


// 生成打印字符串
#define BHLOG_PRINT(out,tips,end,BHDF_FM,el,...)        \
    out<<BHDF_FM(tips,el,__VA_ARGS__)<<end              \


// 生成快捷打印宏-带颜色-等级4
#define BHLOG_MAKE_COL_L4D(out,end,el,...)                                  \
    BHLOG_PRINT(out,"\033[32m[Deb]","\033[0m"<<end,BHLOG_FORMAT_VSC,        \
    el->set_level(bhtools::bhenum::level4::e_deb),__VA_ARGS__)              \

#define BHLOG_MAKE_COL_L4I(out,end,el,...)                                  \
    BHLOG_PRINT(out,"\033[36m[Inf]","\033[0m"<<end,BHLOG_FORMAT_VSC,        \
    el->set_level(bhtools::bhenum::level4::e_inf),__VA_ARGS__)              \

#define BHLOG_MAKE_COL_L4W(out,end,el,...)                                  \
    BHLOG_PRINT(out,"\033[33m[War]","\033[0m"<<end,BHLOG_FORMAT_VSC,        \
    el->set_level(bhtools::bhenum::level4::e_war),__VA_ARGS__)              \

#define BHLOG_MAKE_COL_L4E(out,end,el,...)                                  \
    BHLOG_PRINT(out,"\033[31m[Err]","\033[0m"<<end,BHLOG_FORMAT_VSC,        \
    el->set_level(bhtools::bhenum::level4::e_err),__VA_ARGS__)              \


// 生成快捷打印宏-等级4
#define BHLOG_MAKE_L4D(out,end,el,...)                          \
    BHLOG_PRINT(out,"[Deb]",end,BHLOG_FORMAT_VSC,               \
    el->set_level(bhenum::level4::e_deb),__VA_ARGS__)           \

#define BHLOG_MAKE_L4I(out,end,el,...)                          \
    BHLOG_PRINT(out,"[Inf]",end,BHLOG_FORMAT_VSC,               \
    el->set_level(bhenum::level4::e_inf),__VA_ARGS__)           \

#define BHLOG_MAKE_L4W(out,end,el,...)                          \
    BHLOG_PRINT(out,"[War]",end,BHLOG_FORMAT_VSC,               \
    el->set_level(bhenum::level4::e_war),__VA_ARGS__)           \

#define BHLOG_MAKE_L4E(out,end,el,...)                          \
    BHLOG_PRINT(out,"[Err]",end,BHLOG_FORMAT_VSC,               \
    el->set_level(bhenum::level4::e_err),__VA_ARGS__)           \


// 快捷打印宏
#ifndef BHLOG_CLOSE_LOG

    // 快捷命令行打印 等级4
    #ifndef BHLOG_CLOSE_COL
        #define vlogd(...) BHLOG_MAKE_COL_L4D((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vlogi(...) BHLOG_MAKE_COL_L4I((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vlogw(...) BHLOG_MAKE_COL_L4W((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vloge(...) BHLOG_MAKE_COL_L4E((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #else
        #define vlogd(...) BHLOG_MAKE_L4D((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vlogi(...) BHLOG_MAKE_L4I((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vlogw(...) BHLOG_MAKE_L4W((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
        #define vloge(...) BHLOG_MAKE_L4E((*bhtools::_sp_cmd4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #endif

    // 快捷文件打印 等级4
    #define flogd(...) BHLOG_MAKE_L4D((*bhtools::_sp_file4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define flogi(...) BHLOG_MAKE_L4I((*bhtools::_sp_file4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define flogw(...) BHLOG_MAKE_L4W((*bhtools::_sp_file4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define floge(...) BHLOG_MAKE_L4E((*bhtools::_sp_file4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)

    // 快捷文件打印 等级4
    #define aflogd(...) BHLOG_MAKE_L4D((*bhtools::_sp_afile4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define aflogi(...) BHLOG_MAKE_L4I((*bhtools::_sp_afile4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define aflogw(...) BHLOG_MAKE_L4W((*bhtools::_sp_afile4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define afloge(...) BHLOG_MAKE_L4E((*bhtools::_sp_afile4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    
    // 快捷空值打印 等级4
    #define nulogd(...) BHLOG_MAKE_L4D((*bhtools::_sp_null4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define nulogi(...) BHLOG_MAKE_L4I((*bhtools::_sp_null4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define nulogw(...) BHLOG_MAKE_L4W((*bhtools::_sp_null4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)
    #define nuloge(...) BHLOG_MAKE_L4E((*bhtools::_sp_null4_),(*bhtools::_sp_end_),bhtools::_sp_level4_,__VA_ARGS__)

#else
    #define vlogd(...)
    #define vlogi(...)
    #define vlogw(...)
    #define vloge(...)

    #define vlogd8(...)

    #define flogd(...) 
    #define flogi(...) 
    #define flogw(...) 
    #define floge(...) 
#endif

//
//
//
//

// 快捷使用定义
typedef Tlog_level<bhenum::level4> logel4;
typedef bhenum::level4 el4;

static Tlog_level<bhenum::level4> *_sp_level4_ = new Tlog_level<bhenum::level4>;
static Tlog_level<bhenum::level8> *_sp_level8_ = new Tlog_level<bhenum::level8>;
static Tlog_end *_sp_end_ = new Tlog_end;

static Tlog_null4 *_sp_null4_ = new Tlog_null4;
static Tlog_cmd4 *_sp_cmd4_ = new Tlog_cmd4;
static Tlog_cmd8 *_sp_cmd8_ = new Tlog_cmd8;
static Tlog_file4 *_sp_file4_ = new Tlog_file4;
static Tlog_asyn_file4 *_sp_afile4_ = new Tlog_asyn_file4;


#define $(value) "["#value": "<<value<<"] "
#define $C(value) "["#value": "<<bhtools::Tlog_con(value)<<"] "
#define $S(value) "["#value": "<<Tsstream_string()(value)<<"] "
#define $Q(value) "["#value": "<<value.toStdString()<<"] "


} // bhtools


#endif // TLOG_H
