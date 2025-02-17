
#ifndef FFILE_H
#define FFILE_H

#include <fstream>
#include <functional>
#include <vector>
#include "Fstm.h"


namespace bhtools {


// 跨平台处理函数
#ifdef __linux__
    namespace bhtools_platform {
        #include <sys/stat.h>
        #include <dirent.h>
        #include <cstdio>
        #include <unistd.h>

        // 分隔符
        inline static std::string file_splitter() { return "/"; }

        // 换行符
        inline static std::string file_break() { return "\n"; }

        // 合并路径
        inline static std::string path_merge(const std::string &prefix,const std::string &file) 
        {
            if(prefix.size() > 0)
            {
                std::string sp = prefix.substr(prefix.size() - file_splitter().size());
                if(sp == file_splitter()) { return prefix + file; }
                else { return prefix + file_splitter() + file; }
            }
            return prefix + file; 
        }
        
        // 判断文件夹存在
        inline static bool is_exist_dir(const std::string &path)
        {
            DIR *dir = opendir(path.c_str());
            if(dir)
            {
                closedir(dir);
                return true;
            }
            return false;
        }

        // 判断为文件
        inline static bool is_file_type(const std::string &path)
        {
            struct stat st;
            lstat(path.c_str(),&st);
            if(st.st_mode & S_IFREG) { return true; }
            return false;
        }

        // 判断为目录
        inline static bool is_dir_type(const std::string &path)
        {
            struct stat st;
            lstat(path.c_str(),&st);
            if(st.st_mode & S_IFDIR) { return true; }
            return false;
        }

        // 删除文件
        inline static bool remove_file(const std::string &file)
        { return remove(file.c_str()) == 0; }
        
        // 移动文件
        inline static bool move_file(const std::string &src,const std::string &dst)
        { return rename(src.c_str(), dst.c_str()) == 0; }
        
        // 创建单层目录
        inline static bool create_dir(const std::string &dir,int mode)
        { return (!dir.empty() && !is_exist_dir(dir) && (mkdir(dir.c_str(), mode) != 0)) == false; }

        // 创建多层目录
        inline static bool make_dir(const std::string &dir,int mode)
        {
            size_t pos = 0;
            std::string level;
            while((pos = dir.find(file_splitter(), pos)) != std::string::npos) 
            {
                level = dir.substr(0, pos);
                if(create_dir(level,mode) == false)
                { return false; }
                pos++;
            }
            if(pos != dir.size())
            { level = dir.substr(0, dir.size()); }

            if(create_dir(level,mode) == false) { return false; }
            return true;
        }

        // 获取文件与目录列表
        inline static bool get_dir_info(const std::string &path,
                                        std::vector<std::string> &files,
                                        std::vector<std::string> &dirs,
                                        bool recursion)
        {
            DIR *dir = opendir(path.c_str());
            if(dir)
            {
                struct dirent *entry;
                std::vector<std::string> level;
                while((entry = readdir(dir)) != nullptr) 
                {
                    if(std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
                    {
                        std::string path_name = bhtools_platform::path_merge(path,std::string(entry->d_name));
                        if(is_dir_type(path_name))
                        { level.push_back(path_name); }
                        else { files.push_back(path_name); }
                    }
                }
                for(auto &a : level)
                {
                    dirs.push_back(a); 
                    if(recursion) 
                    { get_dir_info(a,files,dirs,recursion); }
                }
                closedir(dir);
                return true;
            }
            return false;
        }

        // 删除文件
        inline static bool rm_file(const std::string &path)
        { return remove(path.c_str()) == 0; }

        // 删除空目录
        inline static bool rm_dir(const std::string &path)
        { return rmdir(path.c_str()) == 0; }

        // 删除目录包括目录下所有内容
        inline static bool remove_dir(const std::string &dir)
        {
            std::vector<std::string> files;
            std::vector<std::string> dirs;
            if(bhtools_platform::get_dir_info(dir,files,dirs,true))
            {
                for(auto it = files.rbegin();it != files.rend();it++)
                {   
                    if(bhtools_platform::rm_file(*it) == false)
                    { return false; }
                }
                for(auto it = dirs.rbegin();it != dirs.rend();it++)
                {   
                    if(bhtools_platform::rm_dir(*it) == false)
                    { return false; }
                }
                if(bhtools_platform::rm_dir(dir) == false)
                { return false; }

                return true;
            }
            return false;
        }


    } // bhtools_platform
#elif
    namespace bhtools_platform {

        // //
        // static bool is_file_type(const std::string &file)
        // { return false; }

    } // bhtools_platform
#endif



// 文件处理类-提供跨平台处理文件与目录的功能
struct Ffsys
{
    // 判断存在-文件
    inline static bool is_exist_file(const std::string &path)
    { std::ifstream ifs(path); return ifs.good(); } 

    // 判断存在-文件夹
    inline static bool is_exist_dir(const std::string &path)
    { return bhtools_platform::is_exist_dir(path); }

