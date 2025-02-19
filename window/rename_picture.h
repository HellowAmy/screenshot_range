
#ifndef RENAME_PICTURE_H
#define RENAME_PICTURE_H

#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>

#include <set>

class rename_picture : public QDialog
{
    Q_OBJECT
public:
    rename_picture(QWidget *parent = nullptr);
    ~rename_picture();

    void set_picture_path(const QString &path);
    void set_picture_list(const QStringList &ls);
    void set_rename_list(const QStringList &ls);
    void reset_items(QListWidget *wid,const QStringList &ls);
    void load_data();
    void display_index(int row);
    void display_item(QListWidgetItem *item);
    void rename_picture_list();
    void load_display(const QString &path);
    void quick_rename();
    void number_rename();
    void set_tips(const QString &txt);

    QStringList get_rename_list();
    
signals:

protected:

private:
    QString _path;
    QStringList _ls_picture_name;

    QListWidget *_ls_picture = nullptr;
    QListWidget *_ls_rename = nullptr;
    QLabel *_lab_display = nullptr;
    QLabel *_lab_tips = nullptr;
    QPushButton *_btn_rename = nullptr;
    QPushButton *_btn_auto = nullptr;
    QPushButton *_btn_number = nullptr;

};
#endif // RENAME_PICTURE_H
