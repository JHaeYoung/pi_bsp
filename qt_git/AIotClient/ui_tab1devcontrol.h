/********************************************************************************
** Form generated from reading UI file 'tab1devcontrol.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TAB1DEVCONTROL_H
#define UI_TAB1DEVCONTROL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tab1DevControl
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pPBtimerStart;
    QComboBox *pCBtimerValue;
    QPushButton *pPBappQuit;
    QHBoxLayout *horizontalLayout_2;
    QDial *pDialLed;
    QLCDNumber *pLCDNumberLed;
    QHBoxLayout *horizontalLayout_4;
    QProgressBar *pProgressBarLed;
    QHBoxLayout *horizontalLayout_3;
    QGridLayout *pGRkey;
    QCheckBox *pCBkey1;
    QCheckBox *pCBkey8;
    QCheckBox *pCBkey2;
    QCheckBox *pCBkey5;
    QCheckBox *pCBkey6;
    QCheckBox *pCBkey7;
    QCheckBox *pCBkey3;
    QCheckBox *pCBkey4;
    QLCDNumber *pLcdNumberKey;

    void setupUi(QWidget *Tab1DevControl)
    {
        if (Tab1DevControl->objectName().isEmpty())
            Tab1DevControl->setObjectName(QString::fromUtf8("Tab1DevControl"));
        Tab1DevControl->resize(667, 616);
        verticalLayout = new QVBoxLayout(Tab1DevControl);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pPBtimerStart = new QPushButton(Tab1DevControl);
        pPBtimerStart->setObjectName(QString::fromUtf8("pPBtimerStart"));
        pPBtimerStart->setCheckable(true);
        pPBtimerStart->setChecked(false);

        horizontalLayout->addWidget(pPBtimerStart);

        pCBtimerValue = new QComboBox(Tab1DevControl);
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->addItem(QString());
        pCBtimerValue->setObjectName(QString::fromUtf8("pCBtimerValue"));

        horizontalLayout->addWidget(pCBtimerValue);

        pPBappQuit = new QPushButton(Tab1DevControl);
        pPBappQuit->setObjectName(QString::fromUtf8("pPBappQuit"));

        horizontalLayout->addWidget(pPBappQuit);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pDialLed = new QDial(Tab1DevControl);
        pDialLed->setObjectName(QString::fromUtf8("pDialLed"));
        pDialLed->setMaximum(255);
        pDialLed->setTracking(true);
        pDialLed->setWrapping(true);
        pDialLed->setNotchesVisible(true);

        horizontalLayout_2->addWidget(pDialLed);

        pLCDNumberLed = new QLCDNumber(Tab1DevControl);
        pLCDNumberLed->setObjectName(QString::fromUtf8("pLCDNumberLed"));
        pLCDNumberLed->setSmallDecimalPoint(false);
        pLCDNumberLed->setDigitCount(2);
        pLCDNumberLed->setMode(QLCDNumber::Hex);
        pLCDNumberLed->setSegmentStyle(QLCDNumber::Filled);

        horizontalLayout_2->addWidget(pLCDNumberLed);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pProgressBarLed = new QProgressBar(Tab1DevControl);
        pProgressBarLed->setObjectName(QString::fromUtf8("pProgressBarLed"));
        pProgressBarLed->setMaximum(255);
        pProgressBarLed->setValue(24);

        horizontalLayout_4->addWidget(pProgressBarLed);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pGRkey = new QGridLayout();
        pGRkey->setObjectName(QString::fromUtf8("pGRkey"));
        pCBkey1 = new QCheckBox(Tab1DevControl);
        pCBkey1->setObjectName(QString::fromUtf8("pCBkey1"));

        pGRkey->addWidget(pCBkey1, 0, 0, 1, 1);

        pCBkey8 = new QCheckBox(Tab1DevControl);
        pCBkey8->setObjectName(QString::fromUtf8("pCBkey8"));

        pGRkey->addWidget(pCBkey8, 1, 3, 1, 1);

        pCBkey2 = new QCheckBox(Tab1DevControl);
        pCBkey2->setObjectName(QString::fromUtf8("pCBkey2"));

        pGRkey->addWidget(pCBkey2, 0, 1, 1, 1);

        pCBkey5 = new QCheckBox(Tab1DevControl);
        pCBkey5->setObjectName(QString::fromUtf8("pCBkey5"));

        pGRkey->addWidget(pCBkey5, 1, 0, 1, 1);

        pCBkey6 = new QCheckBox(Tab1DevControl);
        pCBkey6->setObjectName(QString::fromUtf8("pCBkey6"));

        pGRkey->addWidget(pCBkey6, 1, 1, 1, 1);

        pCBkey7 = new QCheckBox(Tab1DevControl);
        pCBkey7->setObjectName(QString::fromUtf8("pCBkey7"));

        pGRkey->addWidget(pCBkey7, 1, 2, 1, 1);

        pCBkey3 = new QCheckBox(Tab1DevControl);
        pCBkey3->setObjectName(QString::fromUtf8("pCBkey3"));

        pGRkey->addWidget(pCBkey3, 0, 2, 1, 1);

        pCBkey4 = new QCheckBox(Tab1DevControl);
        pCBkey4->setObjectName(QString::fromUtf8("pCBkey4"));

        pGRkey->addWidget(pCBkey4, 0, 3, 1, 1);


        horizontalLayout_3->addLayout(pGRkey);

        pLcdNumberKey = new QLCDNumber(Tab1DevControl);
        pLcdNumberKey->setObjectName(QString::fromUtf8("pLcdNumberKey"));
        pLcdNumberKey->setSmallDecimalPoint(false);
        pLcdNumberKey->setDigitCount(2);
        pLcdNumberKey->setMode(QLCDNumber::Hex);
        pLcdNumberKey->setSegmentStyle(QLCDNumber::Filled);

        horizontalLayout_3->addWidget(pLcdNumberKey);

        horizontalLayout_3->setStretch(0, 1);
        horizontalLayout_3->setStretch(1, 1);

        verticalLayout->addLayout(horizontalLayout_3);

        verticalLayout->setStretch(0, 1);
        verticalLayout->setStretch(1, 4);
        verticalLayout->setStretch(2, 1);
        verticalLayout->setStretch(3, 4);

        retranslateUi(Tab1DevControl);

        QMetaObject::connectSlotsByName(Tab1DevControl);
    } // setupUi

    void retranslateUi(QWidget *Tab1DevControl)
    {
        Tab1DevControl->setWindowTitle(QCoreApplication::translate("Tab1DevControl", "Form", nullptr));
        pPBtimerStart->setText(QCoreApplication::translate("Tab1DevControl", "TimerStart", nullptr));
        pCBtimerValue->setItemText(0, QCoreApplication::translate("Tab1DevControl", "100", nullptr));
        pCBtimerValue->setItemText(1, QCoreApplication::translate("Tab1DevControl", "200", nullptr));
        pCBtimerValue->setItemText(2, QCoreApplication::translate("Tab1DevControl", "500", nullptr));
        pCBtimerValue->setItemText(3, QCoreApplication::translate("Tab1DevControl", "1000", nullptr));
        pCBtimerValue->setItemText(4, QCoreApplication::translate("Tab1DevControl", "2000", nullptr));

        pPBappQuit->setText(QCoreApplication::translate("Tab1DevControl", "Quit", nullptr));
        pCBkey1->setText(QCoreApplication::translate("Tab1DevControl", "1", nullptr));
        pCBkey8->setText(QCoreApplication::translate("Tab1DevControl", "8", nullptr));
        pCBkey2->setText(QCoreApplication::translate("Tab1DevControl", "2", nullptr));
        pCBkey5->setText(QCoreApplication::translate("Tab1DevControl", "5", nullptr));
        pCBkey6->setText(QCoreApplication::translate("Tab1DevControl", "6", nullptr));
        pCBkey7->setText(QCoreApplication::translate("Tab1DevControl", "7", nullptr));
        pCBkey3->setText(QCoreApplication::translate("Tab1DevControl", "3", nullptr));
        pCBkey4->setText(QCoreApplication::translate("Tab1DevControl", "4", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Tab1DevControl: public Ui_Tab1DevControl {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TAB1DEVCONTROL_H
