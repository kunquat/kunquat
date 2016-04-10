# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
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
from .logo import Logo


class AboutMessage(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        default_font = QFont()
        default_family = default_font.defaultFamily()
        default_size = default_font.pointSize()

        program_name = QLabel('Kunquat Tracker')
        program_name.setFont(QFont('DejaVu Sans', default_size + 9, QFont.DemiBold))

        tracker_version_str = 'Unreleased tracker version'
        if KUNQUAT_VERSION:
            tracker_version_str = 'Tracker version: {}'.format(KUNQUAT_VERSION)
        tracker_version = QLabel(tracker_version_str)
        tracker_version.setFont(QFont(default_family, default_size + 1, QFont.DemiBold))

        lib_version_str = 'Library version: {}'.format(get_version())
        lib_version = QLabel(lib_version_str)

        website_str = '<a href="http://kunquat.org/">http://kunquat.org/</a>'
        website = QLabel(website_str)
        website.setTextFormat(Qt.RichText)

        copyright_str = 'CC0 1.0 Universal'
        copyright = QLabel(copyright_str)

        main_authors = QLabel('Main design and programming:')
        main_authors.setFont(QFont(default_family, default_size, QFont.DemiBold))

        main_authors_list_str = u'\n'.join((
            u'Tomi Jylhä-Ollila',
            u'Toni Ruottu',
            ))
        main_authors_list = QLabel(main_authors_list_str)

        add_authors = QLabel('Additional contributors:')
        add_authors.setFont(QFont(default_family, default_size, QFont.DemiBold))

        add_authors_list_str = u'\n'.join((
            u'Ossi Saresoja',
            u'Sami Ketola',
            ))
        add_authors_list = QLabel(add_authors_list_str)

        for label in (
                program_name, tracker_version, lib_version, website, copyright,
                main_authors, main_authors_list, add_authors, add_authors_list):
            label.setAlignment(Qt.AlignHCenter)

        v = QVBoxLayout()
        v.setContentsMargins(8, 0, 8, 8)
        v.addWidget(program_name)
        v.addWidget(tracker_version)
        v.addWidget(lib_version)
        v.addSpacing(8)
        v.addWidget(website)
        v.addSpacing(8)
        v.addWidget(copyright)
        v.addSpacing(8)
        v.addWidget(main_authors)
        v.addWidget(main_authors_list)
        v.addWidget(add_authors)
        v.addWidget(add_authors_list)
        v.addStretch(1)
        self.setLayout(v)


class About(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._logo = Logo()
        self._about_message = AboutMessage()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
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


