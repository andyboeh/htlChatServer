#include <QCoreApplication>
#include "htlchatserver.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    htlChatServer server;
    if(!server.listen(QHostAddress::Any, 62304)) {
        qDebug() << "Error starting server.";
        return EXIT_FAILURE;
    }

    return a.exec();
}
