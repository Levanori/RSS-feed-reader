#ifndef RSS_STORAGE_H
#define RSS_STORAGE_H

#include <QTreeWidget>
#include <QString>
#include <QMessageBox>
#include <QFile>

class rss_storage
{
public:
    rss_storage(QTreeWidget *tree);
    void loadSites();
    bool addRssToTree(QString folder, QString name, QString url);
    void saveAllSites(QString fileName);
private:
    QTreeWidget *rssTree;
    void loadTheTree(QString fileName);
};

#endif // RSS_STORAGE_H
