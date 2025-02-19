
#include "change_config.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "Ffile.h"

change_config::change_config(QWidget *parent) : QDialog(parent)
{
    this->setMinimumWidth(300);

    _ed_number = new QLineEdit(this);
    _ed_path = new QLineEdit(this);
    _ed_format = new QLineEdit(this);
    _ed_name = new QLineEdit(this);

    QPushButton *btn_browse = new QPushButton(this);
    btn_browse->setText("浏览");

    QPushButton *btn_save = new QPushButton(this);
    btn_save->setText("保存退出");

    //
    int stretch1 = 3;
    int stretch2 = 7;
    QVBoxLayout *lay_main = new QVBoxLayout(this);
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setAlignment(Qt::AlignLeft);
        lay->addWidget(new QLabel("文件名格式", this), stretch1);
        lay->addWidget(_ed_name, stretch2);
        lay_main->addLayout(lay);
    }
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setAlignment(Qt::AlignLeft);
        lay->addWidget(new QLabel("图片格式", this), stretch1);
        lay->addWidget(_ed_format, stretch2);
        lay_main->addLayout(lay);
    }
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setAlignment(Qt::AlignLeft);
        lay->addWidget(new QLabel("序号", this), stretch1);
        lay->addWidget(_ed_number, stretch2);
        lay_main->addLayout(lay);
    }
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setAlignment(Qt::AlignLeft);
        lay->addWidget(new QLabel("路径", this), stretch1);
        {
            QHBoxLayout *lay1 = new QHBoxLayout;
            lay1->addWidget(_ed_path);
            lay1->addWidget(btn_browse);
            lay->addLayout(lay1, stretch2);
        }
        lay_main->addLayout(lay);
    }
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->addWidget(btn_save);
        lay_main->addLayout(lay);
    }

    //
    connect(btn_browse, &QPushButton::clicked, this, &change_config::choose_save_path);
    connect(btn_save, &QPushButton::clicked, this, &change_config::save_quit);
}

change_config::~change_config()
{
}

void change_config::set_data(const config_data &d)
{
    _d = d;
    _ed_number->setText(QString::number(_d.order));
    _ed_path->setText(QString::fromStdString(_d.path));
    _ed_format->setText(QString::fromStdString(_d.format_picture));
    _ed_name->setText(QString::fromStdString(_d.format_name));
}

config_data change_config::get_data()
{
    _d.format_name = _ed_name->text().toStdString();
    _d.format_picture = _ed_format->text().toStdString();
    _d.order = _ed_number->text().toInt();
    _d.path = _ed_path->text().toStdString();
    return _d;
}

void change_config::choose_save_path()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QChar end = dir[dir.size() - 1];
    QChar enter = QChar(bhtools::bhtools_platform::file_splitter()[0]);
    if (end == enter)
    {
        _ed_path->setText(dir);
    }
    else
    {
        _ed_path->setText(dir + enter);
    }
}

void change_config::save_quit()
{
    QDialog::accept();
}
