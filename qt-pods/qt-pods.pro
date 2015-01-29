QT  += core gui network widgets

TARGET = qt-pods
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    podsmodel.cpp \
    podmanager.cpp

HEADERS += \
    mainwindow.h \
    podsmodel.h \
    podmanager.h

FORMS += mainwindow.ui

RESOURCES += \
    resources.qrc
