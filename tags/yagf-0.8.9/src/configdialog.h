#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();
    void setSelectedEngine(int value);
    int selectedEngine();
    void setTessDataPath(const QString &value); //"" - find yourself
    QString tessdataPath();
private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
