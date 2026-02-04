#ifndef GUI_H   
#define GUI_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QScrollBar>
#include <QApplication>
#include <cstdio>

enum AuthState {
    STATE_AUTH,
    STATE_READY
};

class Gui : public QMainWindow {
    // NO Q_OBJECT

public:
    Gui(QWidget *parent = nullptr);
    ~Gui();
    Gui(const Gui &) = delete;
    Gui& operator=(const Gui &) = delete;

private:
    void setupUI();
    void appendOutput(const QString &text);
    void switchToAuth();
    void switchToReady();
    void onConnect();
    void onSendCommand();
    void onReceiveData();
    void onDisconnect();

    QTcpSocket *tcpSocket;
    QTextEdit *displayBox;
    QLineEdit *inputField;
    QPushButton *sendBtn;
    QPushButton *connectBtn;

    
    bool isConnected;
    bool isAuthenticated;
    AuthState authState;
};

#endif // GUI_H