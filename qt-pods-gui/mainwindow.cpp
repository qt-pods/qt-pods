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

// Own includes
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pod.h"
#include "poddialog.h"
#include "sourcesdialog.h"

// Qt includes
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDebug>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    setObjectName("MainWindow");
    setupStdOutRedirect();

    _settings = new QSettings("qt-pods", "qt-pods");

    _workerThread = new QThread();
    _podManager = new PodManager();
    _podManager->moveToThread(_workerThread);
    _workerThread->start();

    connect(_podManager, SIGNAL(installPodsFinished(QString, QList<Pod>, bool)),
            this, SLOT(installPodsFinished(QString,QList<Pod>,bool)));
    connect(_podManager, SIGNAL(removePodsFinished(QString, QStringList, bool)),
            this, SLOT(removePodsFinished(QString, QStringList,bool)));
    connect(_podManager, SIGNAL(updatePodsFinished(QString, QStringList, bool)),
            this, SLOT(updatePodsFinished(QString, QStringList, bool)));
    connect(_podManager, SIGNAL(installedPodsFinished(QString, QList<Pod>)),
            this, SLOT(installedPodsFinished(QString,QList<Pod>)));
    connect(_podManager, SIGNAL(availablePodsFinished(QStringList, QList<Pod>)),
            this, SLOT(availablePodsFinished(QStringList,QList<Pod>)));

    ui->setupUi(this);

    _localPods = new PodsModel();
    _remotePods = new PodsModel();

    _localPodsProxyModel = new QSortFilterProxyModel();
    _localPodsProxyModel->setSourceModel(_localPods);
    _localPodsProxyModel->setFilterKeyColumn(-1);

    _remotePodsProxyModel = new QSortFilterProxyModel();
    _remotePodsProxyModel->setSourceModel(_remotePods);
    _remotePodsProxyModel->setFilterKeyColumn(-1);

    ui->tableViewLocal->setModel(_localPodsProxyModel);
    ui->tableViewRemote->setModel(_remotePodsProxyModel);

    _systemTrayIcon.setIcon(QIcon(":/icons/icons/system-upgrade.svg"));
    _systemTrayIcon.setToolTip("Qt Pods");
    _systemTrayIcon.show();

    _availablePodsSpinnerWidget = new WaitingSpinnerWidget(ui->tableViewRemote);
    _localPodsSpinnerWidget = new WaitingSpinnerWidget(ui->tableViewLocal);

    updateBuildInfo();
    loadSettings();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::stdOutActivated(int fileDescriptor) {
    char readBuffer[1024];
    int numberOfBytesRead = ::read(fileDescriptor, readBuffer, sizeof(readBuffer) - 1);
    if(numberOfBytesRead > 0) {
        // Terminate buffer - just in case.
        readBuffer[numberOfBytesRead] = (char)0;

        ui->plainTextEditDiagnostic->appendPlainText(readBuffer);
    }
}

void MainWindow::updateBuildInfo() {
    QString buildString = QString("%1-%2 (built on %3 at %4)")
            .arg(GIT_VERSION)
#ifdef QT_DEBUG
            .arg("debug")
#else
            .arg("release")
#endif
            .arg(__DATE__)
            .arg(__TIME__);

    ui->lineEditBuild->setText(buildString);
}

void MainWindow::setupStdOutRedirect() {
#ifdef Q_OS_UNIX
    // Redirect our own stdout/stderr.
    int pipeDescriptors[2];
    if(::pipe(pipeDescriptors) == 0) {
        ::dup2(pipeDescriptors[1], STDOUT_FILENO);
        ::dup2(pipeDescriptors[1], STDERR_FILENO);
        _stdOutSocketNotifier = new QSocketNotifier(pipeDescriptors[0], QSocketNotifier::Read, this);
        connect(_stdOutSocketNotifier, SIGNAL(activated(int)),
                this, SLOT(stdOutActivated(int)));
    }
#endif
}

