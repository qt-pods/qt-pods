#ifndef PODMANAGER_H
#define PODMANAGER_H

#include <QString>

class PodManager {
public:
    struct Pod {
        QString name;
        QString url;
    };

    PodManager();

    void removePod(QString repository, QString pod);

    QList<Pod> installedPods(QString repository);
};

#endif // PODMANAGER_H
