#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_toolButtonRepository_clicked() {
    QString directory = QFileDialog::getExistingDirectory(this, "Open local git repository");
    if(!directory.isEmpty()) {
        configureForRepository(directory);
    }
}

void MainWindow::on_lineEditSearchLocal_textChanged(QString text) {
    _localPodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_lineEditSearchRemote_textChanged(QString text) {
    _remotePodsProxyModel->setFilterWildcard(text);
}

void MainWindow::on_pushButtonRemoveLocalPods_clicked() {

}

void MainWindow::on_pushButtonUpdateLocalPods_clicked() {

}

void MainWindow::configureForRepository(QString path) {
    ui->lineEditRepository->setText(path);

    // Clear model
    _localPods->reset();

    QDir dir(path);
    QString modulesPath = dir.filePath(".gitmodules");
    if(QFile::exists(modulesPath)) {
        QSettings gitmodules(modulesPath, QSettings::IniFormat);
        QStringList childGroups = gitmodules.childGroups();
        foreach(QString childGroup, childGroups) {
            if(childGroup.startsWith("submodule")) {
                gitmodules.beginGroup(childGroup);

                QString podName = gitmodules.value("path").toString();
                QString podUrl  = gitmodules.value("url").toString();

                if(podName.startsWith("q")) {
                    QList<QStandardItem*> row;
                    row.append(new QStandardItem(podName));
                    row.append(new QStandardItem(podUrl));

                    foreach(QStandardItem *item, row) {
                        item->setEditable(false);
                    }

                    _localPods->appendRow(row);
                }

                gitmodules.endGroup();
            }
        }
    }
}
