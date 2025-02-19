
#include "choose_range.h"

choose_range::choose_range(QWidget *parent) : QWidget(parent)
{
    this->hide();
    this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setWindowFlag(Qt::X11BypassWindowManagerHint);
}

choose_range::~choose_range()
{
}
void choose_range::set_frame(const QRect &rect)
{
    _rect = rect;
    this->move(rect.topLeft());
    this->resize(rect.size());
    this->update();
}

void choose_range::update_col(QColor col)
{
    _col = col;
    this->update();
}

void choose_range::paintEvent(QPaintEvent *event)
{
    QPainter show(this);
    show.fillRect(rect(), Qt::transparent);
    show.setPen(QPen(_col, 2));
    show.drawRect(rect());
    show.end();
}
