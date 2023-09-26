#include "widget.h"
#include "ui_widget.h"
#include <QDial>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    pKeyLed = new KeyLed(this);

    connect(ui->pDial,SIGNAL(valueChanged(int)), ui->pLedValLabel, SLOT(setNum(int)));
    connect(ui->pDial,&QDial::valueChanged, ui->pProgressBar, &QProgressBar::setValue);
    //connect(ui->pDial,&QDial::valueChanged,ui->pLabel, &QLabel::setNum);

    connect(ui->pDial,SIGNAL(valueChanged(int)), pKeyLed, SLOT(writeLedData(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), ui->pKeyValLabel, SLOT(setNum(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), ui->pDial, SLOT(setValue(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), ui->pLedValLabel, SLOT(setNum(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), ui->pProgressBar, SLOT(setValue(int)));
	

}

Widget::~Widget()
{
    delete ui;
}