void MainWindow::on_pushButtonAddRepository_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this, tr("Add git repository"));
    if(!directory.isEmpty()) {
        if(!_podManager->isGitRepository(directory)) {
            QMessageBox::warning(this,
                tr("Invalid repository"),
                tr("The directory you supplied does not appear to be the root of a valid git repository."));
            return;
        }

        if(ui->comboBoxCurrentRepository->findText(directory) == -1) {
            ui->comboBoxCurrentRepository->addItem(directory);
        }

        ui->comboBoxCurrentRepository->setCurrentText(directory);

        saveSettings();

        refreshLocalPods();
    }
}

void MainWindow::on_pushButtonRemoveRepository_clicked() {
    int confirmationResult = QMessageBox::warning(this,
        tr("Confirm removal"),
        tr("Are you sure you want to remove the repository \"%1\"?")
            .arg(ui->comboBoxCurrentRepository->currentText()), QMessageBox::No, QMessageBox::Yes);
    if(confirmationResult == QMessageBox::Yes) {
        ui->comboBoxCurrentRepository->removeItem(ui->comboBoxCurrentRepository->currentIndex());
        saveSettings();
    }
}

void MainWindow::on_comboBoxCurrentRepository_currentTextChanged(QString text) {
    setWindowTitle(QString("Qt Pods - %1").arg(text));
    refreshLocalPods();
}

void MainWindow::on_tabWidgetPods_currentChanged(int index) {
    switch (index) {
    case 0:
        refreshLocalPods();
        break;
    case 1:
        refreshAvailablePods();
    default:
        break;
    }
}

