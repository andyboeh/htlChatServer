#ifndef HTLCHATSERVER_H
#define HTLCHATSERVER_H

#include <QtNetwork>
#include <QMap>

class chatServerThread;

class htlChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit htlChatServer(QObject *parent = nullptr);
signals:
    void broadcastMessage(QString sender, QString message, QString receiver);
    void loginReply(bool result);
    void sendUserList(QStringList userlist);
    void newUserConnected(QString username);
    void userDisconnected(QString username);
    void errorMessage(QString command, QString message);
    void successMessage(QString command);
    void startFileTransfer(QString username, QString filename, QString filesize);
    void finishFileTransfer(QString username);
    void sendChunk(QString username, QString data);
    void acceptFileTransfer(QString username);
    void rejectFileTransfer(QString username);
public slots:
    void login(qintptr socketDescriptor, QString username, QString password);
    void setUsername(qintptr socketDescriptor, QString name);
    void messageToBroadcast(QString sender, QString message, QString receiver);
    void clientDisconnected(qintptr socketDescriptor);
    void getUserList(qintptr socketDescriptor);
    void startFileTransfer(qintptr socketDescriptor, QString receiver, QString filename, QString filesize);
    void sendChunk(qintptr socketDescriptor, QString receiver, QString data);
    void finishFileTransfer(qintptr socketDescriptor, QString receiver);
    void acceptFileTransfer(qintptr socketDescriptor, QString receiver);
    void rejectFileTransfer(qintptr socketDescriptor, QString receiver);
private:
    void incomingConnection(qintptr socketDescriptor) override;

    QMap<qintptr, chatServerThread*> mThreads;
    QMap<qintptr, QString> mUserMap;
};

#endif // HTLCHATSERVER_H
