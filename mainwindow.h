#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "network_access.h"
#include <QLabel>
#include <QTreeWidgetItem>
#include <QFile>

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
private:
    Ui::MainWindow *ui;
    network_access *net;
    QMultiMap<QString, QLabel*> imageMap; // link to the photo and a specific label for the photo location
    void loadSites();
    void readAndAddToTree(QString fileName);
};
#endif // MAINWINDOW_H
