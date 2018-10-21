# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
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
import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.model.trigger import Trigger
from kunquat.tracker.ui.model.triggerposition import TriggerPosition
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from kunquat.tracker.ui.views.updater import Updater
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from . import utils


class Toolbar(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._follow_playback_button = FollowPlaybackButton()
        self._edit_button = EditButton()
        self._replace_button = ReplaceButton()
        self._rest_button = RestButton()
        self._del_selection_button = DelSelectionButton()
        self._undo_button = UndoButton()
        self._redo_button = RedoButton()
        self._cut_button = CutButton()
        self._copy_button = CopyButton()
        self._paste_button = PasteButton()
        self._convert_tr_button = ConvertTriggerButton()
        self._zoom_buttons = [
                ZoomButton('out'),
                ZoomButton('original'),
                ZoomButton('in'),
                ZoomButton('shrink_w'),
                ZoomButton('original_w'),
                ZoomButton('expand_w'),
            ]
        self._grid_toggle = GridToggle()
        self._grid_editor_button = GridEditorButton()
        self._grid_selector = GridSelector()
        self._length_editor = LengthEditor()

        self.add_to_updaters(
                self._follow_playback_button,
                self._edit_button,
                self._replace_button,
                self._rest_button,
                self._del_selection_button,
                self._undo_button,
                self._redo_button,
                self._cut_button,
                self._copy_button,
                self._paste_button,
                self._convert_tr_button,
                *self._zoom_buttons,
                self._grid_toggle,
                self._grid_editor_button,
                self._grid_selector,
                self._length_editor)

        h = QHBoxLayout()
        h.setContentsMargins(2, 0, 2, 2)
        h.setSpacing(2)

        h.addWidget(self._follow_playback_button)
        h.addWidget(self._edit_button)
        h.addWidget(self._replace_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._rest_button)
        if cmdline.get_experimental():
            h.addWidget(self._del_selection_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._undo_button)
        h.addWidget(self._redo_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._cut_button)
        h.addWidget(self._copy_button)
        h.addWidget(self._paste_button)
        h.addWidget(HackSeparator())
        h.addWidget(self._convert_tr_button)
        h.addWidget(HackSeparator())
        for button in self._zoom_buttons:
            h.addWidget(button)

        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        h.addWidget(spacer)

        h.addWidget(self._grid_toggle)
        h.addSpacing(2)
        h.addWidget(self._grid_editor_button)
        h.addSpacing(2)
        h.addWidget(self._grid_selector)
        h.addWidget(self._length_editor)

        for i in range(h.count()):
            widget = h.itemAt(i).widget()
            if isinstance(widget, (QPushButton, QCheckBox)):
                widget.setFocusPolicy(Qt.NoFocus)

        self.setLayout(h)


_ICON_SIZE = QSize(24, 24)


class FollowPlaybackButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self.setCheckable(True)
        self.setFlat(True)
        self.setToolTip('Follow playback')

    def _on_setup(self):
        self.register_action('signal_follow_playback', self._update_playback_following)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('follow_playback')
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._toggle_playback_following)

        self._update_playback_following()

    def _update_playback_following(self):
        playback_mgr = self._ui_model.get_playback_manager()
        old_block = self.blockSignals(True)
        self.setChecked(playback_mgr.get_playback_cursor_following())
        self.blockSignals(old_block)

    def _toggle_playback_following(self):
        is_enabled = self.isChecked()
        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_playback_cursor_following(is_enabled)

        if not is_enabled and playback_mgr.is_playback_active():
            track_num, system_num, row_ts = playback_mgr.get_playback_position()
            if track_num < 0 or system_num < 0:
                ploc = utils.get_current_playback_pattern_location(self._ui_model)
                if ploc:
                    track_num, system_num = ploc

            if track_num >= 0 and system_num >= 0:
                selection = self._ui_model.get_selection()
                edit_location = selection.get_location()
                col_num = edit_location.get_col_num()
                new_location = TriggerPosition(track_num, system_num, col_num, row_ts, 0)
                selection.set_location(new_location)

        self._updater.signal_update('signal_follow_playback')


class EditButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None

        self.setCheckable(True)
        self.setFlat(True)
        #self.setText('Edit')
        self.setToolTip('Edit (Space)')

    def _on_setup(self):
        self.register_action('signal_edit_mode', self._update_state)
        self.register_action('signal_play', self._update_state)
        self.register_action('signal_silence', self._update_state)
        self.register_action('signal_record_mode', self._update_state)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('edit')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._clicked)

    def _update_state(self):
        old_block = self.blockSignals(True)
        disable = not self._sheet_mgr.allow_editing()
        is_checked = self._sheet_mgr.get_typewriter_connected() and not disable
        self.setChecked(is_checked)
        self.setEnabled(not disable)
        self.blockSignals(old_block)

    def _clicked(self):
        self._sheet_mgr.set_typewriter_connected(self.isChecked())


class ReplaceButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None

        self.setCheckable(True)
        self.setFlat(True)
        #self.setText('Replace')
        self.setToolTip('Replace (Insert)')

    def _on_setup(self):
        self.register_action('signal_replace_mode', self._update_state)
        self.register_action('signal_play', self._update_state)
        self.register_action('signal_silence', self._update_state)
        self.register_action('signal_record_mode', self._update_state)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('replace')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._clicked)

    def _update_state(self):
        old_block = self.blockSignals(True)
        disable = not self._sheet_mgr.allow_editing()
        is_checked = self._sheet_mgr.get_replace_mode() and not disable
        self.setChecked(is_checked)
        self.setEnabled(not disable)
        self.blockSignals(old_block)

    def _clicked(self):
        self._sheet_mgr.set_replace_mode(self.isChecked())


class RestButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None

        self.setFlat(True)
        #self.setText('══')
        self.setToolTip('Add rest (1)')

    def _on_setup(self):
        self.register_action('signal_module', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('rest')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._clicked)

    def _update_enabled(self):
        if (not self._sheet_mgr.is_editing_enabled() or
                not self._sheet_mgr.allow_editing()):
            self.setEnabled(False)
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        cur_column = self._sheet_mgr.get_column_at_location(location)
        is_enabled = bool(cur_column)

        self.setEnabled(is_enabled)

    def _clicked(self):
        trigger = Trigger('n-', None)
        self._sheet_mgr.add_trigger(trigger)


class DelSelectionButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None

        self.setFlat(True)
        #self.setText('Del')
        self.setToolTip('Delete selection (Delete)')

    def _on_setup(self):
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_module', self._update_enabled)
        self.register_action('signal_column', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('delete')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        self.setIconSize(_ICON_SIZE)

        self._update_enabled()
        self.clicked.connect(self._clicked)

    def _update_enabled(self):
        if not self._sheet_mgr.is_editing_enabled():
            self.setEnabled(False)
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            self.setEnabled(False)
            return

        cur_column = self._sheet_mgr.get_column_at_location(location)

        has_trigger = bool(cur_column) and cur_column.has_trigger(
                location.get_row_ts(), location.get_trigger_index())

        self.setEnabled(has_trigger)

    def _clicked(self):
        self._sheet_mgr.try_remove_trigger()


class UndoButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self.setFlat(True)
        #self.setText('Undo')
        self.setToolTip('Undo (Ctrl + Z)')

    def _on_setup(self):
        self.register_action('signal_sheet_undo', self._update_enabled)
        self.register_action('signal_sheet_redo', self._update_enabled)
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_pattern_length', self._update_enabled)
        self.register_action('signal_grid', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_history = self._ui_model.get_sheet_history()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('undo')
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._undo)

        self._update_enabled()

    def _update_enabled(self):
        playback_mgr = self._ui_model.get_playback_manager()
        self.setEnabled(self._sheet_history.has_past_changes() and
                (not playback_mgr.follow_playback_cursor() or
                    playback_mgr.is_recording()))

    def _undo(self):
        self._sheet_history.undo()


class RedoButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self.setFlat(True)
        #self.setText('Redo')
        self.setToolTip('Redo (Ctrl + Shift + Z)')

    def _on_setup(self):
        self.register_action('signal_sheet_undo', self._update_enabled)
        self.register_action('signal_sheet_redo', self._update_enabled)
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_pattern_length', self._update_enabled)
        self.register_action('signal_grid', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_history = self._ui_model.get_sheet_history()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('redo')
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._redo)

        self._update_enabled()

    def _update_enabled(self):
        playback_mgr = self._ui_model.get_playback_manager()
        self.setEnabled(self._sheet_history.has_future_changes() and
                (not playback_mgr.follow_playback_cursor() or
                    playback_mgr.is_recording()))

    def _redo(self):
        self._sheet_history.redo()


class CutOrCopyButton(QPushButton, Updater):

    def __init__(self, button_type):
        super().__init__()
        self._sheet_mgr = None

        if button_type == 'cut':
            text = 'Cut'
            shortcut = 'Ctrl + X'
        elif button_type == 'copy':
            text = 'Copy'
            shortcut = 'Ctrl + C'
        else:
            assert False

        self._button_type = button_type

        self.setFlat(True)
        #self.setText(text)
        self.setToolTip('{} ({})'.format(text, shortcut))

    def _on_setup(self):
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path(self._button_type)
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._cut_or_copy)

        self._update_enabled()

    def _update_enabled(self):
        selection = self._ui_model.get_selection()
        enabled = selection.has_area()
        playback_mgr = self._ui_model.get_playback_manager()
        enabled = enabled and (not playback_mgr.follow_playback_cursor() or
                playback_mgr.is_recording())
        self.setEnabled(enabled)

    def _cut_or_copy(self):
        selection = self._ui_model.get_selection()
        if selection.has_area():
            utils.copy_selected_area(self._sheet_mgr)
            if self._button_type == 'cut':
                self._sheet_mgr.try_remove_area()
            selection.clear_area()
            self._updater.signal_update('signal_selection')


