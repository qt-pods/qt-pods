#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_toolButtonRepository_clicked();

private:
    void configureForRepository(QString root);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
