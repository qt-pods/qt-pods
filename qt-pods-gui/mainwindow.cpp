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

    // Redirect all messages from console
    setupStdOutRedirect();

    // Application settings object
    _settings = new QSettings("qt-pods", "qt-pods");

    // Create pod manager and move it to a background thread
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
    connect(_podManager, SIGNAL(listInstalledPodsFinished(QString, QList<Pod>)),
            this, SLOT(listInstalledPodsFinished(QString,QList<Pod>)));
    connect(_podManager, SIGNAL(listAvailablePodsFinished(QStringList, QList<Pod>)),
            this, SLOT(listAvailablePodsFinished(QStringList,QList<Pod>)));
    connect(_podManager, SIGNAL(createProjectFinished(QString,bool)),
            this, SLOT(createProjectFinished(QString,bool)));

    // UI setup
    ui->setupUi(this);
    ui->tabWidget->setTabToolTip(ui->tabWidget->indexOf(ui->tabLocalPods),
                                 tr("Manage locally installed pods."));
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->indexOf(ui->tabLocalPods),
                                 tr("Manage locally installed pods."));
    ui->tabWidget->setTabToolTip(ui->tabWidget->indexOf(ui->tabAvailablePods),
                                 tr("View and install from a list of available pods."));
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->indexOf(ui->tabAvailablePods),
                                 tr("View and install from a list of available pods."));
    ui->tabWidget->setTabToolTip(ui->tabWidget->indexOf(ui->tabDevelopmentTools),
                                 tr("Pod developers tools."));
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->indexOf(ui->tabDevelopmentTools),
                                 tr("Pod developers tools."));
    ui->tabWidget->setTabToolTip(ui->tabWidget->indexOf(ui->tabDiagnostic),
                                 tr("Debug information for developers."));
    ui->tabWidget->setTabWhatsThis(ui->tabWidget->indexOf(ui->tabDiagnostic),
                                 tr("Debug information for developers."));

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

    _availablePodsSpinnerWidget = new WaitingSpinnerWidget(ui->tabAvailablePods);
    _localPodsSpinnerWidget = new WaitingSpinnerWidget(ui->tabLocalPods);
    _developmentToolsSpinnerWidget = new WaitingSpinnerWidget(ui->tabDevelopmentTools);

    updateBuildInfo();
    loadSettings();

#ifndef QT_DEBUG
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabDiagnostic));
#endif
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::stdOutActivated(int fileDescriptor) {
#ifdef Q_OS_UNIX
    char readBuffer[1024];
    int numberOfBytesRead = ::read(fileDescriptor, readBuffer, sizeof(readBuffer) - 1);
    if(numberOfBytesRead > 0) {
        // Terminate buffer - just in case.
        readBuffer[numberOfBytesRead] = (char)0;
#ifdef QT_DEBUG
        ui->plainTextEditDiagnostic->appendPlainText(readBuffer);

#else
        statusBar()->showMessage(readBuffer);
#endif
    }
#endif
}

void MainWindow::updateBuildInfo() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[updateBuildInfo]";
#endif
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
    _buildString = buildString;
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
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonAddRepository_clicked]";
#endif
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
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonRemoveRepository_clicked]";
#endif
    int confirmationResult = QMessageBox::warning(this,
        tr("Confirm removal"),
        tr("Are you sure you want to remove the repository \"%1\"?")
            .arg(ui->comboBoxCurrentRepository->currentText()), QMessageBox::No, QMessageBox::Yes);
    if(confirmationResult == QMessageBox::Yes) {
        ui->comboBoxCurrentRepository->removeItem(ui->comboBoxCurrentRepository->currentIndex());
        saveSettings();
    }
}

void MainWindow::on_pushButtonNewProject_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonNewRepository_clicked]";
#endif

    QString directory = QFileDialog::getExistingDirectory(this, tr("Choose a folder to create the project in"));
    if(!directory.isEmpty()) {
        metaObject()->invokeMethod(_podManager, "createProject",
                                   Q_ARG(QString, directory));
    }
}

