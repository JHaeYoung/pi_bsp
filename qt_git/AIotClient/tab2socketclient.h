#ifndef TAB2SOCKETCLIENT_H
#define TAB2SOCKETCLIENT_H

#include <QWidget>
#include "socketclient.h"
namespace Ui {
class Tab2SocketClient;
}

class Tab2SocketClient : public QWidget
{
    Q_OBJECT

public:
    explicit Tab2SocketClient(QWidget *parent = nullptr);
    ~Tab2SocketClient();

private:
    Ui::Tab2SocketClient *ui;
    SocketClient *pSocketClient;
private slots:
    void connectToSeverSlot(bool);
    void socketRecvUpdataSlot(QString);
};

#endif // TAB2SOCKETCLIENT_H
