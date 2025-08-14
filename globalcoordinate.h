#ifndef GLOBALCOORDINATE_H
#define GLOBALCOORDINATE_H

#include <QWidget>

namespace Ui {
class globalCoordinate;
}

enum class CoordState{
    None,
    GetCurrent,
    GetByIndex,
    SetByIndex
};

enum class CoordType{
    None,
    User,
    Tool
};

class globalCoordinate : public QWidget
{
    Q_OBJECT

public:
    explicit globalCoordinate(QWidget *parent = nullptr);
    ~globalCoordinate();

    QStringList currentCoord;  // 用来存放获取的当前坐标系的数据

private:
    Ui::globalCoordinate *ui;

    int userIndex = -1;
    int toolIndex = -1;

    CoordState currentState = CoordState::None;  // 用来区分是什么操作：获取指定/获取当前/设置指定
    CoordType coordType = CoordType::None;  // 用来区分是哪个坐标系：用户/工具

signals:
    void sendCommendToServer(const QString &cmd);

private slots:
    void on_userIndexButton_clicked();  // 获取指定用户坐标系数据
    void on_userCurrentButton_clicked();  // 获取当前用户坐标系数据

    void showCurrentUser(int userindex, const QStringList &vallist);  // 将获取到的当前用户坐标系数据 和当前用户坐标系的编号 写入table中

    void on_toolIndexButton_clicked();  // 获取指定工具坐标系数据
    void on_toolCurrentButton_clicked();  // 获取当前工具坐标系数据
    void showCurrentTool(int val, const QStringList &vallist);  // 将获取到的当前工具坐标系数据 和当前工具坐标系编号写入table

    void on_setUserButton_clicked();

    void on_setToolButton_clicked();

public slots:
    void handleCoordUser(const QStringList &vallist);  // 处理用户坐标系数据返回的指令(获取指定/获取当前/设置指定)
    void handleCoordTool(const QStringList &vallist);  // 处理工具坐标系数据返回的指令(获取指定/获取当前/设置指定)

    void handleCurrentIndex(int val);  // 处理获取当前坐标系数据(用户/工具)返回的指令
};

#endif // GLOBALCOORDINATE_H
