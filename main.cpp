#include <QCoreApplication>
#include "htlchatserver.h"
#include "chatserverconfig.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("htlChatServer");
    QCoreApplication::setApplicationVersion("0.2");

    QCommandLineParser parser;
    parser.setApplicationDescription("htlChatServer");
    parser.addVersionOption();
    QCommandLineOption certOption(QStringList() << "c" << "certificate",
                                  "Use server certificate from <path>",
                                  "path");
    parser.addOption(certOption);
    QCommandLineOption keyOption(QStringList() << "k" << "key",
                                 "User server key from <path>",
                                 "path");
    parser.addOption(keyOption);

    parser.process(a);

    QString certFile = parser.value(certOption);
    QString keyFile = parser.value(keyOption);

    if(!QFile(certFile).exists() || !QFile(keyFile).exists()) {
        qDebug() << "No certificates specified.";
    } else {
        qDebug() << "Using certificates chain: " << certFile;
        qDebug() << "Using key file          : " << keyFile;
        chatServerConfig &csc = chatServerConfig::getInstance();
        csc.setKeyFile(keyFile);
        csc.setCertFile(certFile);
    }

    htlChatServer server;
    if(!server.listen(QHostAddress::Any, 62304)) {
        qDebug() << "Error starting server.";
        return EXIT_FAILURE;
    }

    return a.exec();
}
