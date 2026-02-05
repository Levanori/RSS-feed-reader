#ifndef NETWORK_ACCESS_H
#define NETWORK_ACCESS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QDateTime>

using namespace std;

class network_access : public QObject
{
    Q_OBJECT
public:
    explicit network_access(QObject *parent = nullptr);
    void getDataFromInternet(QString url);
    void stopAll();
private slots:
    void replyFinished(QNetworkReply *reply);
private:
    QNetworkAccessManager *manager;
    QString cleanHtml(QString html);
signals:
    void newsSend(QString category, QString title, QString date, QString description, QString link, QString imageUrl);
    void imageDownloaded(QByteArray data, QString url);
};

#endif // NETWORK_ACCESS_H
