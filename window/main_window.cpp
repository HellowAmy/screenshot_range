
#include "main_window.h"

#include <QApplication>
#include <QDebug>
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
#include "qxtglobalshortcut/qxtglobalshortcut.h"

main_window::main_window(QWidget *parent) : QWidget(parent)
{
    // this->resize(200, 400);
    this->setFixedWidth(250);

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

    QVBoxLayout *lay_main = new QVBoxLayout(this);
    lay_main->addWidget(_lab_tips);
    lay_main->addWidget(btn_pix_rect);
    lay_main->addWidget(btn_screenshot);
    lay_main->addWidget(btn_close_frame);

    parse_data::read_json(_d, _file);

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
    update_order();
    screenshot(_d);
    parse_data::write_json(_d, _file);

    set_tips(QString("Save Picture %1").arg(_d.order));
}

void main_window::save_choose_pic()
{
    auto rect = _wid_display->get_rect_choose();
    _d.pos.fromq(rect.topLeft());
    _d.size.fromq(rect.size());
    screenshot_save();
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
