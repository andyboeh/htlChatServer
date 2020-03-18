#include "htlchatserver.h"
#include "chatserverthread.h"
#include <QDebug>

htlChatServer::htlChatServer(QObject *parent) : QTcpServer(parent)
{
    qDebug() << "htlChatServer started.";
}

void htlChatServer::login(qintptr socketDescriptor, QString username, QString password)
{

}

void htlChatServer::setUsername(qintptr socketDescriptor, QString name)
{
    QString oldusername;
    chatServerThread *thread = mThreads.value(socketDescriptor);

    // Check if this username is already taken by somebody else
    if(mUserMap.values().contains(name) && mUserMap.value(socketDescriptor) != name) {
        connect(this, SIGNAL(errorMessage(QString,QString)), thread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("setUsername", "Username already in use.");
        disconnect(SIGNAL(errorMessage));
        return;
    }

    if(mUserMap.value(socketDescriptor) == name) {
        connect(this, SIGNAL(successMessage(QString)), thread, SLOT(successMessage(QString)));
        emit successMessage("setUsername");
        disconnect(SIGNAL(successMessage));
        return;
    }

    if(mUserMap.contains(socketDescriptor)) {
        emit userDisconnected(mUserMap.value(socketDescriptor));
    }
    mUserMap.insert(socketDescriptor,name);
    connect(this, SIGNAL(successMessage(QString)), thread, SLOT(successMessage(QString)));
    emit successMessage("setUsername");
    disconnect(SIGNAL(successMessage));
    emit newUserConnected(name);
}

void htlChatServer::messageToBroadcast(QString sender, QString message, QString receiver)
{
    emit broadcastMessage(sender, message, receiver);
}

void htlChatServer::clientDisconnected(qintptr socketDescriptor)
{
    QString username = mUserMap.value(socketDescriptor);
    mThreads.remove(socketDescriptor);
    if(!username.isEmpty()) {
        emit userDisconnected(username);
    }
    mUserMap.remove(socketDescriptor);
}

void htlChatServer::getUserList(qintptr socketDescriptor)
{
    QStringList users = mUserMap.values();
    chatServerThread *thread = mThreads.value(socketDescriptor);
    connect(this, SIGNAL(sendUserList(QStringList)), thread, SLOT(sendUserList(QStringList)));
    emit sendUserList(users);
    disconnect(SIGNAL(sendUserList(QStringList)));
}

void htlChatServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "incomingConnection";
    chatServerThread *thread = new chatServerThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(broadcastMessage(QString,QString,QString)), thread, SLOT(messageToBroadcast(QString,QString,QString)));
    connect(thread, SIGNAL(setUsername(qintptr,QString)), this, SLOT(setUsername(qintptr,QString)));
    connect(thread, SIGNAL(clientDisconnected(qintptr)), this, SLOT(clientDisconnected(qintptr)));
    connect(thread, SIGNAL(getUserList(qintptr)), this, SLOT(getUserList(qintptr)));
    connect(thread, SIGNAL(broadcastMessage(QString,QString,QString)), this, SLOT(messageToBroadcast(QString,QString,QString)));
    connect(this, SIGNAL(newUserConnected(QString)), thread, SLOT(newUserConnected(QString)));
    connect(this, SIGNAL(userDisconnected(QString)), thread, SLOT(userDisconnected(QString)));
    thread->start();
    mThreads.insert(socketDescriptor,thread);
}
