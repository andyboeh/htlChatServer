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
public slots:
    void login(qintptr socketDescriptor, QString username, QString password);
    void setUsername(qintptr socketDescriptor, QString name);
    void messageToBroadcast(QString sender, QString message, QString receiver);
    void clientDisconnected(qintptr socketDescriptor);
    void getUserList(qintptr socketDescriptor);
private:
    void incomingConnection(qintptr socketDescriptor) override;

    QMap<qintptr, chatServerThread*> mThreads;
    QMap<qintptr, QString> mUserMap;
};

#endif // HTLCHATSERVER_H