class CutButton(CutOrCopyButton):

    def __init__(self):
        super().__init__('cut')


class CopyButton(CutOrCopyButton):

    def __init__(self):
        super().__init__('copy')


class PasteButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_mgr = None

        self.setFlat(True)
        #self.setText('Paste')
        self.setToolTip('Paste (Ctrl + V)')

        self._has_valid_data = False

    def _on_setup(self):
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('paste')
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        clipboard = QApplication.clipboard()
        clipboard.dataChanged.connect(self._update_enabled_full)

        self.clicked.connect(self._paste)

        self._update_enabled_full()

    def _update_enabled_full(self):
        self._has_valid_data = utils.is_clipboard_area_valid(self._sheet_mgr)
        self._update_enabled()

    def _update_enabled(self):
        selection = self._ui_model.get_selection()
        enabled = bool(selection.get_location()) and self._has_valid_data
        playback_mgr = self._ui_model.get_playback_manager()
        enabled = enabled and (not playback_mgr.follow_playback_cursor() or
                playback_mgr.is_recording())
        self.setEnabled(enabled)

    def _paste(self):
        selection = self._ui_model.get_selection()
        utils.try_paste_area(self._sheet_mgr)
        selection.clear_area()
        self._updater.signal_update('signal_selection')


class ConvertTriggerButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self.setFlat(True)
        #self.setText('Convert')
        self.setToolTip('Convert between set and slide trigger (/)')

    def _on_setup(self):
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('convert_trigger')
        self.setIcon(QIcon(icon_path))
        self.setIconSize(_ICON_SIZE)

        self.clicked.connect(self._convert_trigger)

        self._update_enabled()

    def _update_enabled(self):
        sheet_mgr = self._ui_model.get_sheet_manager()
        self.setEnabled(
                sheet_mgr.is_editing_enabled() and
                sheet_mgr.allow_editing() and
                sheet_mgr.is_at_convertible_set_or_slide_trigger())

    def _convert_trigger(self):
        sheet_mgr = self._ui_model.get_sheet_manager()
        sheet_mgr.convert_set_or_slide_trigger()


