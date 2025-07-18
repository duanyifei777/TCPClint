#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include "io.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_connectButton_clicked();  // 连接按钮
    void on_closeButton_clicked();  // 关闭连接按钮
    void on_sendButton_clicked();  // 发送消息按钮

    void readready_slot(); // 接收数据确认

    void on_IOButton_clicked();  // IO面板按钮
    void sendCommandToServer(const QString &cmd);  // 处理IO面板发出的信号

    void on_clearsendButton_clicked();
    void on_clearrecButton_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *tcpsocket;
    IO *iowindow = nullptr;
};
#endif // WIDGET_H
