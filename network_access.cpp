#include "network_access.h"

network_access::network_access(QObject *parent)
    : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &network_access::replyFinished);
}

void network_access::getDataFromInternet(QString url)
{
    manager->get(QNetworkRequest(QUrl(url)));
}

void network_access::replyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString url = reply->request().url().toString();

        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString(); // searching by type performed much better than by ending

        if (contentType.startsWith("image/") || url.endsWith(".webp")) {
            emit imageDownloaded(data, url);
        }
        else {
            QString readable_data = QString::fromUtf8(data);
            // qDebug() << "Дані є:" << readable_data;

            QStringList parts = readable_data.split("<item>");

            for (int num_part = 1; num_part < parts.size(); num_part++) {
                QString currentPart = parts[num_part];
                QString title, description, link, date, category, imageUrl;

                int start_title = currentPart.indexOf("<title>") + 7;
                int end_title = currentPart.indexOf("</title>");
                if (start_title != -1 && end_title != -1) { // receiving the text of something provided that it is found
                    title = cleanHtml(currentPart.sliced(start_title, end_title - start_title));
                }

                int start_description  = currentPart.indexOf("<description>") + 13;
                int end_description  = currentPart.indexOf("</description>");
                if (start_description != -1 && end_description != -1) {
                    description = cleanHtml(currentPart.sliced(start_description, end_description - start_description));
                    int postIndex = description.indexOf("The post ");
                    if (postIndex != -1) {
                        description = description.left(postIndex);
                    }
                }

                int start_link = currentPart.indexOf("<link>") + 6;
                int end_link = currentPart.indexOf("</link>");
                if (start_link != -1 && end_link != -1) {
                    link = cleanHtml(currentPart.sliced(start_link, end_link - start_link));
                }

                int start_date = currentPart.indexOf("<pubDate>") + 9;
                int end_date = currentPart.indexOf("</pubDate>");
                if (start_date == -1 && end_date == -1){
                    start_date = currentPart.indexOf("<lastBuildDate>") + 15;
                    end_date = currentPart.indexOf("</lastBuildDate>");
                }
                if (start_date != -1 && end_date != -1) {
                    date = currentPart.sliced(start_date, end_date - start_date);
                }

                int start_category   = currentPart.indexOf("<category>") + 10;
                int end_category   = currentPart.indexOf("</category>");
                if (start_category != -1 && end_category != -1) {
                    category = cleanHtml(currentPart.sliced(start_category , end_category  - start_category));
                }

                /* int start_image = currentPart.indexOf("<enclosure");
                if (start_image == -1){
                    start_image = currentPart.indexOf("<media:thumbnail");
                } */
                int start_urlPhoto = currentPart.indexOf("url=\"");
                if (start_urlPhoto == -1) {
                    start_urlPhoto = currentPart.indexOf("src=\"");
                }
                if (start_urlPhoto == -1) {
                    start_urlPhoto = currentPart.indexOf("src='");
                }
                if (start_urlPhoto != -1) {
                    start_urlPhoto += 5;
                    int firstEnd = currentPart.indexOf("\"", start_urlPhoto);
                    int secondEnd = currentPart.indexOf("'", start_urlPhoto);
                    int end_urlPhoto = -1;

                    if (firstEnd != -1 && secondEnd != -1) {
                        if (firstEnd < secondEnd) {
                            end_urlPhoto = firstEnd;
                        } else {
                            end_urlPhoto = secondEnd;
                        }
                    } else if (firstEnd != -1) {
                        end_urlPhoto = firstEnd;
                    } else {
                        end_urlPhoto = secondEnd;
                    }
                    if (end_urlPhoto != -1) {
                        imageUrl = currentPart.sliced(start_urlPhoto, end_urlPhoto - start_urlPhoto);
                    }
                }

                imageUrl = QUrl::fromEncoded(imageUrl.toUtf8()).toString(); // to avoid any errors with data (%)
                emit newsSend(category, title, date, description, link, imageUrl);
                /* qDebug() << "Категорія:" << category;
                qDebug() << "Заголовок:" << title;
                qDebug() << "Дата:     " << date;
                qDebug() << "Опис:     " << description;
                qDebug() << "Фото:     " << imageUrl;
                qDebug() << "Лінк:     " << link; */
            }
        }
    } else {
        qDebug() << "Помилка";
    }
}

QString network_access::cleanHtml(QString html) {
    QString result = html;
    result.replace("&quot;", "\"").replace("&apos;", "'").replace("&amp;",  "&").replace("&laquo;", "«")
        .replace("&raquo;", "»").replace("&nbsp;",  " ").replace("&ndash;", "–").replace("&mdash;", "—")
        .replace("&lt;",    "<").replace("&gt;",    ">").replace("Повний текст новини", "")
        .replace("<![CDATA[", "").replace("]]>", "").replace("\"", "");

    while (result.contains("<")) {
        int start = result.indexOf("<");
        int end = result.indexOf(">", start);

        if (end != -1) {
            result.remove(start, end - start + 1);
        } else {
            result.remove(start, 1);
        }
    }
    while (result.contains("\n\n")) {
        result.replace("\n\n", "\n");
    }
    return result;
}
