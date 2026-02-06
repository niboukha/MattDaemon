#include "Gui.h"

Gui::Gui() : QMainWindow(nullptr), 
             tcpSocket(new QTcpSocket(this)),
             isConnected(false),
             isAuthenticated(false) ,
             authState(STATE_AUTH) {
    setupUI();
    
    connect(connectBtn, &QPushButton::clicked, this, &Gui::onConnect);
    connect(sendBtn, &QPushButton::clicked, this, &Gui::onSendCommand);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Gui::onReceiveData);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &Gui::onDisconnect);

    onConnect();
}

Gui::~Gui() {
    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            tcpSocket->waitForDisconnected(1000);
        }
        tcpSocket->close();
    }

}

void Gui::setupUI() {
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel *titleLabel = new QLabel("Matt-Daemon Client", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #00ff00;");
    mainLayout->addWidget(titleLabel);
    
    // Display box
    displayBox = new QTextEdit(this);
    displayBox->setReadOnly(true);
    displayBox->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #00ff00;"
        "  border: 2px solid #00ff00;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "  font-family: 'Courier New';"
        "  font-size: 11px;"
        "}"
    );
    displayBox->setMinimumHeight(200);
    mainLayout->addWidget(displayBox);
    
    // Input layout
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    // Input field
    inputField = new QLineEdit(this);
    inputField->setEchoMode(QLineEdit::Normal);
    inputField->setStyleSheet(
        "QLineEdit {"
        "  background-color: #1e1e1e;"
        "  color: #00ff00;"
        "  border: 2px solid #00ff00;"
        "  border-radius: 3px;"
        "  padding: 8px;"
        "  font-family: 'Courier New';"
        "}"
    );
    inputLayout->addWidget(inputField);
    
    // Send button
    sendBtn = new QPushButton("Send", this);
    sendBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #00ff00;"
        "  color: #1e1e1e;"
        "  border: none;"
        "  border-radius: 3px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #00cc00;"
        "}"
    );
    sendBtn->setEnabled(false);
    inputLayout->addWidget(sendBtn);
    
    // Connect button
    connectBtn = new QPushButton("Connect", this);
    connectBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #0066ff;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 3px;"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0050cc;"
        "}"
    );
    inputLayout->addWidget(connectBtn);
    
    mainLayout->addLayout(inputLayout);
    
    // Window properties
    setWindowTitle("Matt-Daemon Client");
    setGeometry(100, 100, 400, 400);
    setStyleSheet("QMainWindow { background-color: #1e1e1e; }");
}

void Gui::appendOutput(const QString &text) {
    displayBox->moveCursor(QTextCursor::End);
    displayBox->insertPlainText(text + "\n");
    displayBox->verticalScrollBar()->setValue(displayBox->verticalScrollBar()->maximum());
}

void Gui::onConnect() {
    if (!isConnected) {
        tcpSocket->connectToHost("localhost", 4242);

        if (tcpSocket->waitForConnected(5000)) {
            isConnected = true;
            connectBtn->setText("Disconnect");
            connectBtn->setEnabled(true);
            sendBtn->setEnabled(true);
            inputField->setEnabled(true);
        } else {
            appendOutput("[ERROR] Unable to connect to server");
            connectBtn->setEnabled(true);
        }
    } else {
        appendOutput("[INFO] Disconnecting...");
        connectBtn->setEnabled(false);
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            tcpSocket->waitForDisconnected(3000);
        }
        connectBtn->setEnabled(true);
    }
   
}

void Gui::onSendCommand() {
    QString input = inputField->text();
    if (input.isEmpty()) return;

    // Only display user input for username and password
    if (inputField->echoMode() == QLineEdit::Password) {
        appendOutput(QString(input.size(), '*'));
    } else if (inputField->placeholderText().contains("username", Qt::CaseInsensitive)) {
        appendOutput(input);
    }
    // Do NOT display command input for Matt-daemon> prompt

    tcpSocket->write((input + "\n").toStdString().c_str());
    tcpSocket->flush();
    inputField->clear();
}

void Gui::onReceiveData() {
    QByteArray data = tcpSocket->readAll();
    QString response = QString::fromStdString(std::string(data.constData(), data.size()));
    response.replace("\r\n", "\n");
    response.replace('\r', '\n');

    QStringList lines = response.split('\n', Qt::KeepEmptyParts);
    for (const QString& line : lines) {
        if (!line.isEmpty())
            appendOutput(line);
        // Set input mode based on prompt
        if (line.contains("USERNAME:")) {
            inputField->setEchoMode(QLineEdit::Normal);
            inputField->setPlaceholderText("Enter username");
            inputField->setFocus();
        } else if (line.contains("PASSWORD:")) {
            inputField->setEchoMode(QLineEdit::Password);
            inputField->setPlaceholderText("Enter password");
            inputField->setFocus();
        } else if (line.contains("Matt-daemon>")) {
            inputField->setEchoMode(QLineEdit::Normal);
            inputField->setPlaceholderText("Matt-daemon> ");
            inputField->setFocus();
        }
    }
}

void Gui::onDisconnect() {
    isConnected = false;
    appendOutput("[INFO] Disconnected from server");
    isAuthenticated = false;
    connectBtn->setText("Connect");
    connectBtn->setEnabled(true);
    sendBtn->setEnabled(false);
    inputField->setEnabled(false);
    inputField->clear();
    tcpSocket->close();

}