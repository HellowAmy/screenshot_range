
#ifndef PARSE_DATA_H
#define PARSE_DATA_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "json.hpp"
#include "Ffile.h"

using namespace nlohmann;

struct size_ct
{
    int w;
    int h;
    void fromq(const QSize &d) { w = d.width(); h = d.height(); }
    QSize toq() { return QSize(w,h); }
};
struct pos_ct
{
    int x;
    int y;
    void fromq(const QPoint &d) { x = d.x(); y = d.y(); }
    QPoint toq() { return QPoint(x,y); }
};
struct config_data
{
    size_ct size;
    pos_ct pos;
    int order;
    std::string path;
    std::string format_picture;
    std::string format_name;
};

struct parse_data
{
    static bool read_json(config_data &d,std::string path)
    {
        try {
            std::ifstream ifs(path);
            json js = json::parse(ifs);
            d.pos.x = js["pos"]["x"];
            d.pos.y = js["pos"]["y"];
            d.size.w = js["size"]["w"]; 
            d.size.h = js["size"]["h"]; 
            d.order = js["order"]; 
            d.path = js["path"]; 
            d.format_picture = js["format_picture"]; 
            d.format_name = js["format_name"]; 
            return ifs.good();
        } catch(...){}
        return false;
    }

    static bool write_json(const config_data &d,std::string path)
    {
        try {
            json js;
            js["pos"]["x"] = d.pos.x;
            js["pos"]["y"] = d.pos.y;
            js["size"]["w"] = d.size.w;
            js["size"]["h"] = d.size.h;
            js["order"] = d.order;
            js["path"] = d.path;
            js["format_picture"] = d.format_picture;
            js["format_name"] = d.format_name;
            std::ofstream ofs(path);
            ofs << std::setw(4) <<js << std::endl;
            return ofs.good();
        } catch(...){}
        return false;
    }

    static std::vector<std::string> read_rename_list(std::string path)
    {
        std::vector<std::string> vec;
        std::fstream fs(path,std::ios::in);
        if(fs.is_open())
        {
            std::string buf;
            while(std::getline(fs, buf))
            {
                vec.push_back(buf);
            }
            fs.close();
        }
        return vec;
    }



};
#endif // PARSE_DATA_H