    // 判断为文件
    inline static bool is_file_type(const std::string &path)
    { return bhtools_platform::is_file_type(path); } 

    // 判断为目录
    inline static bool is_dir_type(const std::string &path)
    { return bhtools_platform::is_dir_type(path); } 

    // 删除文件
    inline static bool remove_file(const std::string &file)
    { return bhtools_platform::remove_file(file); }

    // 移动文件
    inline static bool move_file(const std::string &src,const std::string &dst)
    { return bhtools_platform::move_file(src,dst); } 

    // 复制文件
    inline static bool copy_file(const std::string &src,const std::string &dst)
    {
        std::ifstream ifs(src,std::ios::binary);
        std::ofstream ofs(dst,std::ios::binary);
        if(ifs.is_open() && ofs.is_open())
        {
            ofs << ifs.rdbuf();
            ifs.close();
            ofs.close();
            return true;
        }
        return false;
    }

    // 创建单层目录
    inline static bool create_dir(const std::string &dir,int mode = 0755)
    { return bhtools_platform::create_dir(dir,mode); }

    // 创建多层目录
    inline static bool make_dir(const std::string &dir,int mode = 0755)
    { return bhtools_platform::make_dir(dir,mode); }

    // 删除目录包括目录下所有内容-目录不存在则认为删除成功
    inline static bool remove_dir(const std::string &dir)
    { 
        if(is_exist_dir(dir))
        { return bhtools_platform::remove_dir(dir); }
        else { return true; }
    }

    // 返回后缀
    inline static std::string get_suffix(const std::string &file)
    { return Fstm(file)(".",-1,-1); }

    // 返回全名称
    inline static std::string get_name(const std::string &file)
    { return Fstm(file)(bhtools_platform::file_splitter(),-1,-1); }

    // 返回无后缀名
    inline static std::string get_basename(const std::string &file)
    { return Fstm(file)(bhtools_platform::file_splitter(),-1,-1,".",0,0); }

    // 获取文件
    inline static std::vector<std::string> get_files(const std::string &path,bool recursion = true)
    { 
        std::vector<std::string> files;
        std::vector<std::string> dirs;
        if(bhtools_platform::get_dir_info(path,files,dirs,recursion))
        { return files; }
        return {};
    }

    // 获取目录
    inline static std::vector<std::string> get_dirs(const std::string &path,bool recursion = true)
    { 
        std::vector<std::string> files;
        std::vector<std::string> dirs;
        if(bhtools_platform::get_dir_info(path,files,dirs,recursion))
        { return dirs; }
        return {};
    }

    // 获取文件和目录
    inline static std::tuple<std::vector<std::string>,std::vector<std::string>>
        get_files_and_dirs(const std::string &path,bool recursion = true)
    { 
        std::vector<std::string> files;
        std::vector<std::string> dirs;
        if(bhtools_platform::get_dir_info(path,files,dirs,recursion))
        { return std::make_tuple(files,dirs); }
        return {};
    }
    
};

// 文件读写扩展类
struct Ffio
{
    Ffio(std::fstream &fs) 
    { _fs = &fs; } 

    // 读取所有字节
    std::string read_all() 
    {
        std::string buf;
        buf.resize(file_len_max());
        _fs->read((char *)buf.c_str(),file_len_max());
        return buf;
    }

    // 读取一行文本
    std::string read_line() 
    {
        std::string buf;
        std::getline(*_fs, buf);
        return buf;
    }   

    // 写入缓冲字节
    size_t write(const std::string &buf) 
    {
        size_t bnow = _fs->tellp();
        _fs->write(buf.c_str(),buf.size());

        size_t enow = _fs->tellp();
        size_t len = enow - bnow;
        if(buf.size() == len)
        { return len; }
        return 0;
    }

    // 写入一行字符-返回带换行符长度
    size_t write_line(const std::string &buf) 
    {
        size_t bnow = _fs->tellp();
        std::string sbreak = bhtools_platform::file_break();
        std::string str = buf + sbreak;
        _fs->write(str.c_str(),str.size());

        size_t enow = _fs->tellp();
        size_t len = enow - bnow;
        if((buf.size() + sbreak.size()) == len)
        { return len; }
        return 0;
    }

    // 获取文件最大长度
    size_t file_len_max()
    {
        size_t now = _fs->tellg();
        _fs->seekg(0,std::ios::end);

        size_t end = _fs->tellg();
        _fs->seekg(now,std::ios::beg);
        return end;
    }

    // 清除状态重新开始
    void reset_pos()
    {
        _fs->clear();
        _fs->seekg(0,std::ios::beg);
        _fs->seekp(0,std::ios::beg);
    }

    // 从当前位置跳过指定长度
    void skip_pos(size_t len)
    {
        size_t pnow = _fs->tellp();
        size_t gnow = _fs->tellg();
        _fs->seekp(pnow + len,std::ios::beg);
        _fs->seekg(gnow + len,std::ios::beg);
    }


    std::fstream *_fs = nullptr;    // 操作的文件指针
};



} // bhtools


#endif // FFILE_H
