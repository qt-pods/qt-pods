TEMPLATE = subdirs
include(pods-subdirs.pri)
SUBDIRS += \
    qt-pods-gui \
    qt-pods-cli

qt-pods-cli.depends = qt-pods-core qtwaitingspinner
qt-pods-gui.depends = qt-pods-core qtwaitingspinner
