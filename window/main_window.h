
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QFrame>
#include <QLabel>

#include "json.hpp"
#include "process_picture.h"
#include "choose_range.h"
#include "parse_data.h"

class main_window : public QWidget
{
    Q_OBJECT
public:
    main_window(QWidget *parent = nullptr);
    ~main_window();

    static QPixmap get_screenshot();
    static QPixmap get_screenshot_rect(QRect rect);
    
    static void screenshot(config_data d);
    void screenshot_choose();
    void screenshot_save();
    void save_choose_pic();
    void update_order();
    void close_frame_display();
    void set_tips(const QString &txt);
    void start_rename_wid();
    void reset_number();
    void change_config_data();
    void update_frame_col();

    QStringList get_picture_ls();
    QStringList get_rename_ls();
    void rename_picture_path(const QString &path, const QStringList &old_name,const QStringList &new_name);

    static void screenshot_quick();

signals:

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:    
    config_data _d;
    std::string _file_json = "../config/config.json";
    std::string _file_rename = "../config/renamelist.txt";

    process_picture *_wid_display = nullptr;
    choose_range *_frame = nullptr;
    QLabel *_lab_tips = nullptr;
};
#endif // MAIN_WINDOW_H
