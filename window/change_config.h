
#ifndef CHANGE_CONFIG_H
#define CHANGE_CONFIG_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "parse_data.h"

class change_config : public QDialog
{
    Q_OBJECT
public:
    change_config(QWidget *parent = nullptr);
    ~change_config();

    void set_data(const config_data &d);
    config_data get_data();

    void choose_save_path();
    void save_quit();

signals:

protected:

private:
    config_data _d;
    QLineEdit *_ed_number = nullptr;
    QLineEdit *_ed_path = nullptr;
    QLineEdit *_ed_format = nullptr;
    QLineEdit *_ed_name = nullptr;
};
#endif // CHANGE_CONFIG_H
