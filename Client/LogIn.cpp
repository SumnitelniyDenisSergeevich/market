#include "LogIn.h"
#include "ui_LogIn.h"

LogIn::LogIn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogIn)
{
    ui->setupUi(this);

    connect(ui->sign_in_Button, &QPushButton::clicked, this, [this]
    {
        QString login = ui->login_lineEdit->text();
        QString password = ui->password_lineEdit->text();

        if (login.isEmpty() || password.isEmpty())
            ui->error_label->setText("it is necessary to fill in the fields");
        else
            emit logButtonClicked(login, password);
    });

    connect(ui->register_Button, &QPushButton::clicked, this, [this]
    {
        QString login = ui->login_lineEdit->text();
        QString password = ui->password_lineEdit->text();

        if (login.isEmpty() || password.isEmpty())
            ui->error_label->setText("it is necessary to fill in the fields");
        else
            emit RegButtonClicked(login, password);
    });

}

LogIn::~LogIn()
{
    delete ui;
}

void LogIn::setErrorMessage(const QString& m)
{
    ui->error_label->setText(m);
}
