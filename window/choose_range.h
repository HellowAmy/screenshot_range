
#ifndef CHOOSE_RANGE_H
#define CHOOSE_RANGE_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>

class choose_range : public QWidget
{
    Q_OBJECT
public:
    choose_range(QWidget *parent = nullptr);
    ~choose_range();

    void set_frame(const QRect &rect);

signals:

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QRect _rect;
};
#endif // CHOOSE_RANGE_H
