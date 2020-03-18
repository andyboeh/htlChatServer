#ifndef CHATSERVERCONFIG_H
#define CHATSERVERCONFIG_H

#include <QObject>
#include <QSslCertificate>
#include <QSslKey>

class chatServerConfig
{
public:
    static chatServerConfig &getInstance();

    QString getKeyFile();
    QString getCertFile();

    QSslKey getKey();
    QList<QSslCertificate> getCerts();

    void setKeyFile(QString key);
    void setCertFile(QString cert);
private:
    Q_DISABLE_COPY(chatServerConfig);
    chatServerConfig();
    ~chatServerConfig();
    QList<QSslCertificate> mCerts;
    QSslKey mKey;
    QString mCertFile;
    QString mKeyFile;
};

#endif // CHATSERVERCONFIG_H
