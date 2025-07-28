#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>
#include "io.h"
#include "manualcontrol.h"
#include "pointinfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class myWindow; }
QT_END_NAMESPACE

class myWindow : public QWidget
{
    Q_OBJECT

public:
    explicit myWindow(QWidget *parent = nullptr);
    ~myWindow();

private slots:
    void on_connectButton_clicked();      // 连接按钮槽函数
    void on_closeButton_clicked();        // 关闭按钮槽函数

    void readyRead_slot();                // socket收到数据时的槽

    void sendCommandToServer(const QString &cmd);  // 处理IO窗口发出的命令信号

    void changeSpeed();                   // 改变机器人速率（可能由按钮或UI控制）
    void updateSpeed(QString val);        // 更新速率显示

    void on_enabledButton_clicked();      // 使能按钮点击事件
    void updateEnabled(int val);          // 更新使能状态显示
    void updateAlarm(QStringList vallist); // 更新报警信息显示
    void updateCart(QStringList vallist);  // 更新机器人笛卡尔坐标
    void updateJoint(QStringList vallist); // 更新机器人关节坐标

    void myTimerUpdate();                 // 定时器周期触发函数
    void changeTab(int index);

private:
    Ui::myWindow *ui;
    QTcpSocket *tcpsocket;
    IO *iowindow = nullptr;
    QTimer *mainTimer;                    // 定时器用于轮询或定时刷新
    manualControl *manualwindow = nullptr;
    pointInfo *pointwindow = nullptr;

    int mainindex = -1;
    int ioindex = -1;
    int pointindex = -1;
};

#endif // MYWINDOW_H