void MainWindow::on_comboBoxCurrentRepository_currentTextChanged(QString text) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_comboBoxCurrentRepository_currentTextChanged]";
#endif
    setWindowTitle(QString("Qt Pods - %1 - %2").arg(text).arg(_buildString));
    refreshLocalPods();
}

void MainWindow::on_tabWidget_currentChanged(int index) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_tabWidgetPods_currentChanged]";
#endif
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
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_lineEditSearchLocal_textChanged]";
#endif
    _localPodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_lineEditSearchRemote_textChanged(QString text) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_lineEditSearchRemote_textChanged]";
#endif
    _remotePodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_pushButtonRemoveLocalPods_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonRemoveLocalPods_clicked]";
#endif
    QItemSelection itemSelection = ui->tableViewLocal->selectionModel()->selection();
    QModelIndexList selectedModelIndices = _localPodsProxyModel->mapSelectionToSource(itemSelection).indexes();
    QModelIndexList modelIndices;
    foreach(QModelIndex i, selectedModelIndices) {
        if(i.column() == 0) {
            modelIndices.append(i);
        }
    }

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
            tr("Are you sure you want to remove %1 pods from the repository?")
                .arg(modelIndices.count()), QMessageBox::No, QMessageBox::Yes);
    }

    if(confirmationResult == QMessageBox::Yes) {
        _localPodsSpinnerWidget->start();
        QStringList podNames;
        foreach(QModelIndex modelIndex, modelIndices) {
            podNames.append(_localPods->pod(modelIndex).name);
        }

        metaObject()->invokeMethod(_podManager, "removePods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QStringList, podNames));
    }
}

void MainWindow::on_pushButtonUpdateLocalPods_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonUpdateLocalPods_clicked]";
#endif
    QItemSelection itemSelection = ui->tableViewLocal->selectionModel()->selection();
    QModelIndexList selectedModelIndices = _localPodsProxyModel->mapSelectionToSource(itemSelection).indexes();
    QModelIndexList modelIndices;
    foreach(QModelIndex i, selectedModelIndices) {
        if(i.column() == 0) {
            modelIndices.append(i);
        }
    }

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
            podNames.append(_localPods->pod(modelIndex).name);
        }

        metaObject()->invokeMethod(_podManager, "updatePods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QStringList, podNames));
    }
}

void MainWindow::on_pushButtonRefreshLocalPods_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonRefreshLocalPods_clicked]";
#endif
    ui->tableViewLocal->setEnabled(false);
    ui->pushButtonRemoveLocalPods->setEnabled(false);
    ui->pushButtonRefreshLocalPods->setEnabled(false);

    refreshLocalPods();


    ui->tableViewLocal->setEnabled(true);
    ui->pushButtonRemoveLocalPods->setEnabled(true);
    ui->pushButtonRefreshLocalPods->setEnabled(true);
}

void MainWindow::on_pushButtonManageSources_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonManageSources_clicked]";
#endif
    SourcesDialog sourcesDialog;
    QStringList sources = _settings->value("sources").toStringList();

    // At least add one source
    if(sources.isEmpty()) {
        sources << "http://omg-it.works/pods.json";
    }
    sourcesDialog.setSources(sources);

    if(sourcesDialog.exec() == QDialog::Accepted) {
        _settings->setValue("sources", sourcesDialog.sources());
        _settings->sync();
    }
}

void MainWindow::on_pushButtonRefreshAvailablePods_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonRefreshAvailablePods_clicked]";
#endif
    ui->tableViewRemote->setEnabled(false);
    ui->pushButtonInstallPods->setEnabled(false);
    ui->pushButtonRefreshAvailablePods->setEnabled(false);

    refreshAvailablePods();

    ui->tableViewRemote->setEnabled(true);
    ui->pushButtonInstallPods->setEnabled(true);
    ui->pushButtonRefreshAvailablePods->setEnabled(true);
}

