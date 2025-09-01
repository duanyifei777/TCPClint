#include "pointinfo.h"
#include "ui_pointinfo.h"
#include "paraminputdialog.h"

#include <QMessageBox>
#include <QDebug>

pointInfo::pointInfo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pointInfo)
{
    ui->setupUi(this);

    checkMotionTimer = new QTimer(this);
    connect(checkMotionTimer, &QTimer::timeout, this, [=](){
        emit sendCommandToServer("MB_WAITPOS");
    });
    checkManualTimer = new QTimer(this);
    connect(checkManualTimer, &QTimer::timeout, this, [=](){
        emit sendCommandToServer("MB_WAITREALPOS");
    });
}

pointInfo::~pointInfo()
{
    delete ui;
}

void pointInfo::on_getPointButton_clicked()
{
    ui->pointInfoTable->clearContents();  // 清空表格内容
    ui->pointInfoTable->setRowCount(0);  // 初始化表格的行数，重置当前行
    currentrow = 0;

    QString input = ui->pointlineEdit->text().trimmed();  // 获取框中输入的编号
    if(input.contains("-"))  // 说明是区间（1-10）
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)
        {
            bool ok1, ok2;
            int startpoint = parts[0].trimmed().toInt(&ok1);  // 起始点位号
            int endpoint = parts[1].trimmed().toInt(&ok2);  // 结束点位号
            if(ok1 && ok2 && startpoint >=0 && endpoint <=999 && startpoint <= endpoint)
            {
                basePointNumber = startpoint;  // 保存当前起始点位号

                for(int i=startpoint; i<=endpoint; i++)
                {
                    QString cmd = QString("MB_GLOBALPOINT:%1").arg(i);  // 循环获取单个点位信息的指令，直到获取所有点位信息
                    QTimer::singleShot(50 * (i-startpoint), this, [=](){
                       emit sendCommandToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的数字区间：0~999");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式(如1-10)");
        }
    }
    else  // 说明是单个点位（1,2...）
    {
        bool ok;
        int pointNumber = input.toInt(&ok);  // 共享点位序号

        basePointNumber = pointNumber;

        if(ok && pointNumber>=0 && pointNumber<=999)
        {
            QString cmd = QString("MB_GLOBALPOINT:%1").arg(pointNumber);
            emit sendCommandToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确点位号");
        }
    }
}

void pointInfo::handelPointInfo(const QStringList &vallist)  // 处理获取/写入共享点位信息的返回指令，判断是查询还是写入结果
{
    if(vallist.size() == 1)
    {
        if(vallist[0] == "0")
        {
            return;  // 写入正常
        }
        else if(vallist[0] == "1")
        {
            QMessageBox::warning(this, "提示", "失败");  // 获取失败或写入失败
            return;
        }
    }
    else
    {
        if(isRecordingPoint)  // 将当前坐标写入当前行
        {
            int selectrow = ui->pointInfoTable->currentRow();
            QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
            int pointNumber = item->text().trimmed().toInt();
            updateTableRow(pointNumber, vallist);

            isRecordingPoint = false;
        }
        else
        {
            int poinNumber = basePointNumber + currentrow;
            // qDebug() << "basepointnumber=" << basePointNumber << " " << "currentrow=" << currentrow;
            updateTableRow(poinNumber, vallist);

            currentrow++;  // 下一次写入表格的下一行
        }
    }
}

void pointInfo::on_writePointButton_clicked()
{
    int selectrow = ui->pointInfoTable->currentRow();  // 获取选中的行
    QTableWidgetItem *pointItem = ui->pointInfoTable->item(selectrow, 0);  // 获取点位号（第0列）
    if(!pointItem)
    {
        QMessageBox::warning(this, "提示", "点位号为空");
        return;
    }

    QString pointText = pointItem->text().trimmed();
    bool ok;
    int pointNumber = pointText.toInt(&ok);
    if(!ok || pointNumber < 0 || pointNumber >999)
    {
        QMessageBox::warning(this, "提示", "点位号无效");
        return;
    }

    QStringList datalist;
    int col = ui->pointInfoTable->columnCount();  // 获取表格列数
    for(int i=1 ; i<col; i++)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, i);
        if(item)
        {
            datalist << item->text();
        }
        else{
            datalist << "";
        }
    }

    QString cmd = QString("MB_GLOBALPOINT:%1,%2").arg(pointNumber).arg(datalist.join(","));
    emit sendCommandToServer(cmd);
}


