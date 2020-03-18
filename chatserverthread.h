#ifndef CHATSERVERTHREAD_H
#define CHATSERVERTHREAD_H

#include <QDebug>
#include <QThread>
#include <QAbstractEventDispatcher>
#include <QAbstractSocket>
#include <QDataStream>

class QTcpSocket;

class chatServerThread : public QThread
{
    Q_OBJECT
public:
    chatServerThread(int socketDescriptor, QObject *parent = nullptr);
    void run() override;

public slots:
    void messageToBroadcast(QString sender, QString message, QString receiver);
    void newUserConnected(QString username);
    void userDisconnected(QString username);
    void sendUserList(QStringList userlist);
    void errorMessage(QString command, QString message);
    void successMessage(QString command);
private slots:
    void readyRead();
    void socketError(QAbstractSocket::SocketError error);
signals:
    void started();
    void broadcastMessage(QString sender, QString message, QString receiver);
    void setUsername(qintptr socket, QString username);
    void clientDisconnected(qintptr socketDescriptor);
    void getUserList(qintptr socketDescriptor);

private:
    void waitForCommand() {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if(!dispatcher) {
            qCritical() << "thread with no dispatcher.";
            return;
        }
        while(mCommandList.isEmpty()) {
            dispatcher->processEvents(QEventLoop::AllEvents);
            QThread::msleep(10);
        }
    }

    int mSocketDescriptor;
    QStringList mCommandList;
    QDataStream mStream;
    QString mUsername;
    QTcpSocket *mSocket;
    QList<QStringList> mMessagesToSend;
    void sendCommandList(QStringList commands);
    void prepareGreeting();
};

#endif // CHATSERVERTHREAD_H