void MainWindow::on_pushButtonInstallPods_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonInstallPods_clicked]";
#endif
    _availablePodsSpinnerWidget->start();
    _localPodsSpinnerWidget->start();

    QItemSelection itemSelection = ui->tableViewRemote->selectionModel()->selection();
    QModelIndexList selectedModelIndices =_remotePodsProxyModel->mapSelectionToSource(itemSelection).indexes();
    QModelIndexList modelIndices;
    foreach(QModelIndex i, selectedModelIndices) {
        if(i.column() == 0) {
            modelIndices.append(i);
        }
    }

    QList<Pod> pods = _remotePods->pods(modelIndices);
    metaObject()->invokeMethod(_podManager,
                               "installPods",
                               Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                               Q_ARG(QList<Pod>, pods));
}

void MainWindow::on_pushButtonInstallExternalPod_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonInstallExternalPod_clicked]";
#endif
    PodDialog podDialog;
    if(podDialog.exec() == QDialog::Accepted) {
        _availablePodsSpinnerWidget->start();
        _localPodsSpinnerWidget->start();

        QList<Pod> pods;
        pods.append(podDialog.pod());

        metaObject()->invokeMethod(_podManager,
                                   "installPods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QList<Pod>, pods));
    }
}

void MainWindow::on_pushButtonReportIssue_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonReportIssue_clicked]";
#endif
    QDesktopServices desktopServices;
    desktopServices.openUrl(QUrl("https://github.com/cybercatalyst/qt-pods/issues"));
}

void MainWindow::on_pushButtonExportDiagnostics_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonExportDiagnostics_clicked]";
#endif
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save diagnostics"), "diagnostics.txt", "*.txt");
    if(!fileName.isEmpty()) {
        QFile file(fileName);
        file.open(QFile::ReadWrite);
        if(file.isOpen()) {
            file.write((ui->lineEditBuild->text() + "\n").toUtf8());
            file.write(ui->plainTextEditDiagnostic->toPlainText().toUtf8());
            file.waitForBytesWritten(30000);
            file.close();

            QMessageBox::information(this,
                                     tr("Diagnostics written"),
                                     tr("Diagnostics have been written to %1. Please attach this file when reporting issues. Thank you!")
                                        .arg(file.fileName()));
        } else {
            QMessageBox::warning(this,
                                 tr("Error writing to file"),
                                 tr("Diagnostics could not be written. Reason: %1")
                                    .arg(file.errorString()));
        }
    }
}

void MainWindow::on_tableViewLocal_doubleClicked(QModelIndex index) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_tableViewLocal_doubleClicked]";
#endif
    PodDialog podDialog;
    podDialog.setPod(_localPods->pod(index));
    podDialog.setEditable(false);
    podDialog.exec();
}

void MainWindow::on_tableViewRemote_doubleClicked(QModelIndex index) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_tableViewRemote_doubleClicked]";
#endif
    PodDialog podDialog;
    podDialog.setPod(_remotePods->pod(index));
    podDialog.setEditable(false);
    podDialog.exec();
}

void MainWindow::on_toolButtonChooseDevelopmentPodRoot_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_toolButtonChooseDevelopmentPodRoot_clicked]";
#endif

    QString podRoot = QFileDialog::getExistingDirectory(this, tr("Choose pod root directory"));
    if(!podRoot.isEmpty()) {
        ui->lineEditDevelopmentPodRoot->setText(podRoot);
    }
}

void MainWindow::on_lineEditDevelopmentPodRoot_textChanged(QString text) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_lineEditDevelopmentPodRoot_textChanged]";
#endif
    QFileInfo fileInfo(text);

    if(fileInfo.exists() && fileInfo.isDir()) {
        ui->lineEditDevelopmentPodRoot->setStyleSheet("color: green;");
        ui->pushButtonDevelopmentPodInstall->setEnabled(true);
        //ui->pushButtonDevelopmentPodSanityCheck->setEnabled(true);
    } else {
        ui->lineEditDevelopmentPodRoot->setStyleSheet("color: red;");
        ui->pushButtonDevelopmentPodInstall->setEnabled(false);
        ui->pushButtonDevelopmentPodSanityCheck->setEnabled(false);
    }
}

void MainWindow::on_pushButtonDevelopmentPodSanityCheck_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonDevelopmentPodSanityCheck_clicked]";
#endif

    // TODO: Implement maybe in core.
}

