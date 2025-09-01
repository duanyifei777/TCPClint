#include "manualcontrol.h"
#include "ui_manualcontrol.h"

#include <QMessageBox>
#include <qDebug>

manualControl::manualControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::manualControl)
{
    ui->setupUi(this);

    // 创建定时器
    manualtimer = new QTimer(this);
    connect(manualtimer, &QTimer::timeout, this, &manualControl::changeManualMode);  // 连接定时器与笛卡尔运动

    // 初始化手动模式按钮
    ui->manualModeButton->setText("连续");
}

manualControl::~manualControl()
{
    delete ui;
}

void manualControl::on_manualModeButton_clicked()
{
    int mode = (manualmode+1) % 4;  // 3->0->1->2->3循环

    QString cmd = QString("MB_MANUALMODE:%1").arg(mode);
    emit sendCommandToServer(cmd);
}

void manualControl::handleManualMode(int val)
{
    if(val == 0)  // 切换成功
    {
        manualmode = (manualmode + 1) % 4;

        switch (manualmode) {
        case 0:
            ui->manualModeButton->setText("单步移动:0.1");
            break;
        case 1:
            ui->manualModeButton->setText("单步移动:1");
            break;
        case 2:
            ui->manualModeButton->setText("单步移动:5");
            break;
        case 3:
            ui->manualModeButton->setText("连续");
            break;
        }
    }
    else  // 切换失败
    {
        QMessageBox::warning(this, "提示", "切换模式失败");
    }
}

void manualControl::changeManualMode()
{
    if(currentCarttDirection != CartDirection::None)
    {
        manualCart();
    }
    else if(currentJointDirection != JointDirection::None){
        manualJoint();
    }
}

void manualControl::manualCart()  // 处理整个笛卡尔运动的函数
{
    int axis = 0;

    switch (currentCarttDirection)
    {
    case CartDirection::XPlus:
        axis = 1;
        break;
    case CartDirection::XMinus:
        axis = 2;
        break;
    case CartDirection::YPlus:
        axis = 3;
        break;
    case CartDirection::YMinus:
        axis = 4;
        break;
    case CartDirection::ZPlus:
        axis = 5;
        break;
    case CartDirection::ZMinus:
        axis = 6;
        break;
    case CartDirection::APlus:
        axis = 7;
        break;
    case CartDirection::AMinus:
        axis = 8;
        break;
    case CartDirection::BPlus:
        axis = 9;
        break;
    case CartDirection::BMinus:
        axis = 10;
        break;
    case CartDirection::CPlus:
        axis = 11;
        break;
    case CartDirection::CMinus:
        axis = 12;
        break;
    default:
        break;
    }

    QString cmd = QString("MB_CARTMOVE:%1").arg(axis);
    emit sendCommandToServer(cmd);
}

void manualControl::startCartManual(CartDirection cdir)  // 按钮按下时的处理函数
{
    currentCarttDirection = cdir;
    manualtimer->start(100);
}

void manualControl::stopCartManual()  // 按钮松开时的处理函数
{
    manualtimer->stop();
    currentCarttDirection = CartDirection::None;


    manualCart();  // 松开按钮后调用该函数，会立即执行停止命令
}

void manualControl::manualJoint()
{
    int joint = 0;
    switch (currentJointDirection) {
    case JointDirection::J1Plus:
        joint = 1;
        break;
    case JointDirection::J1Minus:
        joint = 2;
        break;
    case JointDirection::J2Plus:
        joint = 3;
        break;
    case JointDirection::J2Minus:
        joint = 4;
        break;
    case JointDirection::J3Plus:
        joint = 5;
        break;
    case JointDirection::J3Minus:
        joint = 6;
        break;
    case JointDirection::J4Plus:
        joint = 7;
        break;
    case JointDirection::J4Minus:
        joint = 8;
        break;
    case JointDirection::J5Plus:
        joint = 9;
        break;
    case JointDirection::J5Minus:
        joint = 10;
        break;
    case JointDirection::J6Plus:
        joint = 11;
        break;
    case JointDirection::J6Minus:
        joint = 12;
        break;
    default:
        break;
    }

    QString cmd = QString("MB_JOINTMOVE:%1").arg(joint);
    emit sendCommandToServer(cmd);
    // qDebug() << "发送指令:" << cmd;
}

void manualControl::startJointManual(JointDirection jdir)
{
    currentJointDirection = jdir;
    manualtimer->start(100);
}

void manualControl::stopJointManual()
{
    manualtimer->stop();
    currentJointDirection = JointDirection::None;

    manualJoint();  // 松开按钮后调用该函数，会立即执行停止命令
}

void manualControl::on_XppushButton_pressed()  // X+按下
{
    startCartManual(CartDirection::XPlus);
}

void manualControl::on_XppushButton_released()  // X+松开
{
    stopCartManual();
}

void manualControl::on_XmpushButton_pressed()  // X-按下
{
    startCartManual(CartDirection::XMinus);
}

