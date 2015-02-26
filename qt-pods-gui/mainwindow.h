///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    This file is part of qt-pods.                                          //
//    Copyright (C) 2015 Jacob Dawid, jacob@omg-it.works                     //
//                                                                           //
//    qt-pods is free software: you can redistribute it and/or modify        //
//    it under the terms of the GNU General Public License as published by   //
//    the Free Software Foundation, either version 3 of the License, or      //
//    (at your option) any later version.                                    //
//                                                                           //
//    qt-pods is distributed in the hope that it will be useful,             //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                           //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with qt-pods. If not, see <http://www.gnu.org/licenses/>.        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Own includes
#include "podsmodel.h"

// qt-pods includes
#include "podmanager.h"

// qwaitingspinner includes
#include "waitingspinnerwidget.h"

// Qt includes
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QSystemTrayIcon>
#include <QSocketNotifier>
#include <QThread>
#include <QSettings>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupStdOutRedirect();

public slots:
    // Local repository management
    void on_pushButtonAddRepository_clicked();
    void on_pushButtonRemoveRepository_clicked();
    void on_comboBoxCurrentRepository_currentTextChanged(QString text);
    void on_tabWidget_currentChanged(int index);

    // Local pods (local repository)
    void on_pushButtonUpdateLocalPods_clicked();
    void on_pushButtonRefreshLocalPods_clicked();
    void on_pushButtonRemoveLocalPods_clicked();
    void on_lineEditSearchLocal_textChanged(QString text);
    void on_tableViewLocal_doubleClicked(QModelIndex index);

    // Remote pods (available pods)
    void on_pushButtonManageSources_clicked();
    void on_pushButtonRefreshAvailablePods_clicked();
    void on_pushButtonInstallPods_clicked();
    void on_pushButtonInstallExternalPod_clicked();
    void on_lineEditSearchRemote_textChanged(QString text);
    void on_tableViewRemote_doubleClicked(QModelIndex index);

    // Diagnostics
    void on_pushButtonReportIssue_clicked();
    void on_pushButtonExportDiagnostics_clicked();

    // Development tools
    void on_toolButtonChooseDevelopmentPodRoot_clicked();
    void on_lineEditDevelopmentPodRoot_textChanged(QString text);
    void on_pushButtonDevelopmentPodSanityCheck_clicked();
    void on_pushButtonDevelopmentPodInstall_clicked();

    void on_webViewDevelopment_urlChanged(QUrl url);
    void on_lineEditDevelopmentUrl_returnPressed();

protected:
    void closeEvent(QCloseEvent *closeEvent);

private slots:
    void stdOutActivated(int fileDescriptor);

    // Pod manager result handlers
    void installPodsFinished(QString repository, QList<Pod> pods, bool success);
    void removePodsFinished(QString repository, QStringList podNames, bool success);
    void updatePodsFinished(QString repository, QStringList podNames, bool success);
    void installedPodsFinished(QString repository, QList<Pod> installedPods);
    void availablePodsFinished(QStringList sources, QList<Pod> availablePods);

private:
    void updateBuildInfo();
    void loadSettings();
    void saveSettings();

    void refreshLocalPods();
    void refreshAvailablePods();

    QSocketNotifier *_stdOutSocketNotifier;
    QSystemTrayIcon _systemTrayIcon;
    QThread *_workerThread;
    QSettings *_settings;

    QString _buildString;

    PodManager *_podManager;
    PodsModel *_localPods;
    PodsModel *_remotePods;

    QSortFilterProxyModel *_localPodsProxyModel;
    QSortFilterProxyModel *_remotePodsProxyModel;

    QNetworkAccessManager _networkAccessManager;

    WaitingSpinnerWidget *_availablePodsSpinnerWidget;
    WaitingSpinnerWidget *_localPodsSpinnerWidget;
    WaitingSpinnerWidget *_developmentToolsSpinnerWidget;
    WaitingSpinnerWidget *_webViewSpinnerWidget;

    Ui::MainWindow *ui;
};
