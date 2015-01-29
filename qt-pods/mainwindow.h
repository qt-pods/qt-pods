#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QNetworkAccessManager>

#include "podsmodel.h"
#include "podmanager.h"

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
    void on_pushButtonRemoveRepository_clicked();

    void on_lineEditSearchLocal_textChanged(QString text);
    void on_lineEditSearchRemote_textChanged(QString text);

    void on_comboBoxCurrentRepository_currentTextChanged(QString text);

    void on_pushButtonUpdateLocalPods_clicked();
    void on_pushButtonRefreshLocalPods_clicked();
    void on_pushButtonRemoveLocalPods_clicked();

    void on_pushButtonRefreshAvailablePods_clicked();

protected:
    void closeEvent(QCloseEvent *closeEvent);

private slots:
    void requestFinished(QNetworkReply* networkReply);

private:
    void loadSettings();
    void saveSettings();

    bool isValidGitRepository(QString path);
    void refreshLocalPods();

    void updateRemotePods();

    PodManager _podManager;
    PodsModel *_localPods;
    PodsModel *_remotePods;

    QSortFilterProxyModel *_localPodsProxyModel;
    QSortFilterProxyModel *_remotePodsProxyModel;

    QNetworkAccessManager _networkAccessManager;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
