#ifndef NETWORK_ACCESS_H
#define NETWORK_ACCESS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

using namespace std;

class network_access : public QObject
{
    Q_OBJECT
public:
    explicit network_access(QObject *parent = nullptr);
    void getRSS();
private slots:
    void replyFinished(QNetworkReply *reply);
private:
    QNetworkAccessManager *manager;
signals:
    void newsSend(QString category, QString title, QString date, QString description);
};

#endif // NETWORK_ACCESS_H
