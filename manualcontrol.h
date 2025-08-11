#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class manualControl;
}

enum class CartDirection
{
    None,
    XPlus, XMinus,
    YPlus, YMinus,
    ZPlus, ZMinus,
    APlus, AMinus,
    BPlus, BMinus,
    CPlus, CMinus
};

enum class JointDirection
{
    None,
    J1Plus, J1Minus,
    J2Plus, J2Minus,
    J3Plus, J3Minus,
    J4Plus, J4Minus,
    J5Plus, J5Minus,
    J6Plus, J6Minus,
};

class manualControl : public QWidget
{
    Q_OBJECT

public:
    explicit manualControl(QWidget *parent = nullptr);
    ~manualControl();

public slots:
    void handleManualMode(int val);

private slots:
    void manualCart();  // 处理整个笛卡尔运动的函数
    void startCartManual(CartDirection cdir);  // 按钮按下时的处理函数
    void stopCartManual();  // 按钮松开时的处理函数

    void manualJoint();  // 处理整个关节运动的函数
    void startJointManual(JointDirection jdir);  // 按钮按下时的处理函数
    void stopJointManual();  // 按钮松开时的处理函数

    void changeManualMode();

    void on_XppushButton_pressed();
    void on_XppushButton_released();

    void on_XmpushButton_pressed();
    void on_XmpushButton_released();

    void on_YppushButton_pressed();
    void on_YppushButton_released();

    void on_YmpushButton_pressed();
    void on_YmpushButton_released();

    void on_ZppushButton_pressed();
    void on_ZppushButton_released();

    void on_ZmpushButton_pressed();
    void on_ZmpushButton_released();

    void on_AppushButton_pressed();
    void on_AppushButton_released();

    void on_AmpushButton_pressed();
    void on_AmpushButton_released();

    void on_BppushButton_pressed();
    void on_BppushButton_released();

    void on_BmpushButton_pressed();
    void on_BmpushButton_released();

    void on_CppushButton_pressed();
    void on_CppushButton_released();

    void on_CmpushButton_pressed();
    void on_CmpushButton_released();

    void on_J1ppushButton_pressed();
    void on_J1ppushButton_released();

    void on_J1mpushButton_pressed();

    void on_J1mpushButton_released();

    void on_J2ppushButton_pressed();

    void on_J2ppushButton_released();

    void on_J2mpushButton_pressed();

    void on_J2mpushButton_released();

    void on_J3ppushButton_pressed();

    void on_J3mpushButton_released();

    void on_J4ppushButton_pressed();

    void on_J4ppushButton_released();

    void on_J4mpushButton_pressed();

    void on_J4mpushButton_released();

    void on_J5ppushButton_pressed();

    void on_J5ppushButton_released();

    void on_J5mpushButton_pressed();

    void on_J5mpushButton_released();

    void on_J6ppushButton_pressed();

    void on_J6ppushButton_released();

    void on_J6mpushButton_pressed();

    void on_J6mpushButton_released();

    void on_J3mpushButton_pressed();

    void on_J3ppushButton_released();

    void on_manualModeButton_clicked();

private:
    Ui::manualControl *ui;
    QTimer *manualtimer;

    CartDirection currentCarttDirection = CartDirection::None;
    JointDirection currentJointDirection = JointDirection::None;

    int manualmode = 3;  // 机器人手动模式 3:连续，0: 0.1，1: 1.0，2: 5.0

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // MANUALCONTROL_H
