#include <QApplication>
#include <QCommandLineParser>
#include <QWidget>

#include "Tlog.h"
#include "window/main_window.h"

using namespace std;

enum type_argv
{
    e_run,
    e_quick
};

int parse_argv(const std::vector<std::string> &vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        std::string s = vec[i];
        if (s == "quick")
        {
            return e_quick;
        }
    }
    return e_run;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::vector<std::string> vec;
    for (int i = 0; i < argc; i++)
    {
        vec.push_back(argv[i]);
    }

    int type = parse_argv(vec);

    if (type == e_quick)
    {
        main_window::screenshot_quick();
        return 0;
    }

    main_window w;
    w.show();
    return a.exec();
}