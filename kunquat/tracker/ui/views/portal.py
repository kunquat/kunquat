# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013-2014
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

from playbutton import PlayButton
from silencebutton import SilenceButton
from eventlistbutton import EventListButton
from aboutbutton import AboutButton


class Portal(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._ui_model = None
        self._play_button = PlayButton()
        self._silence_button = SilenceButton()
        self._event_list_button = EventListButton()
        self._about_button = AboutButton()

        self.addWidget(self._play_button)
        self.addWidget(self._silence_button)
        self.addSeparator()
        self.addWidget(self._event_list_button)
        self.addSeparator()
        self.addWidget(self._about_button)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._play_button.set_ui_model(ui_model)
        self._silence_button.set_ui_model(ui_model)
        self._event_list_button.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._about_button.unregister_updaters()
        self._event_list_button.unregister_updaters()
        self._silence_button.unregister_updaters()
        self._play_button.unregister_updaters()