void pointInfo::on_recordCartButton_clicked()
{
    int selectrow = ui->pointInfoTable->currentRow();
    if(selectrow >= 0)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
        QString pointNumber = item->text().trimmed();
        QString cmd = QString("MB_TEACH:%1").arg(pointNumber);
        emit sendCommandToServer(cmd);

        isRecordingPoint = true;
    }
    else{
        QMessageBox::warning(this, "提示", "请选中一行点数据");
    }
}

void pointInfo::handelTechPoint(const QString &val)
{
    if(val == "0")  // 机器人坐标写入正常
    {
        int selectrow = ui->pointInfoTable->currentRow();
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
        QString pointNumber = item->text().trimmed();

        QString cmd = QString("MB_GLOBALPOINT:%1").arg(pointNumber);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "发送数据失败");
    }
}

void pointInfo::updateTableRow(int pointNumber, const QStringList &vallist)
{
    int row = -1;

    int rowcount = ui->pointInfoTable->rowCount();
    for(int i=0; i<rowcount; i++)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(i, 0);  // 遍历table每行的点位号
        if(item && item->text() == QString::number(pointNumber))  // 如果要写入的点位号已在表中，则标记该行
        {
            row = i;
            break;
        }
    }

    if(row == -1)  // 不在表中，就新增一行用来显示机器人当前坐标的点位信息
    {
        row = ui->pointInfoTable->rowCount();
        ui->pointInfoTable->insertRow(row);  // 增加一行

        QTableWidgetItem *iditem = new QTableWidgetItem(QString::number(pointNumber));
        iditem->setFlags(iditem->flags() & ~Qt::ItemIsEnabled);
        ui->pointInfoTable->setItem(row, 0, iditem);
    }

    for(int i=0; i<vallist.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
        ui->pointInfoTable->setItem(row, i+1, item);
    }
}

void pointInfo::on_clearTableButton_clicked()
{
    ui->pointInfoTable->clearContents();
    ui->pointInfoTable->setRowCount(0);
}


void pointInfo::on_trackButton_clicked()
{
    if(hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "请先释放运动权限");
        return;
    }
    int selectrow = ui->pointInfoTable->currentRow();
    if(selectrow >= 0)
    {
        int type = (ui->trackcomboBox->currentText() == "PTP跟踪")? 0 : 1;  // 获取当前跟踪类型 PTP/直线
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
        int pointNumber = item->text().trimmed().toInt();

        if(type == 0)  // PTP跟踪 0~999
        {
            QString cmd = QString("MB_TRACKMOTION:%1").arg(pointNumber);
            emit sendCommandToServer(cmd);
        }
        else  // 直线跟踪 1000~1999
        {
            QString cmd = QString("MB_TRACKMOTION:%1").arg(pointNumber+1000);
            emit sendCommandToServer(cmd);
        }
    }
    else{
        QMessageBox::warning(this, "提示", "请选中一行点数据");
    }
}

void pointInfo::handleTrackPoint(int val)
{
    if(val == 0)
    {
        startManualTimer();
        setMotionButtonEnabled(false);
        return;
    }
    else{
        QMessageBox::warning(this, "提示", "共享点位手动跟踪失败");
    }
}

void pointInfo::on_motionACButton_clicked()
{
    QString cmd = QString("MB_MACCEPT");
    emit sendCommandToServer(cmd);
}

void pointInfo::on_motionREButton_clicked()
{
    QString cmd = QString("MB_MRELEASE");
    emit sendCommandToServer(cmd);
}

void pointInfo::handleMotionAccept(int val)
{
    if(val == 0)
    {
        hasMotionAuthority = true;
        ui->motionACButton->setEnabled(false);
        ui->motionREButton->setEnabled(true);
        ui->motionREButton->setFocus();
    }
    else{
        QMessageBox::warning(this, "提示", "获取运动权限失败");
    }
}

