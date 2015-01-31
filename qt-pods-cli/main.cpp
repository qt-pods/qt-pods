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

// Qt includes
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QRegExp>

// Own includes
#include "podmanager.h"

// Standard includes
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("qt-pods-cli");
    QCoreApplication::setApplicationVersion(GIT_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
                "Copyright (C) 2015 Jacob Dawid, jacob@omg-it.works\n"
                "This is free software; see the source code for copying conditions.\n"
                "There is ABSOLUTELY NO WARRANTY; not even for MERCHANTABILITY or\n"
                "FITNESS FOR A PARTICULAR PURPOSE.");
    parser.addHelpOption();
    parser.addVersionOption();
//    parser.addPositionalArgument("command", QCoreApplication::translate("main", "Source file to copy."));
//    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    QCommandLineOption listOption(QStringList() << "l" << "list",
            QCoreApplication::translate("main", "Lists installed pods."));
    parser.addOption(listOption);

    QCommandLineOption searchOption(QStringList() << "s" << "search",
            QCoreApplication::translate("main", "Searches pods that match <wildcard>."),
            QCoreApplication::translate("main", "wildcard"));
    parser.addOption(searchOption);

    QCommandLineOption installOption(QStringList() << "i" << "install",
            QCoreApplication::translate("main", "Installs the pod <podname>."),
            QCoreApplication::translate("main", "podname"));
    parser.addOption(installOption);

    QCommandLineOption removeOption(QStringList() << "r" << "remove",
            QCoreApplication::translate("main", "Removes the pod <podname>."),
            QCoreApplication::translate("main", "podname"));
    parser.addOption(removeOption);

    QCommandLineOption updateOption(QStringList() << "u" << "update",
            QCoreApplication::translate("main", "Updates the pod <podname>."),
            QCoreApplication::translate("main", "podname"));
    parser.addOption(updateOption);

    QCommandLineOption checkOption(QStringList() << "c" << "check",
            QCoreApplication::translate("main", "Checks <podname>, if it complies to the pod rules."),
            QCoreApplication::translate("main", "podname"));
    parser.addOption(checkOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    QStringList sources;
    sources << "https://raw.githubusercontent.com/cybercatalyst/qt-pods-master/master/pods.json";

    PodManager podManager;
    QString repositoryPath = QDir::current().absolutePath();
    if(!podManager.isGitRepository(repositoryPath)) {
        std::cerr << "The current repository is not a git repository." << std::endl;
        return 1;
    } else
    if(parser.isSet(listOption)) {
        QList<Pod> pods = podManager.installedPods(repositoryPath);
        foreach(Pod pod, pods) {
            std::cout << pod.name.toStdString() << std::endl;
        }
    } else
    if(parser.isSet(searchOption)) {
        QList<Pod> pods = podManager.availablePods(sources);
        std::cout << "The following pods are available and match your criteria:" << std::endl;
        QRegExp regExp(parser.value(searchOption), Qt::CaseInsensitive, QRegExp::Wildcard);
        foreach(Pod pod, pods) {
            if(pod.name.contains(regExp)) {
                std::cout << pod.name.toStdString() << std::endl;
            }
        }
    } else
    if(parser.isSet(installOption)) {
        QList<Pod> pods = podManager.availablePods(sources);
        foreach(Pod pod, pods) {
            if(pod.name == parser.value(installOption)) {
                bool result = podManager.installPod(repositoryPath, pod);
                if(result) {
                    std::cout << "Pod \"" << pod.name.toStdString() << "\" installed successfully." << std::endl;
                    return 0;
                } else {
                    std::cout << "Installing pod failed." << std::endl;
                    return 2;
                }
            }
        }

        std::cout << "Pod \"" << parser.value(installOption).toStdString() << "\" not found." << std::endl;
        return 3;
    } else
    if(parser.isSet(removeOption)) {
        QList<Pod> pods = podManager.installedPods(repositoryPath);
        foreach(Pod pod, pods) {
            if(pod.name == parser.value(removeOption)) {
                bool result = podManager.removePod(repositoryPath, pod.name);
                if(result) {
                    std::cout << "Pod \"" << pod.name.toStdString() << "\" removed successfully." << std::endl;
                    return 0;
                } else {
                    std::cout << "Removing pod failed." << std::endl;
                    return 2;
                }
            }
        }
        std::cout << "Pod \"" << parser.value(installOption).toStdString() << "\" not found." << std::endl;
        return 3;
    } else
    if(parser.isSet(updateOption)) {
        QList<Pod> pods = podManager.installedPods(repositoryPath);
        foreach(Pod pod, pods) {
            if(pod.name == parser.value(updateOption)) {
                bool result = podManager.updatePod(repositoryPath, pod.name);
                if(result) {
                    std::cout << "Pod \"" << pod.name.toStdString() << "\" updated successfully." << std::endl;
                    return 0;
                } else {
                    std::cout << "Updating pod failed." << std::endl;
                    return 2;
                }
            }
        }
        std::cout << "Pod \"" << parser.value(installOption).toStdString() << "\" not found." << std::endl;
        return 3;
    } else
    if(parser.isSet(checkOption)) {
        QList<Pod> pods = podManager.installedPods(repositoryPath);
        foreach(Pod pod, pods) {
            if(pod.name == parser.value(checkOption)) {
                bool result = podManager.checkPod(repositoryPath, pod.name);
                if(result) {
                    std::cout << "Pod \"" << pod.name.toStdString() << "\" is valid." << std::endl;
                    return 0;
                } else {
                    std::cout << "Pod seems not to be valid." << std::endl;
                    return 2;
                }
            }
        }
        std::cout << "Pod \"" << parser.value(installOption).toStdString() << "\" not found." << std::endl;
        return 3;
    } else
    parser.showHelp();
    return 0;
}
