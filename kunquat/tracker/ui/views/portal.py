# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2017
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.kunquat.limits import *
from .eventlistbutton import EventListButton
from .exiting import ExitHelper
from . import utils
from .kqtutils import try_open_kqt_module_or_au
from .saving import try_save_module
from .updater import Updater


class Portal(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._file_button = FileButton()
        self._about_button = AboutButton()
        self._connections_button = ConnectionsButton()
        self._songs_channels_button = SongsChannelsButton()
        self._notation_button = NotationButton()
        self._env_bind_button = EnvBindButton()
        self._general_mod_button = GeneralModButton()
        self._settings_button = SettingsButton()
        self._event_list_button = EventListButton()
        self._render_stats_button = RenderStatsButton()

        self.add_to_updaters(
                self._file_button,
                self._connections_button,
                self._songs_channels_button,
                self._notation_button,
                self._env_bind_button,
                self._general_mod_button,
                self._event_list_button,
                self._render_stats_button,
                self._settings_button,
                self._about_button)

        self.addWidget(self._file_button)
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
        self.addWidget(self._settings_button)
        self.addWidget(self._about_button)

    def addWidget(self, button):
        button.setFocusPolicy(Qt.NoFocus)
        super().addWidget(button)


class FileButton(QToolButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('File')
        self.setPopupMode(QToolButton.InstantPopup)

        self._module_loaded = False

        self._exit_helper = ExitHelper()

        menu = QMenu()

        self._new_action = QAction(menu)
        self._new_action.setText('New')
        self._new_action.setShortcut(Qt.CTRL + Qt.Key_N)

        self._open_action = QAction(menu)
        self._open_action.setText('Open...')
        self._open_action.setShortcut(Qt.CTRL + Qt.Key_O)

        self._save_action = QAction(menu)
        self._save_action.setText('Save')
        self._save_action.setShortcut(Qt.CTRL + Qt.Key_S)

        self._save_as_action = QAction(menu)
        self._save_as_action.setText('Save as...')

        self._quit_action = QAction(menu)
        self._quit_action.setText('Quit')

        menu.addAction(self._new_action)
        menu.addAction(self._open_action)
        menu.addAction(self._save_action)
        menu.addAction(self._save_as_action)
        menu.addSeparator()
        menu.addAction(self._quit_action)

        self.setMenu(menu)

    def _on_setup(self):
        self.register_action('signal_module', self._set_module_loaded)
        self.register_action('signal_change', self._update_save_enabled)
        self.register_action(
                'signal_save_module_finished', self._on_save_module_finished)

        self._exit_helper.set_ui_model(self._ui_model)

        self._new_action.triggered.connect(self._new)
        self._open_action.triggered.connect(self._open)
        self._save_action.triggered.connect(self._save)
        self._save_as_action.triggered.connect(self._save_as)
        self._quit_action.triggered.connect(self._quit)

    def _set_module_loaded(self):
        self._module_loaded = True

    def _update_save_enabled(self):
        if self._module_loaded:
            module = self._ui_model.get_module()
            self._save_action.setEnabled(module.is_modified())

    def _on_save_module_finished(self):
        self._exit_helper.notify_save_module_finished()

    def _new(self):
        process_manager = self._ui_model.get_process_manager()
        process_manager.new_kunquat()

    def _open(self):
        try_open_kqt_module_or_au(self._ui_model)

    def _save(self):
        try_save_module(self._ui_model)

    def _save_as(self):
        try_save_module(self._ui_model, save_as=True)

    def _quit(self):
        self._exit_helper.try_exit()


class WindowOpenerButton(QToolButton, Updater):

    def __init__(self, text):
        super().__init__()
        self.setText(text)

    def _on_setup(self):
        self.clicked.connect(self._clicked)

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        self._show_action(visibility_manager)

    # Protected interface

    def _show_action(self, visibility_manager):
        raise NotImplementedError


class ConnectionsButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('Connections')

    def _show_action(self, visibility_manager):
        visibility_manager.show_connections()


class SongsChannelsButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('Songs && channels')

    def _show_action(self, visibility_manager):
        visibility_manager.show_songs_channels()


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


class SettingsButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('Settings')

    def _show_action(self, visibility_manager):
        visibility_manager.show_settings()


class AboutButton(WindowOpenerButton):

    def __init__(self):
        super().__init__('About')

    def _show_action(self, visibility_manager):
        visibility_manager.show_about()


_RENDER_LOAD_METER_CONFIG = {
        'width'         : 12,
        'height'        : 12,
        'bg_colour'     : QColor(0, 0, 0),
        'colour_low'    : QColor(0x11, 0x99, 0x11),
        'colour_mid'    : QColor(0xdd, 0xcc, 0x33),
        'colour_high'   : QColor(0xee, 0x22, 0x11),
    }


class RenderLoadMeter(QWidget):

    def __init__(self):
        super().__init__()
        self._config = None
        self._load_norm = 0

    def set_config(self, config):
        self._config = _RENDER_LOAD_METER_CONFIG.copy()
        self._config.update(config)
        self.update()

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
        painter.setBackground(self._config['bg_colour'])
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

        self.clicked.connect(self._clicked)

        self._update_style()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        self._load_meter.set_load_norm(self._stat_manager.get_render_load())

        if 'signal_style_changed' in signals:
            self._update_style()

    def _update_style(self):
        style_manager = self._ui_model.get_style_manager()
        if not style_manager.is_custom_style_enabled():
            self._load_meter.set_config({})
            return

        def get_colour(param):
            return QColor(style_manager.get_style_param(param))

        config = {
            'bg_colour': get_colour('system_load_bg_colour'),
            'colour_low': get_colour('system_load_low_colour'),
            'colour_mid': get_colour('system_load_mid_colour'),
            'colour_high': get_colour('system_load_high_colour'),
        }

        self._load_meter.set_config(config)

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_render_stats()

    def sizeHint(self):
        return self.layout().sizeHint()