class ZoomButton(QPushButton, Updater):

    INFO = {
            'in': ('Zoom In', 'zoom_in', 'Ctrl + +'),
            'out': ('Zoom Out', 'zoom_out', 'Ctrl + -'),
            'original': ('Zoom to Original', 'zoom_reset', 'Ctrl + 0'),
            'expand_w': ('Expand Columns', 'col_expand', 'Ctrl + Alt + +'),
            'shrink_w': ('Shrink Columns', 'col_shrink', 'Ctrl + Alt + -'),
            'original_w': ('Reset Column Width', 'col_reset_width', 'Ctrl + Alt + 0'),
        }

    def __init__(self, mode):
        super().__init__()
        self._sheet_mgr = None

        self._mode = mode
        self.setFlat(True)
        #self.setText(self._get_text(mode))
        self.setToolTip(self._get_tooltip(mode))

    def _on_setup(self):
        self.register_action('signal_sheet_zoom', self._update_enabled)
        self.register_action('signal_sheet_zoom_range', self._update_enabled)
        self.register_action('signal_sheet_column_width', self._update_enabled)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        icon = self._get_icon(self._mode)
        self.setIcon(icon)
        self.setIconSize(_ICON_SIZE)

        self._update_enabled()
        self.clicked.connect(self._clicked)

    def _update_enabled(self):
        zoom = self._sheet_mgr.get_zoom()
        width = self._sheet_mgr.get_column_width()
        if self._mode == 'in':
            _, maximum = self._sheet_mgr.get_zoom_range()
            is_enabled = zoom < maximum
        elif self._mode == 'out':
            minimum, _ = self._sheet_mgr.get_zoom_range()
            is_enabled = zoom > minimum
        elif self._mode == 'original':
            is_enabled = zoom != 0
        elif self._mode == 'expand_w':
            _, maximum = self._sheet_mgr.get_column_width_range()
            is_enabled = width < maximum
        elif self._mode == 'shrink_w':
            minimum, _ = self._sheet_mgr.get_column_width_range()
            is_enabled = width > minimum
        elif self._mode == 'original_w':
            is_enabled = width != 0

        self.setEnabled(is_enabled)

    def _get_text(self, mode):
        return ZoomButton.INFO[mode][0]

    def _get_icon(self, mode):
        icon_name = ZoomButton.INFO[mode][1]
        icon_bank = self._ui_model.get_icon_bank()
        try:
            icon_path = icon_bank.get_icon_path(icon_name)
        except ValueError:
            return QIcon()
        icon = QIcon(icon_path)
        return icon

    def _get_shortcut(self, mode):
        return ZoomButton.INFO[mode][2]

    def _get_tooltip(self, mode):
        return '{} ({})'.format(self._get_text(mode), self._get_shortcut(mode))

    def _clicked(self):
        if self._mode in ('in', 'out', 'original'):
            new_zoom = 0
            if self._mode == 'in':
                new_zoom = self._sheet_mgr.get_zoom() + 1
            elif self._mode == 'out':
                new_zoom = self._sheet_mgr.get_zoom() - 1
            if self._sheet_mgr.set_zoom(new_zoom):
                self._updater.signal_update('signal_sheet_zoom')
        else:
            new_width = 0
            if self._mode == 'expand_w':
                new_width = self._sheet_mgr.get_column_width() + 1
            elif self._mode == 'shrink_w':
                new_width = self._sheet_mgr.get_column_width() - 1
            self._sheet_mgr.set_column_width(new_width)