void pointInfo::handleMotionRelease(int val)
{
    if(val == 0)
    {
        hasMotionAuthority = false;
        ui->motionREButton->setEnabled(false);
        ui->motionACButton->setEnabled(true);
        ui->motionACButton->setFocus();
    }
    else{
        QMessageBox::warning(this, "提示", "释放运动权限失败");
    }
}

void pointInfo::on_stopButton_clicked()
{
    QString cmd = QString("MB_AUTOSTOP");
    emit sendCommandToServer(cmd);

    stopMotionTimer();
    stopManualTimer();
    setMotionButtonEnabled(true);
}

void pointInfo::on_movePButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }
    int selectrow = ui->pointInfoTable->currentRow();
    int PType = 0;
    if(selectrow >= 0)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow,0);
        QString A = item->text().trimmed();
        QString cmd = QString("MB_MOVP:%1, , %2,%3,%4,%5,%6,%7")
                          .arg(A)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(PType);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请选中一行点数据");
    }
}

void pointInfo::handleMoveP(int val)
{
    switch (val) {
    case 0:  // 说明返回正常
        startMotionTimer();
        setMotionButtonEnabled(false);
        break;
    case 1:
        QMessageBox::warning(this, "提示", "速度比超出范围");
        break;
    case 2:
        QMessageBox::warning(this, "提示", "加速度比超出范围");
        break;
    case 3:
        QMessageBox::warning(this, "提示", "减速度比超出范围");
        break;
    case 4:
        QMessageBox::warning(this, "提示", "加加速度比超出范围");
        break;
    case 5:
        QMessageBox::warning(this, "提示", "CP值超出范围");
        break;
    case 7:
        QMessageBox::warning(this, "提示", "运动缓存已满");
        break;
    case 8:
        QMessageBox::warning(this, "提示", "设置速度失败");
        break;
    case 10:
        QMessageBox::warning(this, "提示", "发送运动指令失败");
        break;
    case 12:
        QMessageBox::warning(this, "提示", "单轴运动轴号错误");
        break;
    case 13:
        QMessageBox::warning(this, "提示", "单轴运动失败");
        break;
    case 14:
        QMessageBox::warning(this, "提示", "四轴联动运动失败");
        break;
    default:
        break;
    }
}

void pointInfo::on_movPRButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }
    int PType = 1;
    QStringList paramNames = {"A", "B"};
    QMap<QString, QString> paramHints;
    paramHints["A"] = "轴号:1~6";
    paramHints["B"] = "移动的相对距离";

    // 创建对话框 传入参数
    ParamInputDialog dialog(paramNames, paramHints, this);

    if(dialog.exec() == QDialog::Accepted)  // 输入完参数点击确认
    {
        QMap<QString, QString> params = dialog.getParamValues();

        QString A = params["A"];
        QString B = params["B"];

        QString cmd = QString("MB_MOVP:%1,%2,%3,%4,%5,%6,%7,%8")
                          .arg(A)
                          .arg(B)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(PType);
        emit sendCommandToServer(cmd);
    }
}

