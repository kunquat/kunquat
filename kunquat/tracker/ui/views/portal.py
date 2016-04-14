# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from .newbutton import NewButton
from .openbutton import OpenButton
from .savebutton import SaveButton
from .connectionsbutton import ConnectionsButton
from .songschannelsbutton import SongsChannelsButton
from .eventlistbutton import EventListButton
from .aboutbutton import AboutButton
from . import utils


class Portal(QToolBar):

    def __init__(self):
        super().__init__()
        self._new_button = NewButton()
        self._open_button = OpenButton()
        self._save_button = SaveButton()
        self._about_button = AboutButton()
        self._connections_button = ConnectionsButton()
        self._songs_channels_button = SongsChannelsButton()
        self._notation_button = NotationButton()
        self._env_bind_button = EnvBindButton()
        self._general_mod_button = GeneralModButton()
        self._event_list_button = EventListButton()
        self._render_stats_button = RenderStatsButton()

        self.addWidget(self._new_button)
        self.addWidget(self._open_button)
        self.addWidget(self._save_button)
        self.addSeparator()
        self.addWidget(self._connections_button)
        self.addWidget(self._songs_channels_button)
        self.addWidget(self._notation_button)
        self.addWidget(self._env_bind_button)
        self.addWidget(self._general_mod_button)
        self.addSeparator()
        self.addWidget(self._event_list_button)
        self.addWidget(self._render_stats_button)
        self.addSeparator()
        self.addWidget(self._about_button)

    def set_ui_model(self, ui_model):
        self._new_button.set_ui_model(ui_model)
        self._open_button.set_ui_model(ui_model)
        self._save_button.set_ui_model(ui_model)
        self._connections_button.set_ui_model(ui_model)
        self._songs_channels_button.set_ui_model(ui_model)
        self._notation_button.set_ui_model(ui_model)
        self._env_bind_button.set_ui_model(ui_model)
        self._general_mod_button.set_ui_model(ui_model)
        self._event_list_button.set_ui_model(ui_model)
        self._render_stats_button.set_ui_model(ui_model)
        self._about_button.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._about_button.unregister_updaters()
        self._event_list_button.unregister_updaters()
        self._render_stats_button.unregister_updaters()
        self._general_mod_button.unregister_updaters()
        self._env_bind_button.unregister_updaters()
        self._notation_button.unregister_updaters()
        self._songs_channels_button.unregister_updaters()
        self._connections_button.unregister_updaters()
        self._save_button.unregister_updaters()
        self._open_button.unregister_updaters()
        self._new_button.unregister_updaters()


class WindowOpenerButton(QToolButton):

    def __init__(self, text):
        super().__init__()
        self._ui_model = None

        self.setText(text)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        self._show_action(visibility_manager)

    # Protected interface

    def _show_action(self, visibility_manager):
        raise NotImplementedError


class EnvBindButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('Environment && bindings')

    def _show_action(self, visibility_manager):
        visibility_manager.show_env_and_bindings()


class NotationButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('Notations')

    def _show_action(self, visibility_manager):
        visibility_manager.show_notation_editor()


class GeneralModButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('General')

    def _show_action(self, visibility_manager):
        visibility_manager.show_general_module_settings()


_RENDER_LOAD_METER_CONFIG = {
        'width'         : 12,
        'height'        : 12,
        'colour_low'    : QColor(0x11, 0x99, 0x11),
        'colour_mid'    : QColor(0xdd, 0xcc, 0x33),
        'colour_high'   : QColor(0xee, 0x22, 0x11),
    }


class RenderLoadMeter(QWidget):

    def __init__(self):
        super().__init__()
        self._config = _RENDER_LOAD_METER_CONFIG.copy()
        self._load_norm = 0

    def set_load_norm(self, load_norm):
        self._load_norm = min(max(0.0, load_norm), 1.0)
        self.update()

    def _get_colour(self, norm):
        if norm < 0.5:
            lerp_val = norm * 2.0
            from_colour = self._config['colour_low']
            to_colour = self._config['colour_mid']
        else:
            lerp_val = (norm - 0.5) * 2.0
            from_colour = self._config['colour_mid']
            to_colour = self._config['colour_high']
        return utils.lerp_colour(from_colour, to_colour, lerp_val)

    def paintEvent(self, event):
        width = self._config['width']
        height = self._config['height']

        painter = QPainter(self)
        painter.setBackground(QColor(0, 0, 0))
        painter.eraseRect(0, 0, width, height)
        painter.setRenderHint(QPainter.Antialiasing)

        colour = self._get_colour(self._load_norm)
        bar_height = height * self._load_norm
        fill_rectf = QRectF(0, height - bar_height, width, bar_height)
        painter.fillRect(fill_rectf, colour)

    def sizeHint(self):
        return QSize(self._config['width'], self._config['height'])


class RenderStatsButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._stat_manager = None

        self._load_meter = RenderLoadMeter()

        h = QHBoxLayout()
        h.setContentsMargins(6, 6, 6, 6)
        h.addWidget(self._load_meter, 0, Qt.AlignVCenter)
        h.addWidget(QLabel('System load'))
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._stat_manager = ui_model.get_stat_manager()

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        self._load_meter.set_load_norm(self._stat_manager.get_render_load())

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_render_stats()

    def sizeHint(self):
        return self.layout().sizeHint()


