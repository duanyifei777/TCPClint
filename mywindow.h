#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>
#include "io.h"
#include "manualcontrol.h"
#include "pointinfo.h"
#include "ramdata.h"
#include "globalcoordinate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class myWindow; }
QT_END_NAMESPACE

enum class CoordQueryFrom  // 标志位 记录当前的查询坐标指令MB_COORD来自哪里
{
    None,
    Main,
    TabCoord
};

class myWindow : public QWidget
{
    Q_OBJECT

public:
    explicit myWindow(QWidget *parent = nullptr);
    ~myWindow();

    CoordQueryFrom coordQueryForm = CoordQueryFrom::None;

private slots:
    void on_connectButton_clicked();      // 连接按钮槽函数
    void on_closeButton_clicked();        // 关闭按钮槽函数

    void readyRead_slot();                // socket收到数据时的槽

    void sendCommandToServer(const QString &cmd);  // 处理IO窗口发出的命令信号

    void changeSpeed();                   // 改变机器人速率（可能由按钮或UI控制）
    void updateSpeed(const QString &val);  // 更新速率显示

    void on_enabledButton_clicked();      // 使能按钮点击事件
    void updateEnabled(int val);          // 更新使能状态显示
    void updateAlarm(const QStringList &vallist); // 更新报警信息显示
    void updateCart(const QStringList &vallist);  // 更新机器人笛卡尔坐标
    void updateJoint(const QStringList &vallist); // 更新机器人关节坐标

    void changeTab(int index);
    void myTimerUpdate();  // 定时更新主界面其余信息
    void coordTimerUpdate();  // 定时更新机器人坐标
    void processCoordQueue();  // 处理坐标指令队列
    void processMainQueue();  // 处理其余信息指令队列

    void handleARWork(int val);  // 处理设置机器人运行状态返回的指令
    void on_startARButton_clicked();  // 启动AR程序
    void on_stopARButton_clicked();  // 停止AR程序
    void on_pauseARButton_clicked();  // 暂停AR程序
    void on_restartARButton_clicked();  // 复位机器人(只能清除报警)

    void updateARState(int val);  // 更新机器人运行状态
    void updateMode(int val);  // 更新当前系统模式

    void handleCoord(int val);  // 更新当前坐标系序号/处理修改坐标系返回的指令

    void on_changeCoordButton_clicked();  // 修改用户坐标系

    void on_changeSpeedButton_clicked();

    void on_changeCoordButton_2_clicked();

private:
    Ui::myWindow *ui;
    QTcpSocket *tcpsocket;
    IO *iowindow = nullptr;

    QTimer *mainTimer;  // 定时器用于轮询或定时刷新其余指令
    QTimer *coordTimer;  // 用来更新机器人当前坐标的定时器
    QQueue<QString> coordQueue;  // 坐标指令队列
    QQueue<QString> mainQueue;  // 其余指令队列
    QTimer *coordScheduler;  // 调度器
    QTimer *mainScheduler;  // 调度器

    manualControl *manualwindow = nullptr;
    pointInfo *pointwindow = nullptr;
    RAMData *ramwindow = nullptr;
    globalCoordinate *coordwindow = nullptr;

    bool isAlarm = false;  // 是否报警

    int mainindex = -1;
    int ioindex = -1;
    int pointindex = -1;
    int ramindex = -1;
    int coordindex = -1;

    QString ARWorkStatus = "";  // 判断ARWork状态的变量
    bool ARPause = false;  // 标记是否暂停

    bool ChangeCoord = false;  // 判断是否是修改坐标系/获取坐标系
};

#endif // MYWINDOW_H
