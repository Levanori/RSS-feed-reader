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
        // qDebug() << "Дані є:" << readable_data;

        QStringList parts = readable_data.split("<item>");

        for (int num_part = 1; num_part < parts.size(); num_part++) {
            QString currentPart = parts[num_part];
            QString title, description, link, date, category, imageUrl;

            int start_title = currentPart.indexOf("<title>") + 7;
            int end_title = currentPart.indexOf("</title>");

            int start_description  = currentPart.indexOf("<description>") + 13;
            int end_description  = currentPart.indexOf("</description>");

            int start_link = currentPart.indexOf("<link>") + 6;
            int end_link = currentPart.indexOf("</link>");

            int start_date = currentPart.indexOf("<pubDate>") + 9;
            int end_date = currentPart.indexOf("</pubDate>");

            int start_category   = currentPart.indexOf("<category>") + 10;
            int end_category   = currentPart.indexOf("</category>");

            int start_enclosure   = currentPart.indexOf("<enclosure"); // image

            if (start_title != -1 && end_title != -1) { // receiving the text of something provided that it is found
                title = currentPart.sliced(start_title, end_title - start_title);
                title = title.replace("<![CDATA[","").replace("\"","").replace("]]>","");
            }

            if (start_description != -1 && end_description != -1) {
                description = currentPart.sliced(start_description, end_description - start_description);
                description = description.replace("<![CDATA[","").replace("\"","").replace("]]>",""); // upd: figure out how to clean up the copy-paste
            }

            if (start_link != -1 && end_link != -1) {
                link = currentPart.sliced(start_link, end_link - start_link);
            }

            if (start_date != -1 && end_date != -1) {
                date = currentPart.sliced(start_date, end_date - start_date);
            }

            if (start_category != -1 && end_category != -1) {
                category = currentPart.sliced(start_category, end_category - start_category);
            }

            if (start_enclosure != -1) {
                int start_url = currentPart.indexOf("url=\"", start_enclosure);
                int end_enclosure = currentPart.indexOf("/>", start_enclosure);

                if (start_url != -1 && end_enclosure != -1) {
                    start_url += 5;
                    int end_url = currentPart.indexOf("\"", start_url);
                    imageUrl = currentPart.sliced(start_url, end_url - start_url);
                }
            }

            qDebug() << "Категорія:" << category;
            qDebug() << "Заголовок:" << title;
            qDebug() << "Дата:     " << date;
            qDebug() << "Опис:     " << description;
            qDebug() << "Фото:     " << imageUrl;
            qDebug() << "Лінк:     " << link;
        }
    } else {
        qDebug() << "Помилка";
    }
}