void pointInfo::on_marchPButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }

    int selectrow = ui->pointInfoTable->currentRow();
    if(selectrow >= 0)
    {
        int ZsAcc = 20;
        int ZsDec = 20;
        int ZsSpd = 20;
        int ZeAcc = 20;
        int ZeDec = 20;
        int ZeSpd = 20;

        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
        QString A = item->text().trimmed();  // 获取点位号

        QStringList ArchTypeInput = {"ArchType"};  // 先确定运动类型
        QMap<QString, QString> typeHint;
        typeHint["ArchType"] = "运动类型(0:拱形;1:门形)";

        ParamInputDialog typedialog(ArchTypeInput, typeHint, this);  // 第一个窗口获取运动类型
        if(typedialog.exec() == QDialog::Accepted)
        {
            QMap<QString, QString> type = typedialog.getParamValues();
            if(type["ArchType"] == "0")  // 表示拱形运动
            {
                int ArchType =0;
                QStringList paramNames = {"B", "C", "D"};
                QMap<QString, QString> paramHints;
                paramHints["B"] = "Z轴上抬限位高度";
                paramHints["C"] = "Z轴相对起始点上升高度";
                paramHints["D"] = "Z轴相对终点下降高度";

                ParamInputDialog dialog(paramNames, paramHints, this);  // 第二个窗口获取剩余参数
                if(dialog.exec() == QDialog::Accepted)
                {
                    QMap<QString, QString> params = dialog.getParamValues();
                    QString B = params["B"];
                    QString C = params["C"];
                    QString D = params["D"];

                    QString cmd = QString("MB_MARCHP:%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16")
                                      .arg(A)
                                      .arg(B)
                                      .arg(C)
                                      .arg(D)
                                      .arg(CP)
                                      .arg(Acc)
                                      .arg(Dec)
                                      .arg(Spd)
                                      .arg(Jerk)
                                      .arg(ZsAcc)
                                      .arg(ZsDec)
                                      .arg(ZsSpd)
                                      .arg(ZeAcc)
                                      .arg(ZeDec)
                                      .arg(ZeSpd)
                                      .arg(ArchType);
                    emit sendCommandToServer(cmd);
                }
            }
            else if(type["ArchType"] == "1")  // 表示门形运动
            {
                int ArchType = 1;
                QStringList paramNames = {"B"};
                QMap<QString, QString> paramHints;
                paramHints["B"] = "Z轴上抬限位高度";

                ParamInputDialog dialog(paramNames, paramHints, this);
                if(dialog.exec() == QDialog::Accepted)
                {
                    QMap<QString, QString> params = dialog.getParamValues();
                    QString B = params["B"];

                    QString cmd = QString("MB_MARCHP:%1,%2,,,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14")
                                      .arg(A)
                                      .arg(B)
                                      .arg(CP)
                                      .arg(Acc)
                                      .arg(Dec)
                                      .arg(Spd)
                                      .arg(Jerk)
                                      .arg(ZsAcc)
                                      .arg(ZsDec)
                                      .arg(ZsSpd)
                                      .arg(ZeAcc)
                                      .arg(ZeDec)
                                      .arg(ZeSpd)
                                      .arg(ArchType);
                    emit sendCommandToServer(cmd);
                }
            }
        }
    }
    else{
        QMessageBox::warning(this, "提示", "请选中一行点数据");
    }
}

void pointInfo::handleMArchP(int val)
{
    switch (val) {
    case 0:  // 说明返回正常
        startMotionTimer();
        setMotionButtonEnabled(false);
        break;
    case 1:
        QMessageBox::warning(this, "提示", "速度比超出范围");
        break;
    case 2:
        QMessageBox::warning(this, "提示", "加速度比超出范围");
        break;
    case 3:
        QMessageBox::warning(this, "提示", "减速度比超出范围");
        break;
    case 4:
        QMessageBox::warning(this, "提示", "加加速度比超出范围");
        break;
    case 5:
        QMessageBox::warning(this, "提示", "CP值超出范围");
        break;
    case 6:
        QMessageBox::warning(this, "提示", "目标点 Z 轴位置超过 B 值上限");
        break;
    case 8:
        QMessageBox::warning(this, "提示", "设置速度失败");
        break;
    case 9:
        QMessageBox::warning(this, "提示", "拱形运动失败");
        break;
    case 10:
        QMessageBox::warning(this, "提示", "发送运动指令失败");
        break;
    case 11:
        QMessageBox::warning(this, "提示", "拱形指令类型错误");
        break;
    default:
        break;
    }
}

