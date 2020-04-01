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
    if(!mThreads.contains(socketDescriptor))
        return;
    QString oldusername;
    chatServerThread *thread = mThreads.value(socketDescriptor);

    // Check if this username is already taken by somebody else
    if(mUserMap.values().contains(name) && mUserMap.value(socketDescriptor) != name) {
        connect(this, SIGNAL(errorMessage(QString,QString)), thread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("setUsername", "Username already in use.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }

    if(mUserMap.value(socketDescriptor) == name) {
        connect(this, SIGNAL(successMessage(QString)), thread, SLOT(successMessage(QString)));
        emit successMessage("setUsername");
        disconnect(SIGNAL(successMessage(QString)));
        return;
    }

    if(mUserMap.contains(socketDescriptor)) {
        emit userDisconnected(mUserMap.value(socketDescriptor));
    }
    mUserMap.insert(socketDescriptor,name);
    connect(this, SIGNAL(successMessage(QString)), thread, SLOT(successMessage(QString)));
    emit successMessage("setUsername");
    disconnect(SIGNAL(successMessage(QString)));
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
    if(!mThreads.contains(socketDescriptor))
        return;
    QStringList users = mUserMap.values();
    chatServerThread *thread = mThreads.value(socketDescriptor);
    connect(this, SIGNAL(sendUserList(QStringList)), thread, SLOT(sendUserList(QStringList)));
    emit sendUserList(users);
    disconnect(SIGNAL(sendUserList(QStringList)));
}

void htlChatServer::startFileTransfer(qintptr socketDescriptor, QString receiver, QString filename, QString filesize)
{
    if(!mThreads.contains(socketDescriptor))
        return;
    if(!mUserMap.values().contains(receiver)) {
        chatServerThread *senderThread = mThreads.value(socketDescriptor);
        connect(this, SIGNAL(errorMessage(QString,QString)), senderThread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("startFileTransfer", "Receiver not connected.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }
    qintptr recv = mUserMap.key(receiver);
    chatServerThread *thread = mThreads.value(recv);
    QString sender = mUserMap.value(socketDescriptor);

    connect(this, SIGNAL(startFileTransfer(QString,QString,QString)), thread, SLOT(startFileTransfer(QString,QString,QString)));
    emit startFileTransfer(sender, filename, filesize);
    disconnect(SIGNAL(startFileTransfer(QString,QString,QString)));
}

void htlChatServer::sendChunk(qintptr socketDescriptor, QString receiver, QString data)
{
    if(!mThreads.contains(socketDescriptor))
        return;
    if(!mUserMap.values().contains(receiver)) {
        chatServerThread *senderThread = mThreads.value(socketDescriptor);
        connect(this, SIGNAL(errorMessage(QString,QString)), senderThread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("sendChunk", "Receiver not connected.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }
    qintptr recv = mUserMap.key(receiver);
    chatServerThread *thread = mThreads.value(recv);
    QString sender = mUserMap.value(socketDescriptor);

    connect(this, SIGNAL(sendChunk(QString,QString)), thread, SLOT(sendChunk(QString,QString)));
    emit sendChunk(sender, data);
    disconnect(SIGNAL(sendChunk(QString,QString)));
}

void htlChatServer::finishFileTransfer(qintptr socketDescriptor, QString receiver)
{
    if(!mThreads.contains(socketDescriptor))
        return;
    if(!mUserMap.values().contains(receiver)) {
        chatServerThread *senderThread = mThreads.value(socketDescriptor);
        connect(this, SIGNAL(errorMessage(QString,QString)), senderThread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("finishFileTransfer", "Receiver not connected.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }
    qintptr recv = mUserMap.key(receiver);
    chatServerThread *thread = mThreads.value(recv);
    QString sender = mUserMap.value(socketDescriptor);

    connect(this, SIGNAL(finishFileTransfer(QString)), thread, SLOT(finishFileTransfer(QString)));
    emit finishFileTransfer(sender);
    disconnect(SIGNAL(finishFileTransfer(QString)));
}

void htlChatServer::acceptFileTransfer(qintptr socketDescriptor, QString receiver)
{
    if(!mThreads.contains(socketDescriptor))
        return;
    if(!mUserMap.values().contains(receiver)) {
        chatServerThread *senderThread = mThreads.value(socketDescriptor);
        connect(this, SIGNAL(errorMessage(QString,QString)), senderThread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("acceptFileTransfer", "Receiver not connected.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }
    qintptr recv = mUserMap.key(receiver);
    chatServerThread *thread = mThreads.value(recv);
    QString sender = mUserMap.value(socketDescriptor);

    connect(this, SIGNAL(fileTransferAccepted(QString)), thread, SLOT(fileTransferAccepted(QString)));
    emit fileTransferAccepted(sender);
    disconnect(SIGNAL(fileTransferAccepted(QString)));
}

void htlChatServer::rejectFileTransfer(qintptr socketDescriptor, QString receiver)
{
    if(!mThreads.contains(socketDescriptor))
        return;
    if(!mUserMap.values().contains(receiver)) {
        chatServerThread *senderThread = mThreads.value(socketDescriptor);
        connect(this, SIGNAL(errorMessage(QString,QString)), senderThread, SLOT(errorMessage(QString,QString)));
        emit errorMessage("rejectFileTransfer", "Receiver not connected.");
        disconnect(SIGNAL(errorMessage(QString,QString)));
        return;
    }
    qintptr recv = mUserMap.key(receiver);
    chatServerThread *thread = mThreads.value(recv);
    QString sender = mUserMap.value(socketDescriptor);

    connect(this, SIGNAL(fileTransferRejected(QString)), thread, SLOT(fileTransferRejected(QString)));
    emit fileTransferRejected(sender);
    disconnect(SIGNAL(fileTransferRejected(QString)));
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
    connect(thread, SIGNAL(startFileTransfer(qintptr,QString,QString,QString)), this, SLOT(startFileTransfer(qintptr,QString,QString,QString)));
    connect(thread, SIGNAL(sendChunk(qintptr,QString,QString)), this, SLOT(sendChunk(qintptr,QString,QString)));
    connect(thread, SIGNAL(finishFileTransfer(qintptr,QString)), this, SLOT(finishFileTransfer(qintptr,QString)));
    connect(thread, SIGNAL(acceptFileTransfer(qintptr,QString)), this, SLOT(acceptFileTransfer(qintptr,QString)));
    connect(thread, SIGNAL(rejectFileTransfer(qintptr,QString)), this, SLOT(rejectFileTransfer(qintptr,QString)));
    connect(this, SIGNAL(newUserConnected(QString)), thread, SLOT(newUserConnected(QString)));
    connect(this, SIGNAL(userDisconnected(QString)), thread, SLOT(userDisconnected(QString)));

    thread->start();
    mThreads.insert(socketDescriptor,thread);
}
