#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

void MainWindow::configureForRepository(QString root) {
    ui->lineEditRepository->setText(root);


}
