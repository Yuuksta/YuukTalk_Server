#include "mainwindow.h"
#include "ui_mainwindow.h"

int I = 0;
int J = 0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    saveData();
    delete ui;
}

void MainWindow::init(){
    loadData();
    Server = new QTcpServer;
    Socket = new QTcpSocket;
    if(Server->listen(QHostAddress::Any,6666)){
        ui->textBrowser->setText("Start Listen");
        qDebug() << "Start Listen";
    }
    else{
        ui->textBrowser->setText(Server->errorString());
        qDebug() << Server->errorString();
        Server->close();
    }
    connect(Server,SIGNAL(newConnection()),SLOT(newConnection()));
    connect(ui->pushButton_showOnlineList,SIGNAL(clicked(bool)),SLOT(showOnlineList()));
    connect(ui->pushButton_showOfflineMsg,SIGNAL(clicked(bool)),SLOT(showOfflineMessage()));
}

void MainWindow::newConnection(){
    Socket = Server->nextPendingConnection();
    //ui->textBrowser->append("here comes a connection");
    connect(Socket,SIGNAL(readyRead()),SLOT(receiveData()));
}

void MainWindow::receiveData(){
    QByteArray Data = Socket->readAll();
    QString DataString = Data;
    QString dataBaseString[Max];
    QString OfflineMsgToSend;
    int i,j;
    for(i = 0;i < Max;i++){
        dataBaseString[i] = dataBase[i];
    }
    if(DataString.section("|",2,2) == "login"){ //Msg to log in
        for(i = 0;i < Max; i++){
            if(dataBaseString[i].section("|",0,0) == DataString.section("|",0,0)
                    && dataBaseString[i].section("|",1,1) == DataString.section("|",1,1)){ //compare ID and Password
                for(j=0;j<100;j++){
                    if(DataString.section("|",0,0) == OfflineData[j].section("|",3,3)){ //prepare to send offline message
                        OfflineMsgToSend.append(OfflineData[j].section("|",1,1) + ": " + OfflineData[j].section("|",4,4) + "\n");
                    }
                }
                ui->textBrowser->append(OfflineMsgToSend);
                Socket->write("loginOK|"+OfflineMsgToSend.toLocal8Bit());

                ui->textBrowser->append(DataString.section("|",0,0)+" is Online");
                break;
            }
        }
        if(i == Max) Socket->write("ERROR");
    }
    else if(DataString.section("|",1,1) == "IP"){
        IPList[I++] = (DataString.section("|",0,0)+"|"+DataString.section("|",2,2));
    }
    else if(DataString.section("|",2,2) == "signup"){ //Msg to sign up
        Data.chop(7); //left ID and Password
        if(dataBase[Max - 1].contains("|")){  //no space for registing
            Socket->write("full");
            ui->textBrowser->append("SignUp Failed");
        }
        else{
            for(i = 0;i < Max;i++){
                if(dataBaseString[i].section("|",0,0) == DataString.section("|",0,0)){ //ID has been registed
                    Socket->write("alreadyReg");
                    ui->textBrowser->append("SignUp Failed");
                    break;
                }
                else if(dataBase[i].isEmpty()){ //registed successfully
                         dataBase[i] = Data;
                         Socket->write("signupOK");
                         ui->textBrowser->append("ID:"+DataString.section("|",0,0)+" SignUp Successfully");
                         ui->textBrowser->append("Password:" +DataString.section("|",1,1));
                         break;
                     }
            }
        }
    }
    else if(DataString.section("|",1,1) == "findPassword"){ //Msg to findPassword
        Data.chop(13); //left ID
        ui->textBrowser->append("Someone wanna find password");
        ui->textBrowser->append("ID:"+Data);
        for(i = 0;i < Max; i++){
            if(DataString.section("|",0,0) == dataBaseString[i].section("|",0,0)){ //if find
                Socket->write(dataBaseString[i].section("|",1,1).toLocal8Bit());
                ui->textBrowser->append("have found!");
                break;
            }
        }
        if(i == Max){
            Socket->write("have not found");
            ui->textBrowser->append("have not found");
        }
    }
    else if(DataString.section("|",1,1) == "offline"){ //Msg to be offline
        ui->textBrowser->append(DataString.section("|",0,0) + " is OffLine");
        for(i=0;i<Max;i++){
            if(DataString.section("|",0,0) == IPList[i].section("|",0,0)){ //remove from the Online List
                IPList[i] = "";
                //break;
            }
        }
        Socket->close();
    }
    else if(DataString.section("|",0,0) == "ConnectTo"){ //Msg to connect to other user
        for(i=0;i<Max;i++){
            if(DataString.section("|",1,1) == IPList[i].section("|",0,0)){ //ID equal
                Socket->write("IP|"+IPList[i].section("|",1,1).toLocal8Bit()); //Send IP to Client
                ui->textBrowser->append("want to connect to "+DataString.section("|",1,1));
                break;
            }
        }
        if(i == Max) Socket->write(DataString.section("|",1,1).toLocal8Bit()+(" is offline!"));
    }
    else{
        ui->textBrowser->append(DataString.toLocal8Bit());
        OfflineData[J++] = DataString;
    }
}

void MainWindow::showOnlineList(){
    ui->textBrowser->append("Online List:");
    for(int j=0;j<Max;j++){ //online list
        if(IPList[j].contains("|")){
            ui->textBrowser->append(IPList[j]);
        }
    }
}

void MainWindow::showOfflineMessage(){
    ui->textBrowser->append("Offline Message:");
    for(int j=0;j<100;j++){ //online list
        if(OfflineData[j].contains("|")){
            ui->textBrowser->append(OfflineData[j]);
        }
    }
}

void MainWindow::saveData(){
    QFile file("dataBase.dat");
    if(!file.open(QIODevice::WriteOnly)){
        return;
    }
    QDataStream out(&file);
    for(int i = 0;i < Max;i++){
        for(int j = 0; j < dataBase[i].size();j++){
            dataBase[i][j] = dataBase[i][j]^1;
        }
        out << dataBase[i];
    }
    file.flush();
    file.close();
}

void MainWindow::loadData(){
    QFile file("dataBase.dat");
    if(!file.open(QIODevice::ReadOnly)){
        return;
    }
    QDataStream in(&file);
    for(int i = 0;!in.atEnd();i++){
        in >> dataBase[i];
        for(int j = 0; j < dataBase[i].size();j++){
            dataBase[i][j] = dataBase[i][j]^1;
        }
    }
    for(int i = 0;i < Max;i++){
        ui->textBrowser->append(dataBase[i]);
    }
}
