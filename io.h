#ifndef IO_H
#define IO_H

#include <QWidget>
#include <QVector>
#include <QTimer>

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
    QTimer *iotimer;

private slots:
    void handleOutputButton();  //改变输出状态的按钮

    void setButtonEnabled(QString &name, bool enabled);  // 设置按钮状态（是否可按下）

public slots:
    void updateInputState(const int &instate);  // 处理输入状态更新
    void updateOutputState(const int &outstate);  //处理输出状态更新

    void startTimer();  // 打开/关闭定时器
    void stopTimer();

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // IO_H
