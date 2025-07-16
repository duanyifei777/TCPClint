#ifndef IO_H
#define IO_H

#include <QWidget>
#include <QVector>

namespace Ui {
class IO;
}

class IO : public QWidget
{
    Q_OBJECT

public:
    explicit IO(QWidget *parent = nullptr);
    ~IO();

private:
    Ui::IO *ui;

private slots:
    void handleOutputButton();

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // IO_H