void pointInfo::on_movJButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }

    QStringList PTypeInput = {"PType"};
    QMap<QString, QString> typeHints;
    typeHints["PType"] = "运动类型(0:四轴;1:单轴)";

    ParamInputDialog typedialog(PTypeInput, typeHints, this);  // 第一个窗口获取运动类型
    if(typedialog.exec() == QDialog::Accepted)
    {
        QMap<QString, QString> type = typedialog.getParamValues();
        if(type["PType"] == "0")  // 表示四轴联动
        {
            int PType = 0;
            QStringList paramNames = {"Joint[0]","Joint[1]","Joint[2]","Joint[3]","Joint[4]","Joint[5]",};
            QMap<QString, QString> paramHints;
            for(int i=0; i<6; i++)
            {
                paramHints[QString("Joint[%1]").arg(i)] = QString("J%1轴关节坐标").arg(i+1);
            }

            ParamInputDialog dialog(paramNames, paramHints, this);
            if(dialog.exec() == QDialog::Accepted)
            {
                QMap<QString, QString> params = dialog.getParamValues();

                QStringList Joint;
                for(int i=0; i<6; i++)
                {
                    Joint << params[QString("Joint[%1]").arg(i)];
                }

                QString cmd = QString("MB_MOVJ:%1,%2,%3,%4,%5,%6,,,%7,%8,%9,%10,%11,%12")
                                  .arg(Joint[0])
                                  .arg(Joint[1])
                                  .arg(Joint[2])
                                  .arg(Joint[3])
                                  .arg(Joint[4])
                                  .arg(Joint[5])
                                  .arg(CP)
                                  .arg(Acc)
                                  .arg(Dec)
                                  .arg(Spd)
                                  .arg(Jerk)
                                  .arg(PType);
                emit sendCommandToServer(cmd);
            }
        }
        else if(type["PType"] == "1")  // 表示单轴运动
        {
            int PType = 1;
            QStringList paramNames = {"Axis", "Rel"};
            QMap<QString, QString> paramHints;
            paramHints["Axis"] = "轴号:1~6";
            paramHints["Rel"] = "单轴运动绝对距离";

            ParamInputDialog dialog(paramNames, paramHints, this);
            if(dialog.exec() == QDialog::Accepted)
            {
                QMap<QString,QString> params = dialog.getParamValues();
                QString Axis = params["Axis"];
                QString Rel = params["Rel"];

                QString cmd = QString("MB_MOVJ:,,,,,,%1,%2,%3,%4,%5,%6,%7,%8")
                                  .arg(Axis)
                                  .arg(Rel)
                                  .arg(CP)
                                  .arg(Acc)
                                  .arg(Dec)
                                  .arg(Spd)
                                  .arg(Jerk)
                                  .arg(PType);
                emit sendCommandToServer(cmd);
            }
        }
    }
}

void pointInfo::handleMovJ(int val)
{
    switch (val) {
    case 0:  // 说明返回正常
        startMotionTimer();
        setMotionButtonEnabled(false);
        break;
    case 1:
        QMessageBox::warning(this, "提示", "速度比超出范围");
        break;
    case 2:
        QMessageBox::warning(this, "提示", "加速度比超出范围");
        break;
    case 3:
        QMessageBox::warning(this, "提示", "减速度比超出范围");
        break;
    case 4:
        QMessageBox::warning(this, "提示", "加加速度比超出范围");
        break;
    case 5:
        QMessageBox::warning(this, "提示", "CP值超出范围");
        break;
    case 7:
        QMessageBox::warning(this, "提示", "运动缓存已满");
        break;
    case 8:
        QMessageBox::warning(this, "提示", "设置速度失败");
        break;
    case 10:
        QMessageBox::warning(this, "提示", "发送运动指令失败");
        break;
    case 12:
        QMessageBox::warning(this, "提示", "单轴运动轴号错误");
        break;
    case 21:
        QMessageBox::warning(this, "提示", "J1轴运动目标位置超过限位");
        break;
    case 22:
        QMessageBox::warning(this, "提示", "J2单轴运动目标位置超过限位");
        break;
    case 23:
        QMessageBox::warning(this, "提示", "J3单轴运动目标位置超过限位");
        break;
    case 24:
        QMessageBox::warning(this, "提示", "J4单轴运动目标位置超过限位");
        break;
    case 25:
        QMessageBox::warning(this, "提示", "单轴运动失败");
        break;
    case 26:
        QMessageBox::warning(this, "提示", "四轴联动运动失败");
        break;
    case 27:
        QMessageBox::warning(this, "提示", "单轴运动目标位置超过限位");
        break;
    default:
        break;
    }
}

void pointInfo::on_moveLButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }

    int selectrow = ui->pointInfoTable->currentRow();
    if(selectrow >= 0)
    {
        int PType = 0;
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, 0);
        QString pointNumber = item->text().trimmed();

        int AccC = 20;
        int DecC = 20;
        int SpdC = 20;
        int JerkC = 20;

        QString cmd = QString("MB_MOVL:%1,,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")
                          .arg(pointNumber)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(AccC)
                          .arg(DecC)
                          .arg(SpdC)
                          .arg(JerkC)
                          .arg(PType);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请选中一行点数据");
    }
}

