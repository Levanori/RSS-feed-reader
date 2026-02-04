#include "rss_storage.h"

rss_storage::rss_storage(QTreeWidget *tree) : rssTree(tree) {}

void rss_storage::loadSites() {
    QFile userFile("user_sites.txt");
    if (userFile.open(QIODevice::ReadOnly)) {
        userFile.close();
        loadTheTree("user_sites.txt");
    } else {
        loadTheTree(":/configs/default_sites.txt");
        saveAllSites("user_sites.txt");
    }
}

void rss_storage::loadTheTree(QString fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QString readableData = QString::fromUtf8(data);
        QStringList listOfItemsForTree = readableData.split('\n');

        for (int index = 0; index < listOfItemsForTree.size(); index++) {
            QString itemForTree = listOfItemsForTree[index].replace("\r", "");
            if (itemForTree.size() > 1) {
                QStringList endListOfItemsForTree = itemForTree.split(';');

                if (endListOfItemsForTree.size() == 3) {
                    addRssToTree(endListOfItemsForTree[0], endListOfItemsForTree[1], endListOfItemsForTree[2]);
                }
                else if (endListOfItemsForTree.size() == 2) {
                    addRssToTree("", endListOfItemsForTree[0], endListOfItemsForTree[1]);
                }
            }
        }
    }
}

bool rss_storage::addRssToTree(QString folder, QString name, QString url) {
    if (name.isEmpty() || url.isEmpty()) {
        return 0;
    }

    for (int indexOfItem = 0; indexOfItem < rssTree->topLevelItemCount(); indexOfItem++) { // rss name and link identity check
        QTreeWidgetItem *topItem = rssTree->topLevelItem(indexOfItem);
        bool errorDuplicate = 0;

        if (topItem->text(1) == url || (!topItem->text(1).isEmpty() && topItem->text(0) == name)) {
            errorDuplicate = 1;
        }
        for (int indexItemInFolder = 0; indexItemInFolder < topItem->childCount(); indexItemInFolder++) {
            if (topItem->child(indexItemInFolder)->text(1) == url || topItem->child(indexItemInFolder)->text(0) == name) {
                errorDuplicate = 1;
            }
        }
        if (errorDuplicate) {
            QMessageBox::warning(rssTree, "Warning!", "Виявлено дублювання назви чи RSS стрічки");
            return 0;
        }
    }

    if (folder.size() > 0) {
        QTreeWidgetItem *foundFolder = 0;

        for (int index = 0; index < rssTree->topLevelItemCount(); index++) {
            if (rssTree->topLevelItem(index)->text(0) == folder) {
                foundFolder = rssTree->topLevelItem(index);
                break;
            }
        }

        if (foundFolder == 0) {
            foundFolder = new QTreeWidgetItem(rssTree);
            foundFolder->setText(0, folder);

        }
        QTreeWidgetItem *siteItem = new QTreeWidgetItem(foundFolder);
        siteItem->setText(0, name);
        siteItem->setText(1, url);
    }
    else {
        QTreeWidgetItem *siteItem = new QTreeWidgetItem(rssTree);
        siteItem->setText(0, name);
        siteItem->setText(1, url);
    }
    return 1;
}

void rss_storage::saveAllSites(QString fileName) {
    QFile fileOfSites(fileName);
    if (fileOfSites.open(QIODevice::WriteOnly | QIODevice::Text)) {
        for (int indexItem = 0; indexItem < rssTree->topLevelItemCount(); indexItem++) {
            QTreeWidgetItem *item = rssTree->topLevelItem(indexItem);

            if (item->text(1) == "all_news") {
                continue;
            }

            if (item->childCount() > 0) {
                for (int indexItemInFolder = 0; indexItemInFolder < item->childCount(); indexItemInFolder++) {
                    QString folder = item->text(0);
                    QString name = item->child(indexItemInFolder)->text(0);
                    QString url = item->child(indexItemInFolder)->text(1);

                    fileOfSites.write(folder.toUtf8() + ";" + name.toUtf8() + ";" + url.toUtf8() + "\n");
                }
            }
            else if (!item->text(1).isEmpty()) {
                QString name = item->text(0);
                QString url = item->text(1);

                fileOfSites.write(";" + name.toUtf8() + ";" + url.toUtf8() + "\n");
            }
        }
        fileOfSites.close();
    }
}

bool rss_storage::deleteRss(QString folder, QString name, QString url) {
    for (int indexOfItem = 0; indexOfItem < rssTree->topLevelItemCount(); indexOfItem++) {
        QTreeWidgetItem *topItem = rssTree->topLevelItem(indexOfItem);

        if (!folder.isEmpty() && name.isEmpty() && url.isEmpty()) {
            if (topItem->text(0) == folder && topItem->childCount() > 0) {
                delete topItem;
                saveAllSites("user_sites.txt");
                return 1;
            }
        }

        if (folder.isEmpty() && !name.isEmpty() && url.isEmpty()) { // upd: if sb have time, remove the the copy-paste
            for (int indexItemInFolder = 0; indexItemInFolder < topItem->childCount(); indexItemInFolder++) {
                if (topItem->child(indexItemInFolder)->text(0) == name) {
                    QTreeWidgetItem *parentFolder = topItem;

                    delete topItem->child(indexItemInFolder);
                    if (parentFolder->childCount() == 0) {
                        delete parentFolder;
                    }
                    saveAllSites("user_sites.txt");
                    return 1;
                }
            }
            if (!topItem->text(1).isEmpty() && topItem->text(0) == name) {
                delete topItem;
                saveAllSites("user_sites.txt");
                return 1;
            }
        }

        if (folder.isEmpty() && name.isEmpty() && !url.isEmpty()) {
            for (int indexItemInFolder = 0; indexItemInFolder < topItem->childCount(); indexItemInFolder++) {
                if (topItem->child(indexItemInFolder)->text(1) == url) {
                    QTreeWidgetItem *parentFolder = topItem;

                    delete topItem->child(indexItemInFolder);
                    if (parentFolder->childCount() == 0){
                        delete parentFolder;
                    }
                    saveAllSites("user_sites.txt");
                    return 1;
                }
            }
            if (topItem->text(1) == url) {
                delete topItem;
                saveAllSites("user_sites.txt");
                return 1;
            }
        }
    }
    return 0;
}
