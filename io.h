#ifndef IO_H
#define IO_H

#include <QWidget>
#include <QVector>

namespace Ui {
class IO;
}

class IO : public QWidget
{
    Q_OBJECT

public:
    explicit IO(QWidget *parent = nullptr);
    ~IO();

private:
    Ui::IO *ui;

private slots:
    void handleOutputButton();  //改变输出状态的按钮

    void on_updateInputButton_clicked();  //更新输入按钮
    void on_updateOutputButton_clicked();  //更新输出按钮

public slots:
    void updateInputState(const int &instate);  // 处理输入状态更新
    void updateOutputState(const int &outstate);  //处理输出状态更新

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // IO_H
