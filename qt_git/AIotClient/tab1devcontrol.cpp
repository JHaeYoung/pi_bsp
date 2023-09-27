#include "tab1devcontrol.h"
#include "ui_tab1devcontrol.h"
#include <math.h>
#include <stdio.h>
Tab1DevControl::Tab1DevControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab1DevControl)
{
    ui->setupUi(this);
    pKeyLed = new KeyLed(this);
    connect(ui->pDialLed, SIGNAL(valueChanged(int)), pKeyLed,SLOT(writeLedData(int)));
    connect(ui->pDialLed,SIGNAL(valueChanged(int)), ui->pProgressBarLed, SLOT(setValue(int)));
    //connect(ui->pDialLed,SIGNAL(valueChanged(int)), this, SLOT(progressBarSetSlot(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), ui->pDialLed, SLOT(setValue(int)));
    connect(ui->pDialLed,SIGNAL(valueChanged(int)), ui->pLCDNumberLed, SLOT(display(int)));
    connect(pKeyLed,SIGNAL(updateKeydataSig(int)), this, SLOT(keyCheckBoxSlot(int)));



}

void Tab1DevControl::progressBarSetSlot(int ledVal)
{
    ledVal = int(ledVal /255.0 *100);
    ui->pProgressBarLed->setValue(ledVal);
}


void Tab1DevControl::keyCheckBoxSlot(int keyNo)
{
    static int lcdData;
    lcdData ^=(0x01 <<(keyNo-1));
    ui->pLcdNumberKey->display(lcdData);

    //printf("lcdData : %d",lcdData);

    QCheckBox *pQCheckBoxArray[8] = {ui->pCBkey1,ui->pCBkey2 ,ui->pCBkey3,ui->pCBkey4
                                    ,ui->pCBkey5,ui->pCBkey6,ui->pCBkey7,ui->pCBkey8};
    for(int i=0;i<8;i++)
    {
        if(keyNo-1 ==i){
            if(pQCheckBoxArray[i]->isChecked())
                pQCheckBoxArray[i]->setChecked(false);
            else
                pQCheckBoxArray[i]->setChecked(true);
        }
    }



}

Tab1DevControl::~Tab1DevControl()
{
    delete ui;
}
