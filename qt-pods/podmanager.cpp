#include "podmanager.h"

#include <QDir>
#include <QSettings>

PodManager::PodManager() {
}


void PodManager::removePod(QString repository, QString pod) {

}

QList<PodManager::Pod> PodManager::installedPods(QString repository) {
    QList<Pod> pods;
    QDir dir(repository);
    QString gitmodulesPath = dir.filePath(".gitmodules");
    if(QFile::exists(gitmodulesPath)) {
        QSettings gitmodules(gitmodulesPath, QSettings::IniFormat);
        QStringList childGroups = gitmodules.childGroups();
        foreach(QString childGroup, childGroups) {
            if(childGroup.startsWith("submodule")) {
                gitmodules.beginGroup(childGroup);
                Pod pod;
                pod.name = gitmodules.value("path").toString();
                pod.url  = gitmodules.value("url").toString();
                pods.append(pod);
                gitmodules.endGroup();
            }
        }
    }
    return pods;
}
