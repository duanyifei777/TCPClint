#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include "io.h"

namespace Ui {
class myWindow;
}

class myWindow : public QWidget
{
    Q_OBJECT

public:
    explicit myWindow(QWidget *parent = nullptr);
    ~myWindow();

private slots:
    void on_connectButton_clicked();
    void on_closeButton_clicked();
    void readyRead_slot();

    void sendCommandToServer(const QString &cmd);  // 处理IO面板发出的信号

    void changeSpeed();  // 改变机器人速率
    void updateSpeed(QString val);  // 更新当前速率

    void on_enabledButton_clicked();  // 使能按钮
    void updateEnabled(int val);  // 更新使能状态
    void updateAlarm(QStringList vallist);  // 更新报警信息
    void updateCart(QStringList vallist);  // 更新机器人笛卡尔坐标
    void updateJoint(QStringList vallist);  // 更新机器人关节坐标

    void myTimerUpdate();

private:
    Ui::myWindow *ui;
    QTcpSocket *tcpsocket;
    IO *iowindow = nullptr;
    QTimer *mainTimer;
};

#endif // MYWINDOW_H
