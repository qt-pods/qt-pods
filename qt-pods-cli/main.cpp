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
            QCoreApplication::translate("main", "Lists all available pods."));
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

//    const QStringList args = parser.positionalArguments();
//    // source is args.at(0), destination is args.at(1)

//    bool showProgress = parser.isSet(showProgressOption);
//    bool force = parser.isSet(forceOption);
//    QString targetDir = parser.value(targetDirectoryOption);
//    // ...

    parser.showHelp();
    return 0;
}