void pointInfo::on_moveLRButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }

    int PType = 1;
    int AccC = 20;
    int DecC = 20;
    int SpdC = 20;
    int JerkC = 20;
    QStringList paramNames = {"A","B"};
    QMap<QString, QString> paramHints;
    paramHints["A"] = "轴号:1~6";
    paramHints["B"] = "轴移动的距离";

    ParamInputDialog dialog(paramNames, paramHints, this);
    if(dialog.exec() == QDialog::Accepted)
    {
        QMap<QString, QString> params = dialog.getParamValues();
        QString A = params["A"];
        QString B = params["B"];

        QString cmd = QString("MB_MOVL:%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12")
                          .arg(A)
                          .arg(B)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(AccC)
                          .arg(DecC)
                          .arg(SpdC)
                          .arg(JerkC)
                          .arg(PType);
        emit sendCommandToServer(cmd);
    }
}

void pointInfo::handleMovL(int val)
{
    switch (val) {
    case 0:  // 说明返回正常
        startMotionTimer();
        setMotionButtonEnabled(false);
        break;
    case 1:
        QMessageBox::warning(this, "提示", "速度比超出范围");
        break;
    case 2:
        QMessageBox::warning(this, "提示", "加速度比超出范围");
        break;
    case 3:
        QMessageBox::warning(this, "提示", "减速度比超出范围");
        break;
    case 4:
        QMessageBox::warning(this, "提示", "加加速度比超出范围");
        break;
    case 5:
        QMessageBox::warning(this, "提示", "CP值超出范围");
        break;
    case 7:
        QMessageBox::warning(this, "提示", "运动缓存已满");
        break;
    case 8:
        QMessageBox::warning(this, "提示", "设置速度失败");
        break;
    case 10:
        QMessageBox::warning(this, "提示", "发送运动指令失败");
        break;
    case 12:
        QMessageBox::warning(this, "提示", "单轴运动轴号错误");
        break;
    case 15:
        QMessageBox::warning(this, "提示", "姿态速度比列超出范围");
        break;
    case 16:
        QMessageBox::warning(this, "提示", "姿态加速度比列超出范围");
        break;
    case 17:
        QMessageBox::warning(this, "提示", "姿态减速度比列超出范围");
        break;
    case 18:
        QMessageBox::warning(this, "提示", "姿态加加速度比列超出范围");
        break;
    case 19:
        QMessageBox::warning(this, "提示", "单轴运动失败");
        break;
    case 20:
        QMessageBox::warning(this, "提示", "四轴联动运动失败");
        break;
    default:
        break;
    }
}

void pointInfo::on_marcButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }
    QModelIndexList rowindex = ui->pointInfoTable->selectionModel()->selectedRows();

    if(rowindex.size() == 2)
    {
        int row1 = rowindex[0].row();
        int row2 = rowindex[1].row();

        QTableWidgetItem *item1 = ui->pointInfoTable->item(row1, 0);
        QTableWidgetItem *item2 = ui->pointInfoTable->item(row2, 0);

        QString A = item1->text().trimmed();
        QString B = item2->text().trimmed();

        int isCircle = 0;  // 圆弧运动
        int AccC = 20;
        int DecC = 20;
        int SpdC = 20;
        int JerkC = 20;

        QString cmd = QString("MB_MARC:%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12")
                          .arg(A)
                          .arg(B)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(AccC)
                          .arg(DecC)
                          .arg(SpdC)
                          .arg(JerkC)
                          .arg(isCircle);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请选中两个点位(目标点和中间点)");
    }

}

