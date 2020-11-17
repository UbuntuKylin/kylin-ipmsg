# DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
# Copyright (C) 2011 Emanuele Colombo
#               2020 KylinSoft Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

QT += quick qml

SOURCES += $$PWD/qmlapplicationviewer.cpp
HEADERS += $$PWD/qmlapplicationviewer.h
INCLUDEPATH += $$PWD

contains(CONFIG,qdeclarative-boostable):contains(MEEGO_EDITION,harmattan) {
    DEFINES += HARMATTAN_BOOSTER
}

defineTest(qtcAddDeployment) {
for(deploymentfolder, DEPLOYMENTFOLDERS) {
    item = item$${deploymentfolder}
    greaterThan(QT_MAJOR_VERSION, 4) {
        itemsources = $${item}.files
    } else {
        itemsources = $${item}.sources
    }
    $$itemsources = $$eval($${deploymentfolder}.source)
    itempath = $${item}.path
    $$itempath= $$eval($${deploymentfolder}.target)
    export($$itemsources)
    export($$itempath)
    DEPLOYMENT += $$item
}

MAINPROFILEPWD = $$PWD

copyCommand =
for(deploymentfolder, DEPLOYMENTFOLDERS) {
    source = $$MAINPROFILEPWD/$$eval($${deploymentfolder}.source)
    source = $$replace(source, \\\\, /)
    macx {
        target = $$OUT_PWD/$${TARGET}.app/Contents/Resources/$$eval($${deploymentfolder}.target)
    } else {
        target = $$OUT_PWD/$$eval($${deploymentfolder}.target)
    }
    target = $$replace(target, \\\\, /)
    sourcePathSegments = $$split(source, /)
    targetFullPath = $$target/$$last(sourcePathSegments)
    targetFullPath ~= s,/\\.?/,/,
    !isEqual(source,$$targetFullPath) {
        !isEmpty(copyCommand):copyCommand += &&
        copyCommand += $(MKDIR) \"$$target\"
        copyCommand += && $(COPY_DIR) \"$$source\" \"$$target\"
    }
}
!isEmpty(copyCommand) {
    copyCommand = @echo Copying application data... && $$copyCommand
    copydeploymentfolders.commands = $$copyCommand
    first.depends = $(first) copydeploymentfolders
    export(first.depends)
    export(copydeploymentfolders.commands)
    QMAKE_EXTRA_TARGETS += first copydeploymentfolders
}
installPrefix = /opt/$${TARGET}
for(deploymentfolder, DEPLOYMENTFOLDERS) {
    item = item$${deploymentfolder}
    itemfiles = $${item}.files
    $$itemfiles = $$eval($${deploymentfolder}.source)
    itempath = $${item}.path
    $$itempath = $${installPrefix}/$$eval($${deploymentfolder}.target)
    export($$itemfiles)
    export($$itempath)
    INSTALLS += $$item
}

!isEmpty(desktopfile.path) {
    export(icon.files)
    export(icon.path)
    export(desktopfile.files)
    export(desktopfile.path)
    INSTALLS += icon desktopfile
}

target.path = $${installPrefix}/bin
export(target.path)
INSTALLS += target

export (ICON)
export (INSTALLS)
export (DEPLOYMENT)
export (LIBS)
export (QMAKE_EXTRA_TARGETS)
}
