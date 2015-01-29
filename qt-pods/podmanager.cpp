#include "podmanager.h"

#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>

PodManager::PodManager(QObject *parent)
    : QObject(parent) {
}

bool PodManager::isValidRepository(QString repository) {
    QDir dir(repository);
    QString gitPath = dir.filePath(".git");
    return QFile::exists(gitPath);
}

void PodManager::addPod(QString repository, Pod pod) {
    if(!isValidRepository(repository)) {
        return;
    }

    QDir cwd = QDir::current();
    QDir::setCurrent(repository);

    QProcess::execute(QString("git submodule add %1 %2").arg(pod.url).arg(pod.name));

    QDir::setCurrent(cwd.absolutePath());
}

void PodManager::removePod(QString repository, QString podName) {
    if(!isValidRepository(repository)) {
        return;
    }

    // Check if the repository actually contains such a pod.
    QList<Pod> pods = installedPods(repository);
    bool repositoryContainsPod = false;
    foreach(Pod pod, pods) {
        if(pod.name == podName) {
            repositoryContainsPod = true;
            break;
        }
    }

    if(repositoryContainsPod) {
        QDir cwd = QDir::current();
        QDir::setCurrent(repository);

        QProcess::execute(QString("git rm -rf %1/%2").arg(repository).arg(podName));

        QDir::setCurrent(cwd.absolutePath());
    }
}

void PodManager::updatePods(QString repository) {
    Q_UNUSED(repository);
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

QList<PodManager::Pod> PodManager::availablePods(QStringList sources) {
    QList<Pod> pods;
    foreach(QString source, sources) {
        QNetworkRequest request;
        request.setUrl(QUrl(source));
        QNetworkReply *reply = _networkAccessManager.get(request);

        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        QByteArray response = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(response, &parseError);

        if(QJsonParseError::NoError == parseError.error) {
            QJsonObject object = document.object();
            QStringList keys = object.keys();

            foreach(QString key, keys) {
                Pod pod;
                pod.name = key;
                pod.url = object.value(key).toString();
                pods.append(pod);
            }
        } else {
            qDebug() << source << " is a malformed source: " << parseError.errorString();
        }
    }
    return pods;
}

