#include "RequestDialog.h"
#include "ui_RequestDialog.h"

RequestDialog::RequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestDialog)
{
    ui->setupUi(this);

    ui->TypeBox->addItems({"buy", "sale"});//{0,1}

    connect(ui->AddRequestButton, &QPushButton::clicked, this, [this]{
        ui->errLabel->setText("");

        int usdCount = ui->USDCountBox->value();
        double usdPrice = ui->UsdPriceBox->value();
        bool forSale = ui->TypeBox->currentIndex() == 1;

        if (usdCount > 0 && usdPrice > 0.)
            emit addRequestButtonClicked(usdCount, usdPrice, forSale);
        else
            ui->errLabel->setText("Specify the number of dollars and the price per dollar");
    });
}

RequestDialog::~RequestDialog()
{
    delete ui;
}
