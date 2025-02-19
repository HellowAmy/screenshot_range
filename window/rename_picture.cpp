
#include "rename_picture.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

rename_picture::rename_picture(QWidget *parent) : QDialog(parent)
{

    _lab_display = new QLabel(this);
    _lab_display->setFixedSize(960, 540);
    _lab_display->setFrameShape(QFrame::Box);

    _ls_picture = new QListWidget(this);
    _ls_picture->setDragDropMode(QAbstractItemView::InternalMove);

    _ls_rename = new QListWidget(this);
    _ls_rename->setDragDropMode(QAbstractItemView::InternalMove);

    _lab_tips = new QLabel(this);
    _lab_tips->setMinimumHeight(30);
    _lab_tips->setWordWrap(true);
    _lab_tips->setAlignment(Qt::AlignCenter);
    _lab_tips->setFrameShape(QFrame::Box);

    _btn_auto = new QPushButton(this);
    _btn_auto->setText("自动排序");

    _btn_number = new QPushButton(this);
    _btn_number->setText("序号重排");

    _btn_rename = new QPushButton(this);
    _btn_rename->setText("保存重命");

    //
    QHBoxLayout *lay_main = new QHBoxLayout(this);
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->addWidget(_lab_display);
        lay_main->addLayout(lay);
    }
    {
        QVBoxLayout *layv = new QVBoxLayout;
        {
            QHBoxLayout *lay = new QHBoxLayout;
            lay->addWidget(_ls_picture);
            lay->addWidget(_ls_rename);
            layv->addLayout(lay);
        }
        {
            QVBoxLayout *lay = new QVBoxLayout;
            lay->addWidget(_lab_tips);
            lay->addWidget(_btn_number);
            lay->addWidget(_btn_auto);
            lay->addWidget(_btn_rename);
            layv->addLayout(lay);
        }
        lay_main->addLayout(layv);
    }

    connect(_btn_rename, &QPushButton::clicked, this, &rename_picture::rename_picture_list);
    connect(_btn_auto, &QPushButton::clicked, this, &rename_picture::quick_rename);
    connect(_btn_number, &QPushButton::clicked, this, &rename_picture::number_rename);
    connect(_ls_picture, &QListWidget::itemClicked, this, &rename_picture::display_item);
}

rename_picture::~rename_picture()
{
}

void rename_picture::set_picture_path(const QString &path)
{
    _path = path;
}

void rename_picture::set_picture_list(const QStringList &ls)
{
    _ls_picture_name = ls;
    reset_items(_ls_picture, ls);
}

void rename_picture::set_rename_list(const QStringList &ls)
{
    reset_items(_ls_rename, ls);
}

void rename_picture::reset_items(QListWidget *wid, const QStringList &ls)
{
    wid->clear();
    for (int i = 0; i < ls.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        item->setText(ls[i]);
        wid->addItem(item);
    }
}

void rename_picture::load_data()
{
    display_index(0);
}

void rename_picture::display_index(int row)
{
    if (row < _ls_picture_name.size())
    {
        load_display(_ls_picture_name[row]);
    }
}

void rename_picture::rename_picture_list()
{
    auto ls = get_rename_list();

    bool is_ok = true;
    QStringList ls_repeat_name;
    std::set<QString> st;
    for (auto &a : ls)
    {
        auto ret = st.emplace(a);
        if (ret.second == false)
        {
            ls_repeat_name.push_back(*ret.first);
            is_ok = false;
        }
    }

    if (is_ok)
    {
        QDialog::accept();
    }
    else
    {
        QString str("重复名称: ");
        for (auto &a : ls_repeat_name)
        {
            str += a + " ";
        }
        set_tips(str);
    }
}

void rename_picture::load_display(const QString &path)
{
    _lab_display->setPixmap(_path + path);
}

void rename_picture::quick_rename()
{
    int s1 = _ls_picture->count();
    int s2 = _ls_rename->count();
    int min = qMin(s1, s2);

    for (int i = 0; i < min; i++)
    {
        QString s1 = _ls_picture->item(i)->text().section(".", 1, 1);
        QString s2 = _ls_rename->item(i)->text();

        QString name("%1.%2");
        name = name.arg(s2).arg(s1);
        _ls_picture->item(i)->setText(name);
    }
}

void rename_picture::number_rename()
{
    for (int i = 0; i < _ls_picture->count(); i++)
    {
        QString s1 = _ls_picture->item(i)->text().section(".", 1, 1);
        QString name("%1.%2");
        name = name.arg(i + 1).arg(s1);
        _ls_picture->item(i)->setText(name);
    }
}

void rename_picture::set_tips(const QString &txt)
{
    _lab_tips->setText(txt);
}

QStringList rename_picture::get_rename_list()
{
    QStringList ls;
    for (int i = 0; i < _ls_picture->count(); i++)
    {
        auto txt = _ls_picture->item(i)->text();
        ls.push_back(txt);
    }
    return ls;
}

void rename_picture::display_item(QListWidgetItem *item)
{
    int row = _ls_picture->row(item);
    display_index(row);
}
