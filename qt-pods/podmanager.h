#ifndef PODMANAGER_H
#define PODMANAGER_H

#include <QString>
#include <QObject>

#include <QNetworkAccessManager>

class PodManager : public QObject {
    Q_OBJECT
public:
    struct Pod {
        QString name;
        QString url;
    };

    PodManager(QObject *parent = 0);

    bool isValidRepository(QString repository);

    void addPod(QString repository, Pod pod);
    void removePod(QString repository, QString podName);

    void updatePods(QString repository);

    QList<Pod> installedPods(QString repository);

    QList<Pod> availablePods(QStringList sources);

private:
    QNetworkAccessManager _networkAccessManager;
};

#endif // PODMANAGER_H
