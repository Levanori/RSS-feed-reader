#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QLabel>
#include <QDesktopServices>
#include "network_access.h"
#include "rss_storage.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void newsReceived(QString category, QString title, QString date, QString description, QString link, QString imageUrl);
    void openLink(QTableWidgetItem *item);
    void imagePlace(QByteArray data, QString url);
    void addSiteByUser();
    void treeRSSClicked(QTreeWidgetItem *item);
    void deleteSiteByUser();
private:
    Ui::MainWindow *ui;
    network_access *net;
    rss_storage *rssStorage;
    QMultiMap<QString, QLabel*> imageMap; // link to the photo and a specific label for the photo location
};
#endif // MAINWINDOW_H