void MainWindow::on_pushButtonDevelopmentPodInstall_clicked() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[on_pushButtonDevelopmentPodInstall_clicked]";
#endif
    _availablePodsSpinnerWidget->start();
    _localPodsSpinnerWidget->start();
    _developmentToolsSpinnerWidget->start();

    QList<Pod> pods;

    QFileInfo fileInfo(ui->lineEditDevelopmentPodRoot->text());
    if(fileInfo.exists() && fileInfo.isDir()) {
        Pod pod;
        pod.url = ui->lineEditDevelopmentPodRoot->text();
        pod.description = tr("Development pod at %1").arg(fileInfo.absolutePath());
        pod.name = fileInfo.fileName();
        pods.append(pod);
        metaObject()->invokeMethod(_podManager,
                                   "installPods",
                                   Q_ARG(QString, ui->comboBoxCurrentRepository->currentText()),
                                   Q_ARG(QList<Pod>, pods));
    }
}

void MainWindow::closeEvent(QCloseEvent *closeEvent) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[closeEvent]";
#endif
    saveSettings();
    QMainWindow::closeEvent(closeEvent);
}

void MainWindow::loadSettings() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[loadSettings]";
#endif
    QStringList repositories = _settings->value("local-repositories").toStringList();
    ui->comboBoxCurrentRepository->addItems(repositories);
    ui->comboBoxCurrentRepository->setCurrentText(_settings->value("active-repository").toString());
}

void MainWindow::saveSettings() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[saveSettings]";
#endif
    QStringList repositories;
    for(int i = 0; i < ui->comboBoxCurrentRepository->count(); i++) {
        repositories.append(ui->comboBoxCurrentRepository->itemText(i));
    }
    _settings->setValue("local-repositories", repositories);
    _settings->setValue("active-repository", ui->comboBoxCurrentRepository->currentText());
    _settings->sync();
}

void MainWindow::installPodsFinished(QString repository, QList<Pod> pods, bool success) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[installPodsFinished]";
#endif
    _availablePodsSpinnerWidget->stop();
    _localPodsSpinnerWidget->stop();
    _developmentToolsSpinnerWidget->stop();

    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabLocalPods));

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
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[removePodsFinished]";
#endif
    _localPodsSpinnerWidget->stop();

    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabLocalPods));

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
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[updatePodsFinished]";
#endif
    _localPodsSpinnerWidget->stop();

    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->tabLocalPods));

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

void MainWindow::listInstalledPodsFinished(QString repository, QList<Pod> installedPods) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[installedPodsFinished]";
#endif
    Q_UNUSED(repository);
    _localPods->setModelData(installedPods);
    _localPodsSpinnerWidget->stop();
    ui->tableViewLocal->resizeColumnsToContents();
}

void MainWindow::listAvailablePodsFinished(QStringList sources, QList<Pod> availablePods) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[availablePodsFinished]";
#endif
    Q_UNUSED(sources);
    _remotePods->setModelData(availablePods);
    _availablePodsSpinnerWidget->stop();
    ui->tableViewRemote->resizeColumnsToContents();
}

void MainWindow::createProjectFinished(QString repository, bool success) {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[refreshLocalPods]";
#endif
    if(success) {
        ui->comboBoxCurrentRepository->addItem(repository);
        ui->comboBoxCurrentRepository->setCurrentText(repository);
        QMessageBox::information(this,
            tr("Successfully created project"),
            tr("Project has been created in \"%1\".").arg(repository));
    } else {
        QMessageBox::critical(this,
            tr("Could not create project"),
            tr("Failed to create project in \"%1\".").arg(repository));
    }
}

void MainWindow::refreshLocalPods() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[refreshLocalPods]";
#endif
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

    metaObject()->invokeMethod(_podManager, "listInstalledPods", Q_ARG(QString, repository));
}

void MainWindow::refreshAvailablePods() {
#ifdef QT_DEBUG
    qDebug() << __TIME__ << "[refreshAvailablePods]";
#endif
    _availablePodsSpinnerWidget->start();

    QStringList sources = _settings->value("sources").toStringList();
    if(sources.isEmpty()) {
        sources << "http://omg-it.works/pods.json";
    }

    metaObject()->invokeMethod(_podManager, "listAvailablePods", Q_ARG(QStringList, sources));
}

