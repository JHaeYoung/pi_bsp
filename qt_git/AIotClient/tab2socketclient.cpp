#include "tab2socketclient.h"
#include "ui_tab2socketclient.h"

Tab2SocketClient::Tab2SocketClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab2SocketClient)
{
    ui->setupUi(this);

    pSocketClient = new SocketClient(this);
    connect(ui->pPBServerConnect,SIGNAL(clicked(bool)),this, SLOT(connectToSeverSlot(bool)));
    connect(pSocketClient, SIGNAL(sigSocketRecv(QString)),this, SLOT(socketRecvUpdataSlot(QString)));


}
void Tab2SocketClient::connectToSeverSlot(bool bCheck)
{
    bool bOk;
    if(bCheck)
    {
        pSocketClient->slotConnectToServer(bOk);
        if(bOk)
        {
            ui->pPBServerConnect->setText("Server Connected ");
        }

    }
    else
    {
        ui->pPBServerConnect->setText("Server Disconnected ");
        pSocketClient->slotClosedByServer();
    }
}

void Tab2SocketClient::socketRecvUpdataSlot(QString strRecvData)
{
    ui->pTextDataRecv->append(strRecvData);
}


Tab2SocketClient::~Tab2SocketClient()
{
    delete ui;
}
