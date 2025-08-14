#include "globalcoordinate.h"
#include "ui_globalcoordinate.h"
#include "mywindow.h"

#include <QMessageBox>
#include <QDebug>
#include <QTimer>

globalCoordinate::globalCoordinate(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::globalCoordinate)
{
    ui->setupUi(this);

    // 设置表头字体
    QFont font ("微软雅黑", 9);
    ui->coordUserTable->setFont(font);
    ui->coordToolTable->setFont(font);

}

globalCoordinate::~globalCoordinate()
{
    delete ui;
}

void globalCoordinate::on_userIndexButton_clicked()
{
    coordType = CoordType::User;
    currentState = CoordState::GetByIndex;
    userIndex = -1;
    QString input = ui->userIndexEdit->text().trimmed();
    if(input.contains("-"))  // 说明是区间
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)
        {
            bool ok1, ok2;
            int startindex = parts[0].toInt(&ok1);
            int endindex = parts[1].toInt(&ok2);
            userIndex = startindex;
            if(ok1 && ok2 && startindex>=0 && endindex <= 19 && startindex <= endindex)
            {
                for(int i=startindex; i<=endindex; i++)
                {
                    QString cmd = QString("MB_COORD_USER:%1").arg(i);
                    QTimer::singleShot(50 *(i-startindex), [=](){
                        emit sendCommendToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的数字区间");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式");
        }
    }
    else  // 说明是单个点
    {
        bool ok;
        int index = input.toInt(&ok);
        userIndex = index;
        if(ok && index >= 0 && index <= 19)
        {
            QString cmd = QString("MB_COORD_USER:%1").arg(index);
            emit sendCommendToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确用户号");
        }
    }
}

void globalCoordinate::handleCoordUser(const QStringList &vallist)
{
    if(currentState == CoordState::GetByIndex)  // 当前状态为获取指定用户坐标系数据
    {
        if(userIndex < 0) return;

        for(int i=0; i<vallist.size(); i++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
            ui->coordUserTable->setItem(userIndex, i, item);
        }
        userIndex++;
    }
    else if(currentState == CoordState::GetCurrent)  // 当前状态为获取当前用户坐标系数据
    {
        currentCoord = vallist;  // 先将当前用户坐标系的数据保存下来
        QString cmd = "MB_COORD";
        emit sendCommendToServer(cmd);
    }
    else if(currentState == CoordState::SetByIndex)  // 当前状态为设置指定用户坐标系数据
    {
        if(vallist[0] == "0") return;
        else{
            QMessageBox::warning(this, "提示", "设置用户坐标系数据失败");
        }
    }
}

void globalCoordinate::on_userCurrentButton_clicked()
{
    myWindow *mywindow = qobject_cast<myWindow*>(this->window());  // 获取主窗口指针
    mywindow->coordQueryForm = CoordQueryFrom::TabCoord;// 从<全局坐标系>页面发出的查询指令

    coordType = CoordType::User;
    currentState = CoordState::GetCurrent;
    QString cmd = "MB_COORD_USER";
    emit sendCommendToServer(cmd);
}

void globalCoordinate::handleCurrentIndex(int val)
{
    QString binarystate = QString::number(val, 2).rightJustified(16, '0');  // 转换为16位2进制字符串
    QString highstate = binarystate.left(8);  // 高8位
    QString lowstate = binarystate.right(8);  // 低8位

    int user = highstate.toInt(nullptr, 2);  // 用户坐标系编号
    int tool = lowstate.toInt(nullptr, 2);  // 工具坐标系编号

    if(coordType == CoordType::User)
    {
        showCurrentUser(user, currentCoord);
    }
    else if(coordType == CoordType::Tool)
    {
        showCurrentTool(tool, currentCoord);
    }
}

void globalCoordinate::showCurrentUser(int userindex, const QStringList &vallist)
{
    for(int i=0; i<vallist.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
        ui->coordUserTable->setItem(userindex, i, item);
    }

    // 选中整行
    ui->coordUserTable->selectRow(userindex);
    // 滚动到该行
    ui->coordUserTable->scrollToItem(ui->coordUserTable->item(userindex, 0), QAbstractItemView::PositionAtCenter);
}

void globalCoordinate::on_setUserButton_clicked()
{
    currentState = CoordState::SetByIndex;  //  当前为设置指定用户坐标系数据
    int selectrow = ui->coordUserTable->currentRow();  // 获取当前行
    QStringList datalist;
    int col = ui->coordUserTable->columnCount();  // 获取列数
    for(int i=0; i<col; i++)
    {
        QTableWidgetItem *item = ui->coordUserTable->item(selectrow, i);
        if(item)
        {
            datalist << item->text();
        }
        else{
            datalist << "";
        }
    }
    QString cmd = QString("MB_COORD_USER:%1,%2").arg(selectrow).arg(datalist.join(","));
    emit sendCommendToServer(cmd);
}

void globalCoordinate::on_toolIndexButton_clicked()
{
    coordType = CoordType::Tool;
    currentState = CoordState::GetByIndex;
    toolIndex = -1;
    QString input = ui->toolIndexEdit->text().trimmed();
    if(input.contains("-"))  // 说明是区间
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)
        {
            bool ok1, ok2;
            int startindex = parts[0].toInt(&ok1);
            int endindex = parts[1].toInt(&ok2);
            toolIndex = startindex;
            if(ok1 && ok2 && startindex>=0 && endindex <= 19 && startindex <= endindex)
            {
                for(int i=startindex; i<=endindex; i++)
                {
                    QString cmd = QString("MB_COORD_TOOL:%1").arg(i);
                    QTimer::singleShot(50 *(i-startindex), [=](){
                        emit sendCommendToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的数字区间");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式");
        }
    }
    else  // 说明是单个点
    {
        bool ok;
        int index = input.toInt(&ok);
        toolIndex = index;
        if(ok && index >= 0 && index <= 19)
        {
            QString cmd = QString("MB_COORD_TOOL:%1").arg(index);
            emit sendCommendToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确用户号");
        }
    }
}

void globalCoordinate::handleCoordTool(const QStringList &vallist)
{
    if(currentState == CoordState::GetByIndex)  // 当前状态为获取指定用户坐标系数据
    {
        if(toolIndex < 0) return;

        for(int i=0; i<vallist.size(); i++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
            ui->coordToolTable->setItem(toolIndex, i, item);
        }
        toolIndex++;
    }
    else if(currentState == CoordState::GetCurrent)  // 当前状态为获取当前用户坐标系数据
    {
        currentCoord = vallist;  // 先将当前工具坐标系的数据保存下来
        QString cmd = "MB_COORD";
        emit sendCommendToServer(cmd);
    }
    else if(currentState == CoordState::SetByIndex)  // 当前状态为设置指定用户坐标系数据
    {
        if(vallist[0] == "0") return;
        else{
            QMessageBox::warning(this, "提示", "设置工具坐标系数据失败");
        }
    }
}

void globalCoordinate::on_toolCurrentButton_clicked()
{
    myWindow *mywindow = qobject_cast<myWindow*>(this->window());
    mywindow->coordQueryForm = CoordQueryFrom::TabCoord;

    coordType = CoordType::Tool;
    currentState = CoordState::GetCurrent;
    QString cmd = "MB_COORD_TOOL";
    emit sendCommendToServer(cmd);
}

void globalCoordinate::showCurrentTool(int toolindex, const QStringList &vallist)
{
    for(int i=0; i<vallist.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
        ui->coordToolTable->setItem(toolindex, i, item);
    }

    ui->coordToolTable->selectRow(toolindex);
    ui->coordToolTable->scrollToItem(ui->coordToolTable->item(toolindex, 0), QAbstractItemView::PositionAtCenter);
}


void globalCoordinate::on_setToolButton_clicked()
{
    currentState = CoordState::SetByIndex;
    QStringList datalist;
    int selectrow = ui->coordToolTable->currentRow();
    int col = ui->coordToolTable->columnCount();
    for(int i=0; i<col; i++)
    {
        QTableWidgetItem *item = ui->coordToolTable->item(selectrow, i);
        if(item)
        {
            datalist << item->text();
        }
        else{
            datalist << "";
        }
    }
    QString cmd = QString("MB_COORD_TOOL:%1,%2").arg(selectrow).arg(datalist.join(","));
    emit sendCommendToServer(cmd);
}

