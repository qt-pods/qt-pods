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
#include "podmanager.h"

// Qt includes
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

void PodManager::installPod(QString repository, Pod pod) {
    if(!isValidRepository(repository)) {
        return;
    }

    QDir cwd = QDir::current();
    QDir::setCurrent(repository);

    QProcess::execute(QString("git submodule add %1 %2").arg(pod.url).arg(pod.name));

    QDir::setCurrent(cwd.absolutePath());

    // Inject the "pod" flag into .gitmodules
    QDir dir(repository);
    QString gitmodulesPath = dir.filePath(".gitmodules");
    if(QFile::exists(gitmodulesPath)) {
        QSettings gitmodules(gitmodulesPath, QSettings::IniFormat);
        QStringList childGroups = gitmodules.childGroups();
        foreach(QString childGroup, childGroups) {
            if(childGroup.startsWith("submodule")) {
                gitmodules.beginGroup(childGroup);
                if(gitmodules.value("path").toString() == pod.name) {
                    gitmodules.setValue("pod", true);
                }
                gitmodules.endGroup();
            }
        }
    }

    generatePodsPri(repository);
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

        generatePodsPri(repository);
    }
}

void PodManager::updatePods(QString repository) {
    if(!isValidRepository(repository)) {
        return;
    }

    QDir cwd = QDir::current();
    QDir::setCurrent(repository);

    QProcess::execute(QString("git submodule foreach git pull"));

    QDir::setCurrent(cwd.absolutePath());

    generatePodsPri(repository);
}

QList<Pod> PodManager::installedPods(QString repository) {
    QList<Pod> pods;
    QDir dir(repository);
    QString gitmodulesPath = dir.filePath(".gitmodules");
    if(QFile::exists(gitmodulesPath)) {
        QSettings gitmodules(gitmodulesPath, QSettings::IniFormat);
        QStringList childGroups = gitmodules.childGroups();
        foreach(QString childGroup, childGroups) {
            if(childGroup.startsWith("submodule")) {
                gitmodules.beginGroup(childGroup);
                if(gitmodules.contains("pod")) {
                    Pod pod;
                    pod.name = gitmodules.value("path").toString();
                    pod.url  = gitmodules.value("url").toString();
                    pods.append(pod);
                }
                gitmodules.endGroup();
            }
        }
    }
    return pods;
}

QList<Pod> PodManager::availablePods(QStringList sources) {
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

void PodManager::generatePodsPri(QString repository) {
    QList<Pod> pods = installedPods(repository);
    QString header = "# Auto-generated. Do not edit.\n";
    QString includePath = "INCLUDEPATH += ";
    QString libs = "LIBS += ";
    QString includePris = "";
    foreach(Pod pod, pods) {
        includePath += QString("\\\n\t../%1 ").arg(pod.name);
        libs += QString("\\\n\t-L../%1 -l%1 ").arg(pod.name);
        includePris += QString("include(%1/%1.pri)\n").arg(pod.name);
    }

    QString podsPri = QString("%1\n%2\n\n%3\n\n%4\n")
        .arg(header)
        .arg(includePath)
        .arg(libs)
        .arg(includePris);

    QFile file(QDir(repository).filePath("pods.pri"));
    file.remove();
    if(file.open(QFile::ReadWrite)) {
        file.write(podsPri.toUtf8());
        file.close();
    }
}

bool PodManager::checkPod(QString repository, QString podName) {
    QDir dir(repository);
    return (podName == podName.toLower()) &&
        dir.cd(podName) &&
        QFile::exists(dir.filePath("LICENSE")) &&
        QFile::exists(dir.filePath("README.md")) &&
        QFile::exists(dir.filePath(podName + ".pri")) &&
        QFile::exists(dir.filePath(podName + ".pro"));
}

