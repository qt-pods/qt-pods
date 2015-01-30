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

    generatePodsPri(repository);
    generatePodsSubdirsPri(repository);
    generateSubdirsPro(repository);
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
        generatePodsSubdirsPri(repository);
        generateSubdirsPro(repository);
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
    generatePodsSubdirsPri(repository);
    generateSubdirsPro(repository);
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

QList<Pod> PodManager::availablePods(QStringList sources) {
    if(_networkAccessManager.networkAccessible() == QNetworkAccessManager::NotAccessible) {
        qDebug() << "No network connection available.";
        return QList<Pod>();
    }

    qDebug() << "Updating available pods.";
    QList<Pod> pods;
    foreach(QString source, sources) {
        qDebug() << "Updating from source: " << source;

        QNetworkRequest request;
        request.setUrl(QUrl(source));
        QNetworkReply *reply = _networkAccessManager.get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        QByteArray response = reply->readAll();

        if(reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
        } else {
            qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
        }

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
                qDebug() << "Found pod: " << pod.name;
            }
        } else {
            qDebug() << source << " is a malformed source: " << parseError.errorString();
        }
    }
    return pods;
}

void PodManager::generatePodsPri(QString repository) {
    QList<Pod> pods = installedPods(repository);
    QString header = QString("# Auto-generated by qt-pods @%1. Do not edit.\n# Include this to your application project file with:\n# include(../pods.pri)\n").arg(QDateTime::currentDateTime().toString());
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

void PodManager::generatePodsSubdirsPri(QString repository) {
    QList<Pod> pods = installedPods(repository);
    QString header = QString("# Auto-generated by qt-pods @%1. Do not edit.\n# Include this to your subdirs project file with:\n# include(pods-subdirs.pri)\n").arg(QDateTime::currentDateTime().toString());
    QString subdirs = "SUBDIRS += ";

    foreach(Pod pod, pods) {
        subdirs += QString("\\\n\t%1 ").arg(pod.name);
    }

    QString podsSubdirsPri = QString("%1\n%2\n\n")
        .arg(header)
        .arg(subdirs);

    QFile file(QDir(repository).filePath("pods-subdirs.pri"));
    file.remove();
    if(file.open(QFile::ReadWrite)) {
        file.write(podsSubdirsPri.toUtf8());
        file.close();
    }
}

void PodManager::generateSubdirsPro(QString repository) {
    QDir dir(repository);
    QFile file(QDir(repository).filePath(QString("%1.pro").arg(dir.dirName())));
    // Just create one if it doesn't exist yet.
    if(!file.exists()) {
        if(file.open(QFile::ReadWrite)) {
            QString subdirsPro = "TEMPLATE = subdirs\nSUBDIRS =\ninclude(pods-subdirs.pri)\n";
            file.write(subdirsPro.toUtf8());
            file.close();
        }
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

