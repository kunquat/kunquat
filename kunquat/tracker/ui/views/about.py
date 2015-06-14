# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2015
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.version import KUNQUAT_VERSION
from kunquat.kunquat.kunquat import get_version
from logo import Logo


class AboutMessage(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        font_family = QFont().defaultFamily()

        program_name = QLabel('Kunquat Tracker')
        program_name.setFont(QFont('DejaVu Sans', 20, QFont.DemiBold))

        tracker_version_str = 'Unreleased tracker version'
        if KUNQUAT_VERSION:
            tracker_version_str = 'Tracker version {}'.format(KUNQUAT_VERSION)
        tracker_version = QLabel(tracker_version_str)

        lib_version_str = 'Library version: {}'.format(get_version())
        lib_version = QLabel(lib_version_str)

        website_str = '<a href="http://kunquat.org/">http://kunquat.org/</a>'
        website = QLabel(website_str)
        website.setTextFormat(Qt.RichText)

        copyright_str = 'CC0 1.0 Universal'
        copyright = QLabel(copyright_str)

        for label in (program_name, tracker_version, lib_version, website, copyright):
            label.setAlignment(Qt.AlignHCenter)

        v = QVBoxLayout()
        v.addWidget(program_name)
        v.addWidget(tracker_version)
        v.addWidget(lib_version)
        v.addSpacing(8)
        v.addWidget(website)
        v.addSpacing(8)
        v.addWidget(copyright)
        v.addStretch(1)
        self.setLayout(v)


class About(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._logo = Logo()
        self._about_message = AboutMessage()

        v = QVBoxLayout()
        v.setAlignment(Qt.AlignHCenter)
        v.addWidget(self._logo)
        v.setAlignment(self._logo, Qt.AlignHCenter)
        v.addWidget(self._about_message)
        v.setAlignment(self._about_message, Qt.AlignHCenter)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._logo.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._logo.unregister_updaters()


