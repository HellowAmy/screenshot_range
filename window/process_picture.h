
#ifndef PROCESS_PICTURE_H
#define PROCESS_PICTURE_H

#include <QWidget>
#include <QShortcut>
#include <QMouseEvent>
#include <QPainter>

class process_picture : public QWidget
{
    Q_OBJECT
public:
    process_picture(QWidget *parent = nullptr);
    ~process_picture();

    void choose_rect_start();
    void choose_rect_end();
    QPixmap get_background();
    QPixmap get_background_choose();
    QRect get_rect_choose();

signals:
    void sig_choose_finish();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:    
    bool _start_choose = false;
    QPoint _pos_press;
    QPoint _pos_move;
    QRect _choose;
};
#endif // PROCESS_PICTURE_H
