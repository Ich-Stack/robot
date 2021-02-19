QT       += core gui
QT       += webenginewidgets
QT       += multimedia
QT       += webchannel
QT       += serialport
QT       += script
QT       += network
QT       += websockets

RC_ICONS = 64.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    algorithm/astar.cpp \
    algorithm/cs.cpp \
    com/comm.cpp \
    mycontral/mybtn.cpp \
    mycontral/myframe.cpp \
    mycontral/mylabel.cpp \
    state.cpp \
    widget/contral.cpp \
    widget/dialog.cpp \
    widget/edit.cpp \
    widget/init.cpp \
    widget/mainwidget.cpp \
    widget/videowidget.cpp

HEADERS += \
    algorithm/astar.h \
    algorithm/cs.h \
    algorithm/minheap.h \
    com/comm.h \
    mycontral/mybtn.h \
    mycontral/myframe.h \
    mycontral/mylabel.h \
    sdk/HCNetSDK.h \
    state.h \
    widget/contral.h \
    widget/dialog.h \
    widget/edit.h \
    widget/init.h \
    widget/mainwidget.h \
    widget/videowidget.h \

FORMS += \
    contral.ui \
    dialog.ui \
    edit.ui \
    init.ui \
    mainwidget.ui \
    state.ui \
    videowidget.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES +=

win32: LIBS += -L$$PWD/lib/ -lHCNetSDK

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