class GridToggle(QCheckBox, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Grid')

    def _on_setup(self):
        self.register_action('signal_module', self._update_state)
        self.register_action('signal_grid', self._update_state)

        self.clicked.connect(self._set_grid_enabled)

        self._update_state()

    def _update_state(self):
        sheet_mgr = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_mgr.is_grid_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if is_grid_enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _set_grid_enabled(self):
        enabled = (self.checkState() == Qt.Checked)

        sheet_mgr = self._ui_model.get_sheet_manager()
        sheet_mgr.set_grid_enabled(enabled)
        self._updater.signal_update('signal_grid')


class GridEditorButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Edit grids')

    def _on_setup(self):
        self.register_action('signal_module', self._update_enabled)
        self.register_action('signal_grid', self._update_enabled)

        self.clicked.connect(self._open_grid_editor)

        self._update_enabled()

    def _update_enabled(self):
        sheet_mgr = self._ui_model.get_sheet_manager()
        is_grid_enabled = sheet_mgr.is_grid_enabled()
        self.setEnabled(is_grid_enabled)

    def _open_grid_editor(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_grid_editor()


class GridSelector(KqtComboBox, Updater):

    def __init__(self):
        super().__init__()
        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

    def _on_setup(self):
        self.register_action('signal_module', self._update_list)
        self.register_action('signal_grid_pattern_list', self._update_list)
        self.register_action('signal_selection', self._update_selection)
        self.register_action('signal_grid', self._update_selection)

        self.activated.connect(self._change_grid_pattern)

    def _update_list(self):
        self._update_grid_pattern_names()
        self._set_default_grid_pattern()
        self._update_grid_pattern_selection()

    def _update_selection(self):
        self._set_default_grid_pattern()
        self._update_grid_pattern_selection()

    def _update_grid_pattern_names(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_ids = grid_mgr.get_all_grid_pattern_ids()

        gp_items = [(gp_id, grid_mgr.get_grid_pattern(gp_id).get_name())
            for gp_id in gp_ids]
        gp_items = sorted(gp_items, key=lambda x: x[1])

        old_block = self.blockSignals(True)
        self.set_items((name, gp_id) for (gp_id, name) in gp_items)
        self.blockSignals(old_block)

    def _get_pattern_instance(self):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album.get_existence():
            return None

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        song = album.get_song_by_track(location.get_track())
        pinst = song.get_pattern_instance(location.get_system())
        return pinst

    def _set_default_grid_pattern(self):
        grid_mgr = self._ui_model.get_grid_manager()

        gp_id = None

        # Select grid at the current location if possible
        pinst = self._get_pattern_instance()
        if pinst:
            selection = self._ui_model.get_selection()
            location = selection.get_location()
            column = pinst.get_column(location.get_col_num())
            gp_id, _ = column.get_overlay_grid_info_at(location.get_row_ts())

            if gp_id == None:
                pattern = pinst.get_pattern()
                gp_id = pattern.get_base_grid_pattern_id()

        # Fall back to whatever we can find from the grid pattern collection
        if gp_id == None:
            all_gp_ids = grid_mgr.get_all_grid_pattern_ids()
            if all_gp_ids:
                gp_id = all_gp_ids[0]

        if gp_id != None:
            grid_mgr.set_default_grid_pattern_id(gp_id)

    def _update_grid_pattern_selection(self):
        grid_mgr = self._ui_model.get_grid_manager()
        default_id = grid_mgr.get_default_grid_pattern_id()

        old_block = self.blockSignals(True)
        index = self.findData(default_id)
        if index < 0:
            index = self.findData(0)
        assert index >= 0
        self.setCurrentIndex(index)
        self.blockSignals(old_block)

    def _change_grid_pattern(self, index):
        gp_id = self.itemData(index)
        if not gp_id:
            return

        pinst = self._get_pattern_instance()
        if not pinst:
            return

        offset = tstamp.Tstamp(0) # TODO

        pattern = pinst.get_pattern()

        sheet_mgr = self._ui_model.get_sheet_manager()

        selection = self._ui_model.get_selection()
        if selection.has_rect_area():
            top_left = selection.get_area_top_left()
            bottom_right = selection.get_area_bottom_right()
            start_col = top_left.get_col_num()
            stop_col = bottom_right.get_col_num() + 1
            start_ts = top_left.get_row_ts()
            stop_ts = bottom_right.get_row_ts()

            pat_length = pattern.get_length()
            full_columns_selected = ((start_ts == tstamp.Tstamp(0)) and
                    (stop_ts > pat_length))
            all_selected = (full_columns_selected and
                    ((start_col, stop_col) == (0, COLUMNS_MAX)))

            if all_selected:
                sheet_mgr.set_pattern_base_grid_pattern_id(
                        pattern, gp_id, is_final=False)

            if all_selected or (gp_id == pattern.get_base_grid_pattern_id()):
                gp_id = None

            if (full_columns_selected and
                    (pattern.get_base_grid_pattern_id() == gp_id) and
                    (pattern.get_base_grid_pattern_offset() == offset)):
                sheet_mgr.clear_overlay_grids(pinst, start_col, stop_col)
            else:
                sheet_mgr.set_overlay_grid(
                        pinst, start_col, stop_col, start_ts, stop_ts, gp_id, offset)

        else:
            location = selection.get_location()
            col_num = location.get_col_num()
            column = pinst.get_column(location.get_col_num())
            start_ts, stop_ts = column.get_overlay_grid_range_at(location.get_row_ts())
            col_gp_id, offset = column.get_overlay_grid_info_at(location.get_row_ts())

            if col_gp_id != None:
                pat_length = pattern.get_length()
                full_column_selected = ((start_ts == tstamp.Tstamp(0)) and
                        (stop_ts > pat_length))
                if (full_column_selected and
                        (pattern.get_base_grid_pattern_id() == gp_id) and
                        (pattern.get_base_grid_pattern_offset() == offset)):
                    sheet_mgr.clear_overlay_grids(pinst, col_num, col_num + 1)
                else:
                    if gp_id == pattern.get_base_grid_pattern_id():
                        gp_id = None
                    sheet_mgr.set_overlay_grid(
                        pinst, col_num, col_num + 1, start_ts, stop_ts, gp_id, offset)

            else:
                sheet_mgr.set_pattern_base_grid_pattern_id(pattern, gp_id, is_final=True)

        self._updater.signal_update('signal_grid')


class LengthEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._is_latest_committed = True

        self._spinbox = VarPrecSpinBox(step_decimals=0, max_decimals=5)
        self._spinbox.setMinimum(0)
        self._spinbox.setMaximum(1024)

        h = QHBoxLayout()
        h.setContentsMargins(5, 0, 5, 0)
        h.setSpacing(5)
        h.addWidget(QLabel('Pattern length'))
        h.addWidget(self._spinbox)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_module', self._update_value)
        self.register_action('signal_pattern_length', self._update_value)
        self.register_action('signal_selection', self._update_value)
        self.register_action('signal_order_list', self._update_value)
        self.register_action('signal_sheet_undo', self._update_value)
        self.register_action('signal_sheet_redo', self._update_value)

        self._update_value()

        self._spinbox.valueChanged.connect(self._change_length)
        self._spinbox.editingFinished.connect(self._change_length_final)

    def _get_pattern(self):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album.get_existence():
            return None

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        song = album.get_song_by_track(location.get_track())
        pinst = song.get_pattern_instance(location.get_system())
        pattern = pinst.get_pattern()
        return pattern

    def _update_value(self):
        pattern = self._get_pattern()
        if not pattern:
            self.setEnabled(False)
            old_block = self._spinbox.blockSignals(True)
            self._spinbox.setValue(0)
            self._spinbox.blockSignals(old_block)
            return

        length = pattern.get_length()
        length_val = float(length)

        self.setEnabled(True)
        old_block = self._spinbox.blockSignals(True)
        if length_val != self._spinbox.value():
            self._spinbox.setValue(length_val)
        self._spinbox.blockSignals(old_block)

    def _change_value(self, new_value, is_final):
        pattern = self._get_pattern()
        if not pattern:
            return

        sheet_mgr = self._ui_model.get_sheet_manager()

        length = tstamp.Tstamp(new_value)
        if length == pattern.get_length():
            if is_final and not self._is_latest_committed:
                sheet_mgr.set_pattern_length(pattern, length, is_final)
                self._is_latest_committed = True
            return

        sheet_mgr.set_pattern_length(pattern, length, is_final)
        self._updater.signal_update('signal_pattern_length')

    def _change_length(self, new_value):
        self._is_latest_committed = False
        self._change_value(new_value, is_final=False)

    def _change_length_final(self):
        new_value = self._spinbox.value()
        self._change_value(new_value, is_final=True)


class HackSeparator(QFrame):

    def __init__(self):
        super().__init__()
        self.setFrameShape(QFrame.VLine)
        self.setFrameShadow(QFrame.Sunken)


