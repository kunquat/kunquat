# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2018
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.version import KUNQUAT_VERSION
from kunquat.kunquat.kunquat import get_version
from .logo import Logo
from .updater import Updater


class AboutMessage(QWidget):

    def __init__(self):
        super().__init__()

        # Font sizes don't work correctly with QLabels :-(
        program_name = QLabel('<h2>Kunquat Tracker</h2>')
        program_name.setTextFormat(Qt.RichText)

        tracker_version_str = 'Unreleased tracker version'
        if KUNQUAT_VERSION:
            tracker_version_str = 'Tracker version: {}'.format(KUNQUAT_VERSION)
        tracker_version_str = '<strong>' + tracker_version_str + '</strong>'
        tracker_version = QLabel(tracker_version_str)
        tracker_version.setTextFormat(Qt.RichText)

        lib_version_str = 'Library version: {}'.format(get_version())
        lib_version = QLabel(lib_version_str)

        website_str = self._get_website_str()
        self._website = QLabel(website_str)
        self._website.setTextFormat(Qt.RichText)

        copyright_str = 'CC0 1.0 Universal'
        copyright = QLabel(copyright_str)

        main_authors = QLabel('<strong>Main design and programming:</strong>')
        main_authors.setTextFormat(Qt.RichText)

        main_authors_list_str = '\n'.join((
            'Tomi Jylhä-Ollila',
            'Toni Ruottu',
            ))
        main_authors_list = QLabel(main_authors_list_str)

        add_authors = QLabel('<strong>Additional contributors:</strong>')
        add_authors.setTextFormat(Qt.RichText)

        add_authors_list_str = '\n'.join((
            'Ossi Saresoja',
            'Sami Koistinen',
            'Sami Ketola',
            ))
        add_authors_list = QLabel(add_authors_list_str)

        for label in (
                program_name, tracker_version, lib_version, self._website, copyright,
                main_authors, main_authors_list, add_authors, add_authors_list):
            label.setAlignment(Qt.AlignHCenter)

        v = QVBoxLayout()
        v.setContentsMargins(8, 0, 8, 8)
        v.addWidget(program_name)
        v.addWidget(tracker_version)
        v.addWidget(lib_version)
        v.addSpacing(8)
        v.addWidget(self._website)
        v.addSpacing(8)
        v.addWidget(copyright)
        v.addSpacing(8)
        v.addWidget(main_authors)
        v.addWidget(main_authors_list)
        v.addWidget(add_authors)
        v.addWidget(add_authors_list)
        v.addStretch(1)
        self.setLayout(v)

    def update_style(self, style_mgr):
        website_str = self._get_website_str(style_mgr)
        self._website.setText(website_str)
        self.update()

    def _get_website_str(self, style_mgr=None):
        website_base = '<a{} href="http://kunquat.org/">http://kunquat.org/</a>'
        style = ''

        if style_mgr:
            colour = style_mgr.get_link_colour()
            style = ' style="color: {};"'.format(colour)

        return website_base.format(style)


class About(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._logo = Logo()
        self._about_message = AboutMessage()

        self.add_to_updaters(self._logo)
        self.register_action('signal_style_changed', self._update_style)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.setAlignment(Qt.AlignHCenter)
        v.addWidget(self._logo)
        v.setAlignment(self._logo, Qt.AlignHCenter)
        v.addWidget(self._about_message)
        v.setAlignment(self._about_message, Qt.AlignHCenter)
        self.setLayout(v)

    def _on_setup(self):
        self._about_message.update_style(self._ui_model.get_style_manager())

    def _update_style(self):
        self._about_message.update_style(self._ui_model.get_style_manager())


