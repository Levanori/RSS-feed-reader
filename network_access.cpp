#include "network_access.h"

network_access::network_access(QObject *parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &network_access::replyFinished);
}

void network_access::getRSS()
{
    manager->get(QNetworkRequest(QUrl("http://unian.ua/rss")));
}

void network_access::replyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString readable_data = QString::fromUtf8(data);
        qDebug() << "Дані є:" << readable_data;
    } else {
        qDebug() << "Помилка";
    }
}
