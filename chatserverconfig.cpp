#include "chatserverconfig.h"
#include <QFile>

chatServerConfig &chatServerConfig::getInstance()
{
    static chatServerConfig instance;
    return instance;
}

QString chatServerConfig::getKeyFile()
{
    return mKeyFile;
}

QString chatServerConfig::getCertFile()
{
    return mCertFile;
}

QSslKey chatServerConfig::getKey()
{
    return mKey;
}

QList<QSslCertificate> chatServerConfig::getCerts()
{
    return mCerts;
}

void chatServerConfig::setKeyFile(QString key)
{
    QFile keyFile(key);
    if(keyFile.exists()) {
        if(!keyFile.open(QIODevice::ReadOnly)) {
            return;
        }
        QByteArray data = keyFile.readAll();
        QSslKey sslKey(data, QSsl::Rsa);
        if(!sslKey.isNull()) {
            mKey = sslKey;
            mKeyFile = key;
        }
    }
}

void chatServerConfig::setCertFile(QString cert)
{

    QFile certFile(cert);
    if(certFile.exists()) {
        QList<QSslCertificate> certs = QSslCertificate::fromPath(cert);
        if(!certs.isEmpty()) {
            mCerts = certs;
            mCertFile = cert;
        }
    }
}

chatServerConfig::chatServerConfig()
{

}

chatServerConfig::~chatServerConfig()
{

}
