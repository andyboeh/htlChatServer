#include "chatserverthread.h"
#include "chatserverconfig.h"
#include <QtNetwork>
#include <QDebug>
#include <QCoreApplication>

chatServerThread::chatServerThread(int socketDescriptor, QObject *parent) : QThread(parent)
{
    mSocketDescriptor = socketDescriptor;
    mSocket = nullptr;
    qRegisterMetaType<QAbstractSocket::SocketError>();
}

void chatServerThread::run()
{
    qDebug() << "Chat server thread running:" << mSocketDescriptor;
    mSocket = new QSslSocket();
    if(!mSocket->setSocketDescriptor(mSocketDescriptor)) {
        qDebug() << mSocket->error();
        return;
    }

    chatServerConfig &csc = chatServerConfig::getInstance();
    QSslKey key = csc.getKey();
    QList<QSslCertificate> certs = csc.getCerts();

    if(!key.isNull() && !certs.isEmpty()) {
        mSocket->setLocalCertificateChain(certs);
        mSocket->setPrivateKey(key);
        mSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
    }


    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    mStream.setDevice(mSocket);
    mStream.setVersion(QDataStream::Qt_5_0);

    bool running = true;

    mCommandList.append("sendGreeting");

    while(running) {
        waitForCommand();
        if(mCommandList.isEmpty())
            continue;

        QString command = mCommandList.takeFirst();

        if(command == "stopThread") {
            running = false;
        } else if(command == "sendGreeting") {
            prepareGreeting();
        } else if(command == "sendCommandList") {

            if(mMessagesToSend.isEmpty())
                continue;

            QStringList commands = mMessagesToSend.takeFirst();
            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_5_0);
            out << commands;
            mSocket->write(data);
        }

    }

    mSocket->disconnectFromHost();
    mSocket->close();
    delete mSocket;
    mSocket = nullptr;
}

void chatServerThread::prepareGreeting() {
    QStringList commands;
    QString ver = QCoreApplication::applicationVersion();

    commands.append("welcomeToServer");
    commands.append("Version: " + ver);
    commands.append("Commands: setUsername, sendMessage, getUserList, startEncryption");
    sendCommandList(commands);
}

void chatServerThread::sendCommandList(QStringList commands) {

    mMessagesToSend.append(commands);
    mCommandList.append("sendCommandList");
}

void chatServerThread::messageToBroadcast(QString sender, QString message, QString receiver)
{
    if(receiver.isEmpty() || receiver == mUsername) {
        QStringList commands;
        commands.append("newMessage");
        commands.append(sender);
        commands.append(message);
        if(receiver.isEmpty())
            commands.append("public");
        else
            commands.append("private");
        sendCommandList(commands);
        qDebug() << commands;
    }
}

void chatServerThread::newUserConnected(QString username)
{
    QStringList commands;
    commands.append("newUserConnected");
    commands.append(username);
    sendCommandList(commands);
}

void chatServerThread::userDisconnected(QString username)
{
    QStringList commands;
    commands.append("userDisconnected");
    commands.append(username);
    sendCommandList(commands);
}

void chatServerThread::sendUserList(QStringList userlist)
{
    QStringList commands;
    commands.append("sendUserList");
    commands.append(userlist);
    sendCommandList(commands);
}

void chatServerThread::errorMessage(QString command, QString message)
{
    QStringList commands;
    commands.append("commandFailed");
    commands.append(command);
    commands.append(message);
    sendCommandList(commands);
}

void chatServerThread::successMessage(QString command)
{
    QStringList commands;
    commands.append("commandSuccessful");
    commands.append(command);
    sendCommandList(commands);
}

void chatServerThread::readyRead()
{
    mStream.startTransaction();

    QStringList commands;
    mStream >> commands;

    if(!mStream.commitTransaction())
        return;

    if(commands.isEmpty())
        return;

    QString command = commands.at(0);

    if(command == "setUsername" && commands.length() > 1) {
        mUsername = commands.at(1);
        emit setUsername(mSocketDescriptor, commands.at(1));
    } else if(command == "sendMessage" && commands.length() > 1) {
        if(commands.length() > 2) {
            emit broadcastMessage(mUsername, commands.at(1), commands.at(2));
        } else {
            emit broadcastMessage(mUsername, commands.at(1), "");
        }
    } else if(command == "startEncryption") {
        mSocket->startServerEncryption();
    } else if(command == "getUserList") {
        emit getUserList(mSocketDescriptor);
    }

    if(mSocket->bytesAvailable() > 0) {
        QTimer::singleShot(0, this, SLOT(readyRead()));
    }
}

void chatServerThread::socketError(QAbstractSocket::SocketError error)
{
    qDebug() << "socketError" << error;
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        emit clientDisconnected(mSocketDescriptor);
        mCommandList.append("stopThread");
        break;

    default:
        emit clientDisconnected(mSocketDescriptor);
        mCommandList.append("stopThread");
    }
}