void MainWindow::on_lineEditSearchLocal_textChanged(QString text) {
    _localPodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_lineEditSearchRemote_textChanged(QString text) {
    _remotePodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_pushButtonRemoveLocalPods_clicked() {
    QModelIndexList modelIndices = ui->tableViewLocal->selectionModel()->selectedRows(0);
    if(modelIndices.count() == 0) {
        return;
    }

    int confirmationResult;
    if(modelIndices.count() == 1) {
        confirmationResult = QMessageBox::warning(this,
            tr("Confirm removal"),
            tr("Are you sure you want to remove \"%1\" from the repository?")
                .arg(modelIndices.at(0).data().toString()), QMessageBox::No, QMessageBox::Yes);
    } else {
        confirmationResult = QMessageBox::warning(this,
            tr("Confirm removal"),
            tr("Are you sure you want to remove \"%1\" pods from the repository?")
                .arg(modelIndices.count()), QMessageBox::No, QMessageBox::Yes);
    }

    if(confirmationResult == QMessageBox::Yes) {
        _localPodsSpinnerWidget->start();
        QStringList podNames;
        foreach(QModelIndex modelIndex, modelIndices) {
            podNames.append(_localPods->item(modelIndex.row(), 0)->text());
        }

        metaObject()->invokeMethod(_podManager, "removePods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QStringList, podNames));
    }
}

void MainWindow::on_pushButtonUpdateLocalPods_clicked() {
    QModelIndexList modelIndices = ui->tableViewLocal->selectionModel()->selectedRows(0);
    if(modelIndices.count() == 0) {
        return;
    }

    int confirmationResult;
    if(modelIndices.count() == 1) {
        confirmationResult = QMessageBox::warning(this,
            tr("Confirm update"),
            tr("Are you sure you want to update \"%1\"?")
                .arg(modelIndices.at(0).data().toString()),QMessageBox::No, QMessageBox::Yes);
    } else {
        confirmationResult = QMessageBox::warning(this,
            tr("Confirm update"),
            tr("Are you sure you want to update \"%1\" pods?")
                .arg(modelIndices.count()), QMessageBox::No, QMessageBox::Yes);
    }

    if(confirmationResult == QMessageBox::Yes) {
        _localPodsSpinnerWidget->start();
        QStringList podNames;
        foreach(QModelIndex modelIndex, modelIndices) {
            podNames.append(_localPods->item(modelIndex.row(), 0)->text());
        }

        metaObject()->invokeMethod(_podManager, "updatePods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QStringList, podNames));
    }
}

void MainWindow::on_pushButtonRefreshLocalPods_clicked() {
    ui->tableViewLocal->setEnabled(false);
    ui->pushButtonRemoveLocalPods->setEnabled(false);
    ui->pushButtonRefreshLocalPods->setEnabled(false);

    refreshLocalPods();

    ui->tableViewLocal->setEnabled(true);
    ui->pushButtonRemoveLocalPods->setEnabled(true);
    ui->pushButtonRefreshLocalPods->setEnabled(true);
}

void MainWindow::on_pushButtonManageSources_clicked() {
    SourcesDialog sourcesDialog;
    QStringList sources = _settings->value("sources").toStringList();

    // At least add one source
    if(sources.isEmpty()) {
        sources << "https://raw.githubusercontent.com/cybercatalyst/qt-pods-master/master/pods.json";
    }
    sourcesDialog.setSources(sources);

    if(sourcesDialog.exec() == QDialog::Accepted) {
        _settings->setValue("sources", sourcesDialog.sources());
        _settings->sync();
    }
}

void MainWindow::on_pushButtonRefreshAvailablePods_clicked() {
    ui->tableViewRemote->setEnabled(false);
    ui->pushButtonInstallPods->setEnabled(false);
    ui->pushButtonRefreshAvailablePods->setEnabled(false);

    refreshAvailablePods();

    ui->tableViewRemote->setEnabled(true);
    ui->pushButtonInstallPods->setEnabled(true);
    ui->pushButtonRefreshAvailablePods->setEnabled(true);
}

void MainWindow::on_pushButtonInstallPods_clicked() {
    _availablePodsSpinnerWidget->start();
    _localPodsSpinnerWidget->start();

    QModelIndexList modelIndices = ui->tableViewRemote->selectionModel()->selectedRows(0);
    QList<Pod> pods;
    foreach(QModelIndex modelIndex, modelIndices) {
        Pod pod;
        pod.name = _remotePods->item(modelIndex.row(), 0)->text();
        pod.url = _remotePods->item(modelIndex.row(), 1)->text();
        pods.append(pod);
    }

    metaObject()->invokeMethod(_podManager,
                               "installPods",
                               Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                               Q_ARG(QList<Pod>, pods));
}

void MainWindow::on_pushButtonInstallExternalPod_clicked() {
    PodDialog podDialog;
    if(podDialog.exec() == QDialog::Accepted) {
        _availablePodsSpinnerWidget->start();
        _localPodsSpinnerWidget->start();

        QModelIndexList modelIndices = ui->tableViewRemote->selectionModel()->selectedRows(0);
        QList<Pod> pods;
        pods.append(podDialog.pod());

        metaObject()->invokeMethod(_podManager,
                                   "installPods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QList<Pod>, pods));
    }
}

void MainWindow::on_pushButtonReportIssue_clicked() {
    QDesktopServices desktopServices;
    desktopServices.openUrl(QUrl("https://github.com/cybercatalyst/qt-pods/issues"));
}

void MainWindow::closeEvent(QCloseEvent *closeEvent) {
    saveSettings();
    QMainWindow::closeEvent(closeEvent);
}

void MainWindow::loadSettings() {
    QStringList repositories = _settings->value("local-repositories").toStringList();
    ui->comboBoxCurrentRepository->addItems(repositories);
    ui->comboBoxCurrentRepository->setCurrentText(_settings->value("active-repository").toString());
}

void MainWindow::saveSettings() {
    QStringList repositories;
    for(int i = 0; i < ui->comboBoxCurrentRepository->count(); i++) {
        repositories.append(ui->comboBoxCurrentRepository->itemText(i));
    }
    _settings->setValue("local-repositories", repositories);
    _settings->setValue("active-repository", ui->comboBoxCurrentRepository->currentText());
    _settings->sync();
}

void MainWindow::installPodsFinished(QString repository, QList<Pod> pods, bool success) {
    _availablePodsSpinnerWidget->stop();
    _localPodsSpinnerWidget->stop();

    ui->tabWidgetPods->setCurrentIndex(0);

    if(success) {
        QMessageBox::information(this,
                                 tr("Installing pods in %1").arg(repository),
                                 tr("Finished installing %1 pods.").arg(pods.count()));
    } else {
        QMessageBox::information(this,
                                 tr("Installing pods in %1").arg(repository),
                                 tr("Installing some of the selected pods failed."));
    }

    refreshLocalPods();
}

void MainWindow::removePodsFinished(QString repository, QStringList podNames, bool success) {
    _localPodsSpinnerWidget->stop();

    ui->tabWidgetPods->setCurrentIndex(0);

    if(success) {
        QMessageBox::information(this,
                                 tr("Removing pods in %1").arg(repository),
                                 tr("Finished removing %1 pods.").arg(podNames.count()));
    } else {
        QMessageBox::information(this,
                                 tr("Removing pods in %1").arg(repository),
                                 tr("Removing some of the selected pods failed."));
    }

    refreshLocalPods();
}

void MainWindow::updatePodsFinished(QString repository, QStringList podNames, bool success) {
    _localPodsSpinnerWidget->stop();

    ui->tabWidgetPods->setCurrentIndex(0);

    if(success) {
        QMessageBox::information(this,
                                 tr("Updating pods in %1").arg(repository),
                                 tr("Finished updating %1 pods.").arg(podNames.count()));
    } else {
        QMessageBox::information(this,
                                 tr("Updating pods in %1").arg(repository),
                                 tr("Updating some of the selected pods failed."));
    }

    refreshLocalPods();
}

void MainWindow::installedPodsFinished(QString repository, QList<Pod> installedPods) {
    // Clear model
    _localPods->reset();
    foreach(Pod pod, installedPods) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(pod.name));
        row.append(new QStandardItem(pod.url));

        if(!_podManager->checkPod(repository, pod.name)) {
            QFont font = row.at(0)->font();
            font.setItalic(true);
            row.at(0)->setFont(font);
            row.at(1)->setFont(font);
        }

        foreach(QStandardItem *item, row) {
            item->setEditable(false);
        }

        _localPods->appendRow(row);
    }

    _localPodsSpinnerWidget->stop();
}

void MainWindow::availablePodsFinished(QStringList sources, QList<Pod> availablePods) {
    Q_UNUSED(sources);

    // Clear model
    _remotePods->reset();
    foreach(Pod pod, availablePods) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(pod.name));
        row.append(new QStandardItem(pod.url));

        foreach(QStandardItem *item, row) {
            item->setEditable(false);
        }

        _remotePods->appendRow(row);
    }

    _availablePodsSpinnerWidget->stop();
}

void MainWindow::refreshLocalPods() {
    _localPodsSpinnerWidget->start();

    QString repository = ui->comboBoxCurrentRepository->currentText();
    if(!_podManager->isGitRepository(repository)) {
        QMessageBox::warning(this,
            tr("Invalid repository"),
            tr("The directory you supplied does not appear to be the root of a valid git repository."));

        int index;
        while((index = ui->comboBoxCurrentRepository->findText(repository)) != -1) {
            ui->comboBoxCurrentRepository->removeItem(index);
        }
        return;
    }

    metaObject()->invokeMethod(_podManager, "installedPods", Q_ARG(QString, repository));
}

void MainWindow::refreshAvailablePods() {
    _availablePodsSpinnerWidget->start();

    QStringList sources = _settings->value("sources").toStringList();
    if(sources.isEmpty()) {
        sources << "https://raw.githubusercontent.com/cybercatalyst/qt-pods-master/master/pods.json";
    }

    metaObject()->invokeMethod(_podManager, "availablePods", Q_ARG(QStringList, sources));
}

