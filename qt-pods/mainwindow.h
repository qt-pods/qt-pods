#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>

#include "podsmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_toolButtonRepository_clicked();

    void on_lineEditSearchLocal_textChanged(QString text);
    void on_lineEditSearchRemote_textChanged(QString text);

    void on_pushButtonRemoveLocalPods_clicked();
    void on_pushButtonUpdateLocalPods_clicked();

private:
    void configureForRepository(QString path);

    PodsModel *_localPods;
    PodsModel *_remotePods;

    QSortFilterProxyModel *_localPodsProxyModel;
    QSortFilterProxyModel *_remotePodsProxyModel;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
