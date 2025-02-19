
#include "main_window.h"

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QShortcut>
#include <QVBoxLayout>

#include <fstream>

#include "Ffile.h"
#include "Tlog.h"
#include "change_config.h"
#include "qxtglobalshortcut.h"
#include "rename_picture.h"

main_window::main_window(QWidget *parent) : QWidget(parent)
{

    this->setFixedWidth(250);
    this->setWindowIcon(QIcon("../config/pic/logo.png"));

    _wid_display = new process_picture;
    _frame = new choose_range;

    _lab_tips = new QLabel(this);
    _lab_tips->setAlignment(Qt::AlignCenter);
    _lab_tips->setFrameShape(QFrame::Box);
    _lab_tips->setFixedHeight(30);

    QPushButton *btn_pix_rect = new QPushButton(this);
    btn_pix_rect->setText("重选范围");

    QPushButton *btn_screenshot = new QPushButton(this);
    btn_screenshot->setText("获取截图");

    QPushButton *btn_close_frame = new QPushButton(this);
    btn_close_frame->setText("范围显示");

    QPushButton *btn_rename = new QPushButton(this);
    btn_rename->setText("图片命名");

    QPushButton *btn_reset_number = new QPushButton(this);
    btn_reset_number->setText("重置序号");

    QPushButton *btn_config = new QPushButton(this);
    btn_config->setText("配置文件");

    QVBoxLayout *lay_main = new QVBoxLayout(this);
    lay_main->addWidget(_lab_tips);
    lay_main->addWidget(btn_pix_rect);
    lay_main->addWidget(btn_screenshot);
    lay_main->addWidget(btn_close_frame);
    lay_main->addWidget(btn_rename);
    lay_main->addWidget(btn_reset_number);
    lay_main->addWidget(btn_config);

    parse_data::read_json(_d, _file_json);
    _frame->set_frame(QRect(QPoint(_d.pos.toq()), QSize(_d.size.toq())));

    QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
    if (shortcut->setShortcut(QKeySequence("F10")))
    {
        qDebug() << "QKeySequence" << shortcut->shortcut().toString();
        connect(shortcut, &QxtGlobalShortcut::activated, this, &main_window::screenshot_save);
    }

    connect(_wid_display, &process_picture::sig_choose_finish, this, &main_window::save_choose_pic);
    connect(btn_pix_rect, &QPushButton::clicked, this, &main_window::screenshot_choose);
    connect(btn_screenshot, &QPushButton::clicked, this, &main_window::screenshot_save);
    connect(btn_close_frame, &QPushButton::clicked, this, &main_window::close_frame_display);
    connect(btn_rename, &QPushButton::clicked, this, &main_window::start_rename_wid);
    connect(btn_reset_number, &QPushButton::clicked, this, &main_window::reset_number);
    connect(btn_config, &QPushButton::clicked, this, &main_window::change_config_data);
}

main_window::~main_window()
{
}

QPixmap main_window::get_screenshot()
{
    auto screen = QApplication::primaryScreen();
    auto pix = screen->grabWindow(0);
    return pix;
}

QPixmap main_window::get_screenshot_rect(QRect rect)
{
    auto pix = get_screenshot();
    return pix.copy(rect.adjusted(1, 1, -1, -1));
}

void main_window::screenshot_choose()
{
    _frame->hide();
    _wid_display->choose_rect_start();
    set_tips("Reset screenshot range");
}

void main_window::screenshot(config_data d)
{
    bhtools::Ffsys::make_dir(d.path);
    QString path = QString::fromStdString(d.path);
    QDir dir(path);
    if (dir.exists() == false)
    {
        dir.mkpath(path);
    }

    QString format_name = QString::fromStdString(d.format_name);
    path += format_name.arg(d.order);
    auto pix = get_screenshot_rect(QRect(d.pos.toq(), d.size.toq()));
    pix.save(path, d.format_picture.c_str());
}

void main_window::screenshot_save()
{
    screenshot(_d);
    set_tips(QString("Save Picture %1").arg(_d.order));
    update_order();
    update_frame_col();
    parse_data::write_json(_d, _file_json);
}

void main_window::save_choose_pic()
{
    auto rect = _wid_display->get_rect_choose();
    _d.pos.fromq(rect.topLeft());
    _d.size.fromq(rect.size());
    _frame->set_frame(rect);
    _frame->show();
}

void main_window::update_order()
{
    _d.order += 1;
}

void main_window::close_frame_display()
{
    if (_frame->isHidden())
    {
        _frame->show();
        set_tips("Show frame");
    }
    else
    {
        _frame->hide();
        set_tips("Hide frame");
    }
}

void main_window::set_tips(const QString &txt)
{
    _lab_tips->setText(txt);
}

void main_window::start_rename_wid()
{
    QString path = QString::fromStdString(_d.path);
    QStringList old_name = get_picture_ls();
    rename_picture *wid_rename = new rename_picture(this);
    wid_rename->set_picture_path(path);
    wid_rename->set_picture_list(old_name);
    wid_rename->set_rename_list(get_rename_ls());
    wid_rename->load_data();
    wid_rename->show();

    connect(wid_rename, &rename_picture::accepted, this, [=]() {
        auto new_name = wid_rename->get_rename_list();
        rename_picture_path(path, old_name, new_name);
        qDebug() << "new name list: " << new_name;
    });
}

void main_window::reset_number()
{
    _d.order = 1;
    parse_data::write_json(_d, _file_json);
    set_tips(QString("Now order is: %1").arg(_d.order));
}

void main_window::change_config_data()
{
    change_config *wid_config = new change_config(this);
    wid_config->set_data(_d);
    wid_config->show();

    connect(wid_config, &rename_picture::accepted, this, [=]() {
        _d = wid_config->get_data();
        parse_data::write_json(_d, _file_json);
        set_tips(QString("change config"));
        qDebug() << "change config";
    });
}

void main_window::update_frame_col()
{
    if (_d.order % 2 == 0)
    {
        _frame->update_col("#B22222");
    }
    else
    {
        _frame->update_col("#3CB371");
    }
}

QStringList main_window::get_picture_ls()
{
    QString path = QString::fromStdString(_d.path);
    QDir dir(path);
    auto ls_names = dir.entryList(QDir::NoDot | QDir::NoDotDot | QDir::Files);
    return ls_names;
}

QStringList main_window::get_rename_ls()
{
    QStringList ls;
    auto vec = parse_data::read_rename_list(_file_rename);
    for (auto &a : vec)
    {
        ls.push_back(QString::fromStdString(a));
    }
    return ls;
}

void main_window::rename_picture_path(const QString &path, const QStringList &old_name, const QStringList &new_name)
{
    if (old_name.size() != new_name.size())
    {
        return;
    }
    QDir dir(path);
    QString tm = ".tempswap";
    for (int i = 0; i < old_name.size(); i++)
    {
        QString oldn = old_name[i];
        QString tmn = oldn + tm;
        dir.rename(oldn, tmn);
    }
    for (int i = 0; i < old_name.size(); i++)
    {
        QString oldn = old_name[i];
        QString tmn = oldn + tm;
        dir.rename(tmn, new_name[i]);
    }
}

void main_window::screenshot_quick()
{
    config_data d;
    std::string file = "../config/config.json";
    parse_data::read_json(d, file);
    d.order += 1;
    screenshot(d);
    parse_data::write_json(d, file);
}

void main_window::closeEvent(QCloseEvent *event)
{
    _wid_display->deleteLater();
    _frame->deleteLater();
    QWidget::closeEvent(event);
}