void manualControl::on_XmpushButton_released()  // X-松开
{
    stopCartManual();
}

void manualControl::on_YppushButton_pressed()  // Y+按下
{
    startCartManual(CartDirection::YPlus);
}

void manualControl::on_YppushButton_released()  // Y+松开
{
    stopCartManual();
}

void manualControl::on_YmpushButton_pressed()  // Y-按下
{
    startCartManual(CartDirection::YMinus);
}

void manualControl::on_YmpushButton_released()  // Y-松开
{
    stopCartManual();
}

void manualControl::on_ZppushButton_pressed()  // Z+按下
{
    startCartManual(CartDirection::ZPlus);
}

void manualControl::on_ZppushButton_released()  // Z+松开
{
    stopCartManual();
}

void manualControl::on_ZmpushButton_pressed()  // Z-按下
{
    startCartManual(CartDirection::ZMinus);
}

void manualControl::on_ZmpushButton_released()  // Z-松开
{
    stopCartManual();
}

void manualControl::on_AppushButton_pressed()  // A+按下
{
    startCartManual(CartDirection::APlus);
}

void manualControl::on_AppushButton_released()  // A+松开
{
    stopCartManual();
}

void manualControl::on_AmpushButton_pressed()  // A-按下
{
    startCartManual(CartDirection::AMinus);
}

void manualControl::on_AmpushButton_released()  // A-松开
{
    stopCartManual();
}

void manualControl::on_BppushButton_pressed()  // B+按下
{
    startCartManual(CartDirection::BPlus);
}

void manualControl::on_BppushButton_released()  // B+松开
{
    stopCartManual();
}

void manualControl::on_BmpushButton_pressed()  // B-按下
{
    startCartManual(CartDirection::BMinus);
}

void manualControl::on_BmpushButton_released()  // B-松开
{
    stopCartManual();
}

void manualControl::on_CppushButton_pressed()  // C+按下
{
    startCartManual(CartDirection::CPlus);
}

void manualControl::on_CppushButton_released()  // C+松开
{
    stopCartManual();
}

void manualControl::on_CmpushButton_pressed()  // C-按下
{
    startCartManual(CartDirection::CMinus);
}

void manualControl::on_CmpushButton_released()  // C-松开
{
    stopCartManual();
}


void manualControl::on_J1ppushButton_pressed()  // J1+按下
{
    startJointManual(JointDirection::J1Plus);
}

void manualControl::on_J1ppushButton_released()  // J1+松开
{
    stopJointManual();
}

void manualControl::on_J1mpushButton_pressed()  // J1+按下
{
    startJointManual(JointDirection::J1Minus);
}

void manualControl::on_J1mpushButton_released()  // J1-松开
{
    stopJointManual();
}

void manualControl::on_J2ppushButton_pressed()  // J2+按下
{
    startJointManual(JointDirection::J2Plus);
}

void manualControl::on_J2ppushButton_released()  // J2+松开
{
    stopJointManual();
}

void manualControl::on_J2mpushButton_pressed()  // J2-按下
{
    startJointManual(JointDirection::J2Minus);
}

void manualControl::on_J2mpushButton_released()  // J2-松开
{
    stopJointManual();
}

void manualControl::on_J3ppushButton_pressed()  // J3+按下
{
    startJointManual(JointDirection::J3Plus);
}

void manualControl::on_J3ppushButton_released()  // J3+松开
{
    stopJointManual();
}

void manualControl::on_J3mpushButton_pressed()  // J3-按下
{
    startJointManual(JointDirection::J3Minus);
}

void manualControl::on_J3mpushButton_released()  // J3-松开
{
    stopJointManual();
}

void manualControl::on_J4ppushButton_pressed()  // J4+按下
{
    startJointManual(JointDirection::J4Plus);
}

void manualControl::on_J4ppushButton_released()  // J4+松开
{
    stopJointManual();
}

void manualControl::on_J4mpushButton_pressed()  // J4-按下
{
    startJointManual(JointDirection::J4Minus);
}

void manualControl::on_J4mpushButton_released()  // J4-松开
{
    stopJointManual();
}

void manualControl::on_J5ppushButton_pressed()  // J5+按下
{
    startJointManual(JointDirection::J5Plus);
}

void manualControl::on_J5ppushButton_released()  // J5+松开
{
    stopJointManual();
}

void manualControl::on_J5mpushButton_pressed()  // J5-按下
{
    startJointManual(JointDirection::J5Minus);
}

void manualControl::on_J5mpushButton_released()  // J5-松开
{
    stopJointManual();
}

void manualControl::on_J6ppushButton_pressed()  // J6+按下
{
    startJointManual(JointDirection::J6Plus);
}

void manualControl::on_J6ppushButton_released()  // J6+松开
{
    stopJointManual();
}

void manualControl::on_J6mpushButton_pressed()  // J6-按下
{
    startJointManual(JointDirection::J6Minus);
}

void manualControl::on_J6mpushButton_released()  // J6-松开
{
    stopJointManual();
}
