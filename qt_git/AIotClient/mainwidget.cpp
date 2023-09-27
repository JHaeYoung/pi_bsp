#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    ui->pTabWidget ->setCurrentIndex(1); // 0:setting mainTap tap1
    pTab1DevControl = new Tab1DevControl(ui->pTab1); // dynamic hip allocation
    pTab2SocketClient = new Tab2SocketClient(ui->pTab2); // dynamic hip allocation
    ui->pTab1->setLayout(pTab1DevControl->layout()); //
    ui->pTab2->setLayout(pTab2SocketClient->layout());


}

MainWidget::~MainWidget()
{
    delete ui;
}

