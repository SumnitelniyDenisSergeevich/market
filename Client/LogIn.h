#pragma once

#include <QDialog>

namespace Ui {
class LogIn;
}

class LogIn : public QDialog
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();

public:
    void setErrorMessage(const QString& m);

signals:
    void logButtonClicked(const QString& login, const QString& password);
    void RegButtonClicked(const QString& login, const QString& password);

private:
    Ui::LogIn *ui;
};
