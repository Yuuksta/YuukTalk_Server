#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QIODevice>

#define Max 20

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public:
    void init();
    //void newConnection();
    void saveData();
    void loadData();
    void encryptData();

public slots:
    void newConnection();
    void receiveData();
    void showOnlineList();
    void showOfflineMessage();

private:
    Ui::MainWindow *ui;
    QTcpServer *Server;
    QTcpSocket *Socket;
    QByteArray dataBase[Max];
    QString IPList[Max];
    QString OfflineData[100];
};

#endif // MAINWINDOW_H
