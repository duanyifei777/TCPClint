#ifndef POINTINFO_H
#define POINTINFO_H

#include <QWidget>
#include <QTimer>

class myWindow;

namespace Ui {
class pointInfo;
}

class pointInfo : public QWidget
{
    Q_OBJECT

public:
    explicit pointInfo(QWidget *parent = nullptr);
    ~pointInfo();

private slots:
    void on_getPointButton_clicked();  // 获取共享点位表中点位信息

    void on_writePointButton_clicked();  // 往共享点位表中写入单个点位信息

    void on_recordCartButton_clicked();  // 记录当前机器人笛卡尔坐标到共享点位表中

    void on_clearTableButton_clicked();  // 清除table数据

    void on_trackButton_clicked();  // 共享点位手动跟踪

    void on_motionACButton_clicked();  // 获取运动权限按钮
    void on_motionREButton_clicked();  // 释放运动权限按钮

    void on_stopButton_clicked();
    void on_movePButton_clicked();
    void on_marchPButton_clicked();

    void on_movPRButton_clicked();

    void on_movJButton_clicked();

    void on_moveLButton_clicked();

    void on_moveLRButton_clicked();

    void on_marcButton_clicked();

    void on_mcircleButton_clicked();

public slots:
    void handelPointInfo(const QStringList &vallist);  // 处理写入/获取共享点位的返回指令
    void handelTechPoint(const QString &val);  // 处理记录当前坐标到共享点位表的返回指令

    void updateTableRow(int pointNumber, const QStringList &vallist);  // 更新table的行

    void handleTrackPoint(int val);  // 处理跟踪点位的返回指令

    void handleMotionAccept(int val);  // 处理获取运动权限返回的指令
    void handleMotionRelease(int val);  // 处理释放运动权限返回的指令

    void handleMoveP(int val);  // 处理MovP返回的指令
    void handleMArchP(int val);  // 处理MArchP返回的指令
    void handleMovJ(int val);  // 处理MovJ返回的指令
    void handleMovL(int val);  // 处理MovL返回的指令
    void handleMarc(int val);  // 处理Marc返回的指令

    void startMotionTimer();  // 开启motion定时器
    void stopMotionTimer();  // 关闭motion定时器
    void startManualTimer();  // 开启manual定时器
    void stopManualTimer();  // 关闭manual定时器

    void handleWaitPos(int val);  // 处理等待运动状态结束返回的指令
    void handleWaitRealPos(int val);  // 处理等待手动运动状态结束返回的指令

    void setMotionButtonEnabled(bool enabled);  // 集中控制按钮状态的函数

private:
    Ui::pointInfo *ui;
    QTimer *checkMotionTimer;
    QTimer *checkManualTimer;

    int currentrow = 0;  // 表示当前处理的行号
    int basePointNumber = 0;  // 起始点位号

    bool isRecordingPoint = false;  // 是否为 记录当前坐标操作标志位

    bool hasMotionAuthority = false;  // 当前是否拥有运动权限标志位

    int CP = 20;  // 平滑系数
    int Acc = 20;  // 加速度百分比
    int Dec = 20;  // 减速度百分比
    int Spd = 20;  // 速度百分比
    int Jerk = 20;  // 加加速度百分比

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // POINTINFO_H