void pointInfo::on_mcircleButton_clicked()
{
    if(!hasMotionAuthority)
    {
        QMessageBox::warning(this, "提示", "未获取运动权限");
        return;
    }
    QModelIndexList rowindex = ui->pointInfoTable->selectionModel()->selectedRows();
    if(rowindex.size() == 2)
    {
        int row1 = rowindex[0].row();
        int row2 = rowindex[1].row();

        QTableWidgetItem *item1 = ui->pointInfoTable->item(row1, 0);
        QTableWidgetItem *item2 = ui->pointInfoTable->item(row2, 0);

        QString A = item1->text().trimmed();
        QString B = item2->text().trimmed();
        int isCircle = 1;  // 圆运动
        int AccC = 20;
        int DecC = 20;
        int SpdC = 20;
        int JerkC = 20;

        QString cmd = QString("MB_MARC:%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12")
                          .arg(A)
                          .arg(B)
                          .arg(CP)
                          .arg(Acc)
                          .arg(Dec)
                          .arg(Spd)
                          .arg(Jerk)
                          .arg(AccC)
                          .arg(DecC)
                          .arg(SpdC)
                          .arg(JerkC)
                          .arg(isCircle);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请选中两个点位(目标点和中间点)");
    }
}

void pointInfo::handleMarc(int val)
{
    switch (val) {
    case 0:  // 说明返回正常
        startMotionTimer();
        setMotionButtonEnabled(false);
        break;
    case 1:
        QMessageBox::warning(this, "提示", "速度比超出范围");
        break;
    case 2:
        QMessageBox::warning(this, "提示", "加速度比超出范围");
        break;
    case 3:
        QMessageBox::warning(this, "提示", "减速度比超出范围");
        break;
    case 4:
        QMessageBox::warning(this, "提示", "加加速度比超出范围");
        break;
    case 5:
        QMessageBox::warning(this, "提示", "CP值超出范围");
        break;
    case 7:
        QMessageBox::warning(this, "提示", "运动缓存已满");
        break;
    case 8:
        QMessageBox::warning(this, "提示", "设置速度失败");
        break;
    case 10:
        QMessageBox::warning(this, "提示", "发送运动指令失败");
        break;
    case 15:
        QMessageBox::warning(this, "提示", "姿态速度比列超出范围");
        break;
    case 16:
        QMessageBox::warning(this, "提示", "姿态加速度比列超出范围");
        break;
    case 17:
        QMessageBox::warning(this, "提示", "姿态减速度比列超出范围");
        break;
    case 18:
        QMessageBox::warning(this, "提示", "姿态加加速度比列超出范围");
        break;
    case 29:
        QMessageBox::warning(this, "提示", "获取当前笛卡尔位置失败");
        break;
    case 30:
        QMessageBox::warning(this, "提示", "获取当前位置手系失败");
        break;
    case 33:
        QMessageBox::warning(this, "提示", "运动失败");
        break;
    default:
        break;
    }
}

void pointInfo::startMotionTimer()
{
    if(!checkMotionTimer->isActive())
    {
        checkMotionTimer->start(500);
    }
}

void pointInfo::stopMotionTimer()
{
    if(checkMotionTimer->isActive())
    {
        checkMotionTimer->stop();
    }
}


void pointInfo::handleWaitPos(int val)
{
    qDebug() << "waitPos:" << val;
    if(val == 0)  // 说明运动状态结束了，就关闭定时器
    {
        stopMotionTimer();
        setMotionButtonEnabled(true);
    }
    else{
        return;
    }
}

void pointInfo::startManualTimer()
{
    if(!checkManualTimer->isActive())
    {
        checkManualTimer->start(500);
    }
}

void pointInfo::stopManualTimer()
{
    if(checkManualTimer->isActive())
    {
        checkManualTimer->stop();
    }
}

void pointInfo::handleWaitRealPos(int val)
{
    if(val == 0)  // 说明运动状态结束了，就关闭定时器
    {
        stopManualTimer();
        setMotionButtonEnabled(true);
    }
    else{
        return;
    }
}

void pointInfo::setMotionButtonEnabled(bool enabled)
{
    ui->trackButton->setEnabled(enabled);
    ui->movePButton->setEnabled(enabled);
    ui->movPRButton->setEnabled(enabled);
    ui->marchPButton->setEnabled(enabled);
    ui->movJButton->setEnabled(enabled);
    ui->moveLButton->setEnabled(enabled);
    ui->moveLRButton->setEnabled(enabled);
    ui->marcButton->setEnabled(enabled);
    ui->mcircleButton->setEnabled(enabled);
}
