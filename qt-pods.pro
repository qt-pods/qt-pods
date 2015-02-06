TEMPLATE = subdirs
SUBDIRS = \
    qt-pods-gui \
    qt-pods-cli
include(pods-subdirs.pri)

qt-pods-cli.depends = qt-pods-core
qt-pods-gui.depends = qt-pods-core
