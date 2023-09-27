#ifndef TAB1DEVCONTROL_H
#define TAB1DEVCONTROL_H

#include <QWidget>
#include "keyled.h"

namespace Ui {
class Tab1DevControl;
}

class Tab1DevControl : public QWidget
{
    Q_OBJECT

public:
    explicit Tab1DevControl(QWidget *parent = nullptr);
    ~Tab1DevControl();

private:
    Ui::Tab1DevControl *ui;
    KeyLed *pKeyLed;

private slots:
    void progressBarSetSlot(int);
    void keyCheckBoxSlot(int);

};

#endif // TAB1DEVCONTROL_H
