#-------------------------------------------------
#
# Project created by QtCreator 2019-04-08T14:00:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AudioPro
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        project.cpp \
    audioapp.cpp \
    pluginmanager.cpp \
    modulemanager.cpp \
    loadeffects.cpp \
    amplify.cpp \
    audioio.cpp \
    track.cpp \
    overlaypanel.cpp \
    backedpanel.cpp \
    overlay.cpp \
    commandmanager.cpp \
    importraw.cpp \
    multiformatreader.cpp \
    SampleFormat.cpp \
    SpecPowerMeter.cpp \
    FFT.cpp \
    FormatClassifier.cpp \
    importrawdialog.cpp \
    FileFormats.cpp \
    WaveTrack.cpp \
    XMLTagHandler.cpp \
    DirManager.cpp \
    XMLWriter.cpp \
    WaveClip.cpp \
    ViewInfo.cpp \
    Sequence.cpp \
    Envelope.cpp \
    BlockFile.cpp \
    SampleFormat.cpp \
    SimpleBlockFile.cpp \
    CommonTrackPanelCell.cpp \
    TrackPanel.cpp \
    Dither.cpp \
    effect.cpp \
    EffectManager.cpp \
    SelectedRegion.cpp \
    amplifydialog.cpp \
    amplifyform.cpp

HEADERS  += project.h \
    audioapp.h \
    pluginmanager.h \
    modulemanager.h \
    moduleinterface.h \
    componentinterface.h \
    types.h \
    effectinterface.h \
    loadeffects.h \
    effect.h \
    plugininterface.h \
    shuttle.h \
    noisereduction.h \
    memoryx.h \
    audioio.h \
    track.h \
    trackpanelcell.h \
    commontrackpanelcell.h \
    panelwrapper.h \
    backedpanel.h \
    trackpanel.h \
    overlaypanel.h \
    overlay.h \
    cellularpanel.h \
    commandmanager.h \
    importraw.h \
    formatclassifier.h \
    multiformatreader.h \
    specpowermeter.h \
    SampleFormat.h \
    FFT.h \
    importrawdialog.h \
    FileFormats.h \
    WaveTrack.h \
    XMLTagHandler.h \
    DirManager.h \
    XMLWriter.h \
    WaveClip.h \
    ViewInfo.h \
    Sequence.h \
    Envelope.h \
    BlockFile.h \
    SampleFormat.h \
    SimpleBlockFile.h \
    amplify.h \
    Dither.h \
    EffectManager.h \
    SelectedRegion.h \
    amplifydialog.h \
    amplifyform.h

FORMS    += project.ui \
    importrawdialog.ui \
    amplifydialog.ui \
    amplifyform.ui

LIBS += -Le:/GithubProject/AudioPro/libsndfile/libs -llibsndfile

INCLUDEPATH = e:/GithubProject/AudioPro/libsndfile
