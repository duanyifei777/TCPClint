#ifndef PARAMINPUTDIALOG_H
#define PARAMINPUTDIALOG_H

#include <QDialog>
#include <Qmap>

class QLineEdit;

class ParamInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParamInputDialog(const QStringList &paramNames,
                              const QMap<QString, QString> &paramHints = {},
                              QWidget *parent = nullptr);
    ~ParamInputDialog();

    QMap<QString, QString> getParamValues() const;  // 将所有的输入存入<参数名称，文本内容>的映射，方便后续调用

private:
    QMap<QString, QLineEdit*> inputFields;  // 输入参数 <参数名称， 指向对应文本框的指针> 用于读取每个文本框的内容
};

#endif // PARAMINPUTDIALOG_H
