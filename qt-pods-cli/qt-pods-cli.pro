##############################################################################/
##                                                                           ##
##    This file is part of qt-pods.                                          ##
##    Copyright (C) 2015 Jacob Dawid, jacob@omg-it.works                     ##
##                                                                           ##
##    qt-pods is free software: you can redistribute it and/or modify        ##
##    it under the terms of the GNU General Public License as published by   ##
##    the Free Software Foundation, either version 3 of the License, or      ##
##    (at your option) any later version.                                    ##
##                                                                           ##
##    qt-pods is distributed in the hope that it will be useful,             ##
##    but WITHOUT ANY WARRANTY; without even the implied warranty of         ##
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          ##
##    GNU General Public License for more details.                           ##
##                                                                           ##
##    You should have received a copy of the GNU General Public License      ##
##    along with qt-pods. If not, see <http://www.gnu.org/licenses/>.        ##
##                                                                           ##
##############################################################################/

QT       += core network

QT       -= gui

TARGET = qt-pods-cli
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

include(../pods.pri)

GIT_VERSION = $$system(git describe --always --dirty --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

# make install
unix {
        # variables
        OBJECTS_DIR = .obj
        MOC_DIR     = .moc
        UI_DIR      = .ui

        isEmpty(PREFIX) {
                PREFIX = /usr/local
        }

        BINDIR = $$PREFIX/bin
        DATADIR = $$PREFIX/share
        LOCALEDIR = $(localedir)

        DEFINES += DATADIR=\"$$DATADIR\"

        !isEmpty(LOCALEDIR) {
                DEFINES += LOCALEDIR=\"$$LOCALEDIR\"
        }

        # make install
        INSTALLS += target desktop icon

        target.path = $$BINDIR

        desktop.path = $$DATADIR/applications
        desktop.files += $${TARGET}.desktop

        icon.path = $$DATADIR/icons/hicolor/32x32/apps
        icon.files += images/$${TARGET}.png
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qt-pods-core/release/ -lqt-pods-core
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qt-pods-core/debug/ -lqt-pods-core
else:unix: LIBS += -L$$OUT_PWD/../qt-pods-core/ -lqt-pods-core

INCLUDEPATH += $$PWD/../qt-pods-core
DEPENDPATH += $$PWD/../qt-pods-core

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt-pods-core/release/libqt-pods-core.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt-pods-core/debug/libqt-pods-core.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt-pods-core/release/qt-pods-core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qt-pods-core/debug/qt-pods-core.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../qt-pods-core/libqt-pods-core.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qtwaitingspinner/release/ -lqtwaitingspinner
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qtwaitingspinner/debug/ -lqtwaitingspinner
else:unix: LIBS += -L$$OUT_PWD/../qtwaitingspinner/ -lqtwaitingspinner

INCLUDEPATH += $$PWD/../qtwaitingspinner
DEPENDPATH += $$PWD/../qtwaitingspinner

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtwaitingspinner/release/libqtwaitingspinner.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtwaitingspinner/debug/libqtwaitingspinner.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtwaitingspinner/release/qtwaitingspinner.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../qtwaitingspinner/debug/qtwaitingspinner.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../qtwaitingspinner/libqtwaitingspinner.a
