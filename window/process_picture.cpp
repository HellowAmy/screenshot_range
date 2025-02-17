
#include "process_picture.h"

process_picture::process_picture(QWidget *parent) : QWidget(parent)
{
    this->hide();
    this->setAttribute(Qt::WA_TranslucentBackground);
    connect(new QShortcut(Qt::Key_Escape, this), &QShortcut::activated, this, &process_picture::choose_rect_end);
}

process_picture::~process_picture()
{
}

void process_picture::choose_rect_start()
{
    _start_choose = true;
    this->setWindowState(Qt::WindowFullScreen);
    this->show();
}

void process_picture::choose_rect_end()
{
    _start_choose = false;
    this->setWindowState(Qt::WindowNoState);
    this->hide();
    emit sig_choose_finish();
}

QRect process_picture::get_rect_choose()
{
    return _choose;
}

void process_picture::mousePressEvent(QMouseEvent *event)
{
    choose_rect_start();
    _pos_press = event->pos();
    _pos_move = _pos_press;
    this->update();
    QWidget::mousePressEvent(event);
}

void process_picture::mouseReleaseEvent(QMouseEvent *event)
{
    choose_rect_end();
    this->update();
    QWidget::mouseReleaseEvent(event);
}

void process_picture::mouseMoveEvent(QMouseEvent *event)
{
    _pos_move = event->pos();
    this->update();
    QWidget::mouseMoveEvent(event);
}

void process_picture::paintEvent(QPaintEvent *event)
{
    QPainter show(this);
    show.setRenderHint(QPainter::Antialiasing);
    show.fillRect(rect(), Qt::transparent);

    if (_start_choose)
    {
        show.setPen(QPen(QColor("#B22222"), 2));
        _choose = QRect(_pos_press, _pos_move);
        show.setOpacity(0.5);
        show.drawRect(_choose);
    }

    show.end();
}