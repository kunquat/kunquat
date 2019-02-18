# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math
import os
import string

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.config as config
from .filedialog import FileDialog
from .headerline import HeaderLine
from .kqtcombobox import KqtComboBox
from .numberslider import NumberSlider
from . import utils
from .updater import Updater
from .utils import get_abs_window_size, get_default_font_info, update_ref_font_height


class Settings(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._modules = Modules()
        self._instruments = Instruments()
        self._samples = Samples()
        self._effects = Effects()

        self._chord_mode = ChordMode()

        self._font = FontSelector()
        self._border_contrast = BorderContrast()
        self._button_brightness = ButtonBrightness()
        self._button_press_brightness = ButtonPressBrightness()
        self._colours = Colours()

        self.add_to_updaters(
                self._modules,
                self._instruments,
                self._samples,
                self._effects,
                self._chord_mode,
                self._font,
                self._border_contrast,
                self._button_brightness,
                self._button_press_brightness,
                self._colours)

        self._dirs_layout = QGridLayout()
        self._dirs_layout.setContentsMargins(0, 0, 0, 0)
        self._dirs_layout.setHorizontalSpacing(4)
        self._dirs_layout.setVerticalSpacing(2)
        self._dirs_layout.addWidget(QLabel('Modules:'), 0, 0)
        self._dirs_layout.addWidget(self._modules, 0, 1)
        self._dirs_layout.addWidget(QLabel('Instruments:'), 1, 0)
        self._dirs_layout.addWidget(self._instruments, 1, 1)
        self._dirs_layout.addWidget(QLabel('Samples:'), 2, 0)
        self._dirs_layout.addWidget(self._samples, 2, 1)
        self._dirs_layout.addWidget(QLabel('Effects:'), 3, 0)
        self._dirs_layout.addWidget(self._effects, 3, 1)

        self._ui_layout = QGridLayout()
        self._ui_layout.setContentsMargins(0, 0, 0, 0)
        self._ui_layout.setHorizontalSpacing(4)
        self._ui_layout.setVerticalSpacing(2)
        self._ui_layout.setColumnStretch(2, 1)
        self._ui_layout.addWidget(QLabel('Chord editing mode:'), 0, 0)
        self._ui_layout.addWidget(self._chord_mode, 0, 1)

        self._dir_header = HeaderLine('Default directories')
        self._ui_header = HeaderLine('User interface')

        self._behaviour_layout = QVBoxLayout()
        self._behaviour_layout.setContentsMargins(0, 0, 0, 0)
        self._behaviour_layout.setSpacing(4)
        self._behaviour_layout.addWidget(self._dir_header)
        self._behaviour_layout.addLayout(self._dirs_layout)
        self._behaviour_layout.addWidget(self._ui_header)
        self._behaviour_layout.addLayout(self._ui_layout)
        self._behaviour_layout.addStretch(1)

        self._misc_style_layout = QGridLayout()
        self._misc_style_layout.setContentsMargins(0, 0, 0, 0)
        self._misc_style_layout.setSpacing(2)
        self._misc_style_layout.addWidget(QLabel('Default font:'), 0, 0)
        self._misc_style_layout.addWidget(self._font, 0, 1)
        self._misc_style_layout.addWidget(QLabel('Border contrast:'), 1, 0)
        self._misc_style_layout.addWidget(self._border_contrast, 1, 1)
        self._misc_style_layout.addWidget(QLabel('Button brightness:'), 2, 0)
        self._misc_style_layout.addWidget(self._button_brightness, 2, 1)
        self._misc_style_layout.addWidget(QLabel('Button press brightness:'), 3, 0)
        self._misc_style_layout.addWidget(self._button_press_brightness, 3, 1)

        self._appearance_header = HeaderLine('Appearance')

        self._appearance_layout = QVBoxLayout()
        self._appearance_layout.setContentsMargins(0, 0, 0, 0)
        self._appearance_layout.setSpacing(4)
        self._appearance_layout.addWidget(self._appearance_header)
        self._appearance_layout.addLayout(self._misc_style_layout)
        self._appearance_layout.addWidget(self._colours)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(8)
        h.addLayout(self._behaviour_layout)
        h.addLayout(self._appearance_layout)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        spacing_x = style_mgr.get_scaled_size_param('medium_padding')
        spacing_y = style_mgr.get_scaled_size_param('small_padding')

        self._dir_header.update_style(style_mgr)
        self._ui_header.update_style(style_mgr)
        self._appearance_header.update_style(style_mgr)

        self._dirs_layout.setHorizontalSpacing(spacing_x)
        self._dirs_layout.setVerticalSpacing(spacing_y)
        self._ui_layout.setHorizontalSpacing(spacing_x)
        self._ui_layout.setVerticalSpacing(spacing_y)
        self._behaviour_layout.setSpacing(spacing_y)
        self._misc_style_layout.setHorizontalSpacing(spacing_x)
        self._misc_style_layout.setHorizontalSpacing(spacing_y)
        self._appearance_layout.setSpacing(spacing_y)

        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))


class Directory(QWidget, Updater):

    def __init__(self, conf_key, caption):
        super().__init__()
        self._conf_key = conf_key
        self._caption = caption

        self._text = QLineEdit()
        self._browse = QPushButton('Browse...')

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._text)
        h.addWidget(self._browse)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_settings_dir', self._update_dir)
        self.register_action('signal_style_changed', self._update_style)

        self._text.textEdited.connect(self._change_dir_text)
        self._browse.clicked.connect(self._change_dir_browse)

        self._update_style()
        self._update_dir()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_dir(self):
        cfg = config.get_config()
        directory = cfg.get_value(self._conf_key) or ''

        old_block = self._text.blockSignals(True)
        if directory != self._text.text():
            self._text.setText(directory)
        self._text.blockSignals(old_block)

    def _change_dir_text(self, text):
        cfg = config.get_config()
        cfg.set_value(self._conf_key, text)
        self._updater.signal_update('signal_settings_dir')

    def _change_dir_browse(self):
        cfg = config.get_config()
        cur_dir = cfg.get_value(self._conf_key) or os.getcwd()
        dialog = FileDialog(
                self._ui_model, FileDialog.MODE_CHOOSE_DIR, self._caption, cur_dir)
        new_dir = dialog.get_path()
        if new_dir:
            self._change_dir_text(new_dir)


class Modules(Directory):

    def __init__(self):
        super().__init__('dir_modules', 'Choose default directory for Kunquat modules')


class Instruments(Directory):

    def __init__(self):
        super().__init__(
                'dir_instruments', 'Choose default directory for Kunquat instruments')


class Samples(Directory):

    def __init__(self):
        super().__init__('dir_samples', 'Choose default directory for samples')


class Effects(Directory):

    def __init__(self):
        super().__init__('dir_effects', 'Choose default directory for Kunquat effects')


class ChordMode(QCheckBox, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_chord_mode_changed', self._update_enabled)
        self.stateChanged.connect(self._change_enabled)
        self._update_enabled()

    def _update_enabled(self):
        enabled = config.get_config().get_value('chord_mode')

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)
        config.get_config().set_value('chord_mode', enabled)
        self._updater.signal_update('signal_chord_mode_changed')


class FontSelector(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._font_family = FontFamilySelector()
        self._font_size = FontSizeSelector()

        self.add_to_updaters(self._font_family, self._font_size)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.addWidget(self._font_family, 1)
        h.addWidget(self._font_size)
        self.setLayout(h)

    def _on_setup(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class FontFamilyModel(QAbstractItemModel):

    def __init__(self, style_mgr, families, container):
        super().__init__()
        self._style_mgr = style_mgr
        self._families = families
        self._container = container

    # Qt interface

    def index(self, row, column, parent):
        if parent.isValid():
            return QModelIndex()
        if not (0 <= row < len(self._families)) or (column != 0):
            return QModelIndex()

        return self.createIndex(row, column, self._families[row])

    def parent(self, index):
        return QModelIndex()

    def rowCount(self, parent):
        if parent.isValid():
            return 0
        return len(self._families)

    def columnCount(self, parent):
        if parent.isValid():
            return 0
        return 1

    def data(self, index, role):
        row = index.row()
        if (0 <= row < len(self._families)) and (index.column() == 0):
            if role == Qt.DisplayRole:
                return self._families[row]

            elif role == Qt.SizeHintRole:
                _, font_size = get_default_font_info(self._style_mgr)
                font = QFont(self._families[row])
                font.setPointSize(font_size)
                fm = QFontMetrics(font, self._container)
                rect = fm.tightBoundingRect(self._families[row])
                dim_add = self._style_mgr.get_scaled_size_param('medium_padding') * 2
                return QSize(rect.width() + dim_add, rect.height() + dim_add)

            elif role == Qt.TextAlignmentRole:
                return Qt.AlignLeft | Qt.AlignVCenter

            elif role == Qt.FontRole:
                _, font_size = get_default_font_info(self._style_mgr)
                font = QFont(self._families[row])
                font.setPointSize(font_size)
                return font


class FontFamilySelector(KqtComboBox, Updater):

    def __init__(self):
        super().__init__()
        self._model = None

    def _on_setup(self):
        self._families = QFontDatabase().families()

        self._model = FontFamilyModel(
                self._ui_model.get_style_manager(), self._families, self)
        self.setModel(self._model)

        self.register_action('signal_style_changed', self._update_family)
        self.currentTextChanged.connect(self._change_family)

        self._update_family()

    def _update_family(self):
        style_mgr = self._ui_model.get_style_manager()
        family, _ = get_default_font_info(style_mgr)

        old_block = self.blockSignals(True)
        if self.currentText() != family:
            self.setCurrentText(family)
        self.blockSignals(old_block)

    def _change_family(self, new_family):
        style_mgr = self._ui_model.get_style_manager()
        _, font_size = get_default_font_info(style_mgr)

        style_mgr.set_style_param('def_font_family', new_family)
        update_ref_font_height((new_family, font_size), style_mgr)

        self._updater.signal_update('signal_style_changed')


class FontSizeSelector(QSpinBox, Updater):

    _MIN_SIZE = 6
    _MAX_SIZE = 32

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_size)

        self.setRange(self._MIN_SIZE, self._MAX_SIZE)
        self.valueChanged.connect(self._change_size)

        self._update_size()

    def _update_size(self):
        style_mgr = self._ui_model.get_style_manager()
        _, font_size = get_default_font_info(style_mgr)
        size = min(max(self._MIN_SIZE, font_size), self._MAX_SIZE)

        old_block = self.blockSignals(True)
        if size != self.value():
            self.setValue(size)
        self.blockSignals(old_block)

    def _change_size(self, value):
        new_size = min(max(self._MIN_SIZE, self.value()), self._MAX_SIZE)

        style_mgr = self._ui_model.get_style_manager()
        font_family, _ = get_default_font_info(style_mgr)

        style_mgr.set_style_param('def_font_size', new_size)
        update_ref_font_height((font_family, new_size), style_mgr)

        self._updater.signal_update('signal_style_changed')


class StyleSlider(NumberSlider, Updater):

    def __init__(self, param, min_val, max_val, desc=''):
        super().__init__(2, min_val, max_val, title=desc, width_txt='-0.00')
        self._param = param
        self.numberChanged.connect(self._change_param)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_param)
        self._update_param()

    def _update_param(self):
        style_mgr = self._ui_model.get_style_manager()
        self.update_style(style_mgr)
        self.set_number(style_mgr.get_style_param(self._param))

    def _change_param(self, new_value):
        style_mgr = self._ui_model.get_style_manager()
        style_mgr.set_style_param(self._param, new_value)
        self._updater.signal_update('signal_style_changed')


class BorderContrast(StyleSlider):

    def __init__(self):
        super().__init__('border_contrast', 0.0, 1.0)


class ButtonBrightness(StyleSlider):

    def __init__(self):
        super().__init__('button_brightness', -1.0, 1.0)


class ButtonPressBrightness(StyleSlider):

    def __init__(self):
        super().__init__('button_press_brightness', -1.0, 1.0)


class ColourCategoryModel():

    def __init__(self, name):
        self._name = name
        self._colours = []

    def add_colour(self, colour):
        colour.set_category(self)
        self._colours.append(colour)

    def get_name(self):
        return self._name

    def get_colour_count(self):
        return len(self._colours)

    def get_colour(self, index):
        return self._colours[index]

    def get_index_of_colour(self, colour):
        return self._colours.index(colour)


class ColourModel():

    def __init__(self, key, colour):
        self._key = key
        self._colour = colour
        self._category = None

    def set_category(self, category):
        assert self._category == None
        self._category = category

    def get_key(self):
        return self._key

    def get_colour(self):
        return self._colour

    def get_category(self):
        return self._category


_COLOUR_DESCS = [
    ('bg_colour',                       'Default background'),
    ('fg_colour',                       'Default text'),
    ('bg_sunken_colour',                'Sunken background'),
    ('disabled_fg_colour',              'Disabled text'),
    ('important_button_bg_colour',      'Important button background'),
    ('important_button_fg_colour',      'Important button text'),
    ('text_bg_colour',                  'Text field background'),
    ('text_fg_colour',                  'Text field text'),
    ('text_selected_bg_colour',         'Text field selected background'),
    ('text_selected_fg_colour',         'Text field selected text'),
    ('text_disabled_fg_colour',         'Text field disabled text'),
    ('active_indicator_colour',         'Activity indicator'),
    ('conns_bg_colour',                 'Connections background'),
    ('conns_focus_colour',              'Connections focus highlight'),
    ('conns_edge_colour',               'Connections cable'),
    ('conns_port_colour',               'Connections port'),
    ('conns_invalid_port_colour',       'Connections invalid connection'),
    ('conns_inst_bg_colour',            'Connections instrument background'),
    ('conns_inst_fg_colour',            'Connections instrument text'),
    ('conns_effect_bg_colour',          'Connections effect background'),
    ('conns_effect_fg_colour',          'Connections effect text'),
    ('conns_proc_voice_bg_colour',      'Connections voice signal processor background'),
    ('conns_proc_voice_fg_colour',      'Connections voice signal processor text'),
    ('conns_proc_voice_selected_colour', 'Connections voice signal processor highlight'),
    ('conns_proc_mixed_bg_colour',      'Connections mixed signal processor background'),
    ('conns_proc_mixed_fg_colour',      'Connections mixed signal processor text'),
    ('conns_master_bg_colour',          'Connections master background'),
    ('conns_master_fg_colour',          'Connections master text'),
    ('envelope_bg_colour',              'Envelope background'),
    ('envelope_axis_label_colour',      'Envelope axis label'),
    ('envelope_axis_line_colour',       'Envelope axis line'),
    ('envelope_curve_colour',           'Envelope curve'),
    ('envelope_node_colour',            'Envelope node'),
    ('envelope_focus_colour',           'Envelope focus highlight'),
    ('envelope_loop_marker_colour',     'Envelope loop marker'),
    ('peak_meter_bg_colour',            'Peak meter background'),
    ('peak_meter_low_colour',           'Peak meter low level'),
    ('peak_meter_mid_colour',           'Peak meter -6 dB level'),
    ('peak_meter_high_colour',          'Peak meter maximum level'),
    ('peak_meter_clip_colour',          'Peak meter clipping indicator'),
    ('position_bg_colour',              'Position view background'),
    ('position_fg_colour',              'Position view numbers'),
    ('position_stopped_colour',         'Position view stop'),
    ('position_play_colour',            'Position view play'),
    ('position_record_colour',          'Position view record'),
    ('position_infinite_colour',        'Position view infinite mode'),
    ('position_title_colour',           'Position view text'),
    ('sample_map_bg_colour',            'Sample map background'),
    ('sample_map_axis_label_colour',    'Sample map axis label'),
    ('sample_map_axis_line_colour',     'Sample map axis line'),
    ('sample_map_focus_colour',         'Sample map focus highlight'),
    ('sample_map_point_colour',         'Sample map point'),
    ('sample_map_selected_colour',      'Sample map selection highlight'),
    ('sheet_area_selection_colour',     'Sheet area selection'),
    ('sheet_canvas_bg_colour',          'Sheet canvas background'),
    ('sheet_column_bg_colour',          'Sheet column background'),
    ('sheet_column_border_colour',      'Sheet column border'),
    ('sheet_cursor_view_line_colour',   'Sheet navigating cursor line'),
    ('sheet_cursor_edit_line_colour',   'Sheet editing cursor line'),
    ('sheet_grid_level_1_colour',       'Sheet grid level 1'),
    ('sheet_grid_level_2_colour',       'Sheet grid level 2'),
    ('sheet_grid_level_3_colour',       'Sheet grid level 3'),
    ('sheet_header_bg_colour',          'Sheet header background'),
    ('sheet_header_fg_colour',          'Sheet header text'),
    ('sheet_header_solo_colour',        'Sheet header solo'),
    ('sheet_playback_cursor_colour',    'Sheet playback cursor'),
    ('sheet_ruler_bg_colour',           'Sheet ruler background'),
    ('sheet_ruler_fg_colour',           'Sheet ruler foreground'),
    ('sheet_ruler_playback_marker_colour', 'Sheet playback marker'),
    ('sheet_trigger_default_colour',    'Default trigger'),
    ('sheet_trigger_note_on_colour',    'Note on trigger'),
    ('sheet_trigger_hit_colour',        'Hit trigger'),
    ('sheet_trigger_note_off_colour',   'Note off trigger'),
    ('sheet_trigger_warning_bg_colour', 'Trigger value warning background'),
    ('sheet_trigger_warning_fg_colour', 'Trigger value warning text'),
    ('system_load_bg_colour',           'System load background'),
    ('system_load_low_colour',          'System load low level'),
    ('system_load_mid_colour',          'System load medium level'),
    ('system_load_high_colour',         'System load high level'),
    ('waveform_bg_colour',              'Waveform background'),
    ('waveform_focus_colour',           'Waveform focus highlight'),
    ('waveform_centre_line_colour',     'Waveform centre line'),
    ('waveform_zoomed_out_colour',      'Waveform zoomed out'),
    ('waveform_single_item_colour',     'Waveform single item'),
    ('waveform_interpolated_colour',    'Waveform interpolated'),
    ('waveform_loop_marker_colour',     'Waveform loop marker'),
]

_COLOUR_DESCS_DICT = dict(_COLOUR_DESCS)


class ColoursModel(QAbstractItemModel, Updater):

    def __init__(self):
        super().__init__()
        self._colours = []

    def _on_setup(self):
        self._make_colour_list()

    def _make_colour_list(self):
        style_mgr = self._ui_model.get_style_manager()

        colours = []

        categories = {}
        category_info = {
            'conns_'      : 'Connections',
            'envelope_'   : 'Envelope',
            'peak_meter_' : 'Peak meter',
            'position_'   : 'Position view',
            'sample_map_' : 'Sample map',
            'sheet_'      : 'Sheet',
            'system_load_': 'System load',
            'waveform_'   : 'Waveform',
        }

        for k, _ in _COLOUR_DESCS:
            colour = style_mgr.get_style_param(k)
            for cat_prefix, cat_name in category_info.items():
                if k.startswith(cat_prefix):
                    if cat_prefix not in categories:
                        categories[cat_prefix] = ColourCategoryModel(cat_name)
                        colours.append(categories[cat_prefix])
                    if ((cat_prefix != 'position_') or
                            cmdline.get_experimental() or
                            (k != 'position_record_colour')):
                        categories[cat_prefix].add_colour(ColourModel(k, colour))
                    break
            else:
                colours.append(ColourModel(k, colour))

        self._colours = colours

    def get_colour_indices(self):
        for row, node in enumerate(self._colours):
            if isinstance(node, ColourCategoryModel):
                for i in range(node.get_colour_count()):
                    contained_node = node.get_colour(i)
                    yield self.createIndex(i, 1, contained_node)
            else:
                yield self.createIndex(row, 1, node)

    # Qt interface

    def columnCount(self, _):
        return 2

    def rowCount(self, parent):
        if not parent.isValid():
            return len(self._colours)
        node = parent.internalPointer()
        if isinstance(node, ColourCategoryModel):
            return node.get_colour_count()
        return 0

    def index(self, row, col, parent):
        if not parent.isValid():
            node = self._colours[row]
        else:
            parent_node = parent.internalPointer()
            if isinstance(parent_node, ColourCategoryModel):
                node = parent_node.get_colour(row)
            else:
                return QModelIndex()
        return self.createIndex(row, col, node)

    def parent(self, index):
        if not index.isValid():
            return QModelIndex()
        node = index.internalPointer()
        if isinstance(node, ColourCategoryModel):
            return QModelIndex()
        elif isinstance(node, ColourModel):
            category = node.get_category()
            if category:
                index = category.get_index_of_colour(node)
                return self.createIndex(index, 0, category)
            else:
                return QModelIndex()
        else:
            assert False

    def data(self, index, role):
        if role == Qt.DisplayRole:
            assert index.isValid()
            node = index.internalPointer()
            column = index.column()
            if isinstance(node, ColourCategoryModel):
                if column == 0:
                    return node.get_name()
                elif column == 1:
                    return ''
            elif isinstance(node, ColourModel):
                if column == 0:
                    key = node.get_key()
                    desc = _COLOUR_DESCS_DICT[key]
                    category = node.get_category()
                    if category:
                        strip_prefix = category.get_name() + ' '
                        if desc.startswith(strip_prefix):
                            desc = desc[len(strip_prefix):]
                            desc = desc[0].upper() + desc[1:]
                    return desc
                elif column == 1:
                    return '' # ColourButton takes care of display
            else:
                assert False

        return None

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        return Qt.ItemIsEnabled


class ColourButton(QPushButton):

    colourSelected = Signal(str, name='colourSelected')

    def __init__(self, key):
        super().__init__()
        self._key = key

        self.setFixedSize(QSize(48, 16))
        self.setAutoFillBackground(True)

        self.clicked.connect(self._clicked)

    def get_key(self):
        return self._key

    def set_colour(self, colour):
        style = 'QPushButton {{ background-color: {}; }}'.format(colour)
        self.setStyleSheet(style)

    def update_style(self, style_mgr):
        width = style_mgr.get_scaled_size(4.6)
        height = style_mgr.get_scaled_size(1.5)
        self.setFixedSize(QSize(width, height))

    def _clicked(self):
        self.colourSelected.emit(self._key)


class ColourSelector(QWidget):

    colourChanged = Signal(name='colourChanged')
    colourSelected = Signal(name='colourSelected')

    _STATE_IDLE = 'idle'
    _STATE_EDITING_HUE = 'editing_hue'
    _STATE_EDITING_SV = 'editing_sv'

    def __init__(self):
        super().__init__()

        self._config = {
            'hue_ring_thickness'  : 0.25,
            'sv_gradient_radius'  : 16,
            'hue_marker_thickness': 2.5,
            'sv_marker_radius'    : 6,
            'sv_marker_thickness' : 2,
        }

        self._style_mgr = None

        self._hue_outer_radius = None
        self._hue_ring = None
        self._sv_triangle = None
        self._sv_gradients = []

        self._colour = QColor(0, 0, 0)
        self._hue = 0
        self._saturation = 0
        self._value = 0

        self._make_sv_gradients()

        self._state = self._STATE_IDLE

        self.setMouseTracking(True)

    def update_style(self, style_mgr):
        self._style_mgr = style_mgr

        config = {
            'hue_marker_thickness'  : self._style_mgr.get_scaled_size(0.2),
            'sv_marker_radius'      : self._style_mgr.get_scaled_size(0.5),
            'sv_marker_thickness'   : self._style_mgr.get_scaled_size(0.2),
        }

        self._config.update(config)

    def set_colour(self, colour):
        if self._colour.toRgb() != colour.toRgb():
            self._colour = colour
            self._hue, self._saturation, self._value, _ = self._colour.getHsvF()
            self._hue = max(0, self._hue) # set indeterminate hue to red
            self._sv_triangle = None
            self.update()

    def get_colour(self):
        return self._colour.toRgb()

    def _set_colour(self, colour):
        if self._colour != colour:
            self._sv_triangle = None
            self._colour = colour
            self.colourChanged.emit()
            self.update()

    def _get_rel_coords(self, event):
        x = event.x() - (self.width() // 2)
        y = event.y() - (self.height() // 2)
        return x, y

    def _length_and_dir(self, v):
        x = v.x()
        y = v.y()
        vlen = math.sqrt(x * x + y * y)
        vdir = QPointF(1, 0)
        if vlen > 0.05:
            vdir = v * (1 / vlen)
        return vlen, vdir

    def _dir(self, v):
        _, dir_v = self._length_and_dir(v)
        return dir_v

    def _dot(self, a, b):
        return a.x() * b.x() + a.y() * b.y()

    def _set_hue_from_coords(self, x, y):
        v = QPointF(x, y)
        _, vdir = self._length_and_dir(v)

        d = self._dot(vdir, QPointF(1, 0))
        angle = math.acos(min(max(-1, d), 1))
        if vdir.y() > 0:
            angle = 2 * math.pi - angle

        new_hue = angle / (2 * math.pi)
        self._hue = new_hue
        new_colour = QColor.fromHsvF(new_hue, self._saturation, self._value)
        self._set_colour(new_colour)

    def _get_sv_corners(self):
        hue_ring_thickness = self._config['hue_ring_thickness']
        hue_inner_radius = self._hue_outer_radius * (1 - hue_ring_thickness)

        hue_angle = self._hue * 2 * math.pi
        hue_dir = QPointF(math.cos(hue_angle), -math.sin(hue_angle))
        sv_black_dir = QPointF(
                math.cos(hue_angle + math.pi * 2 / 3),
                -math.sin(hue_angle + math.pi * 2 / 3))
        sv_white_dir = QPointF(
                math.cos(hue_angle - math.pi * 2 / 3),
                -math.sin(hue_angle - math.pi * 2 / 3))
        sv_colour_pos = hue_dir * hue_inner_radius
        sv_black_pos = sv_black_dir * hue_inner_radius
        sv_white_pos = sv_white_dir * hue_inner_radius

        return sv_colour_pos, sv_black_pos, sv_white_pos

    def _try_set_sv_from_coords(self, x, y, allow_any_point=False):
        v = QPointF(x, y)

        sv_colour_pos, sv_black_pos, sv_white_pos = self._get_sv_corners()
        side_length, sat_dir = self._length_and_dir(sv_colour_pos - sv_white_pos)

        if allow_any_point:
            # Check snapping to corners and edges
            min_dot = math.cos(math.pi / 3)

            def is_at_corner(corner):
                return (self._dot(self._dir(corner), self._dir(v - corner)) >= min_dot)

            def is_at_edge(point, normal):
                return (self._dot(self._dir(v - point), normal) > 0)

            def get_proj_pos_norm_at_line(a, b):
                amb = a - b
                length_sq = (amb.x() * amb.x()) + (amb.y() * amb.y())
                return min(max(0, self._dot(v - a, b - a) / length_sq), 1)

            new_saturation = None
            new_value = None

            if is_at_corner(sv_colour_pos):
                new_saturation = 1
                new_value = 1
            elif is_at_corner(sv_black_pos):
                new_saturation = 0
                new_value = 0
            elif is_at_corner(sv_white_pos):
                new_saturation = 0
                new_value = 1
            elif is_at_edge(sv_colour_pos, -self._dir(sv_white_pos)):
                new_saturation = 1
                new_value = get_proj_pos_norm_at_line(sv_black_pos, sv_colour_pos)
            elif is_at_edge(sv_white_pos, -self._dir(sv_black_pos)):
                new_saturation = get_proj_pos_norm_at_line(sv_white_pos, sv_colour_pos)
                new_value = 1
            elif is_at_edge(sv_white_pos, -self._dir(sv_colour_pos)):
                new_saturation = 0
                new_value = get_proj_pos_norm_at_line(sv_black_pos, sv_white_pos)

            if new_saturation != None:
                self._value = new_value
                self._saturation = new_saturation
                new_colour = QColor.fromHsvF(self._hue, new_saturation, new_value)
                self._set_colour(new_colour)
                return True

        # Get new value
        bx, by = sv_black_pos.x(), sv_black_pos.y()
        wx, wy = sv_white_pos.x(), sv_white_pos.y()
        x2, y2 = v.x() + sat_dir.x(), v.y() + sat_dir.y()
        val_x = (((bx * wy - by * wx) * (x - x2) - (bx - wx) * (x * y2 - y * x2)) /
                ((bx - wx) * (y - y2) - (by - wy) * (x - x2)))
        val_y = (((bx * wy - by * wx) * (y - y2) - (by - wy) * (x * y2 - y * x2)) /
                ((bx - wx) * (y - y2) - (by - wy) * (x - x2)))
        val_pos = QPointF(val_x, val_y)
        val_dist_from_black, _ = self._length_and_dir(val_pos - sv_black_pos)
        val_dist_from_white, _ = self._length_and_dir(val_pos - sv_white_pos)
        if val_dist_from_black + val_dist_from_white > side_length + 0.0001:
            if not allow_any_point:
                return False
            if val_dist_from_black < val_dist_from_white:
                new_value = 0
            else:
                new_value = 1
        else:
            new_value = min(max(0, val_dist_from_black / side_length), 1)

        # Get new saturation
        sat_length = new_value * side_length
        sat_max_pos = val_pos + (sat_dir * sat_length)
        v_dist_from_grey, _ = self._length_and_dir(v - val_pos)
        v_dist_from_colour, _ = self._length_and_dir(v - sat_max_pos)
        if v_dist_from_grey + v_dist_from_colour > sat_length + 0.0001:
            if not allow_any_point:
                return False
            if v_dist_from_grey < v_dist_from_colour:
                new_saturation = 0
            else:
                new_saturation = 1
        else:
            if sat_length > 0.001:
                new_saturation = min(max(0, v_dist_from_grey / sat_length), 1)
            else:
                new_saturation = 1

        self._value = new_value
        self._saturation = new_saturation
        new_colour = QColor.fromHsvF(self._hue, new_saturation, new_value)
        self._set_colour(new_colour)
        return True

    def hideEvent(self, event):
        self._colour = QColor(0, 0, 0)
        self._hue = 0
        self._saturation = 0
        self._value = 0
        self._sv_triangle = None

    def mouseMoveEvent(self, event):
        x, y = self._get_rel_coords(event)

        if self._state == self._STATE_IDLE:
            pass
        elif self._state == self._STATE_EDITING_HUE:
            self._set_hue_from_coords(x, y)
        elif self._state == self._STATE_EDITING_SV:
            self._try_set_sv_from_coords(x, y, allow_any_point=True)
        else:
            assert False

    def mousePressEvent(self, event):
        if event.buttons() != Qt.LeftButton:
            return

        x, y = self._get_rel_coords(event)

        hue_ring_thickness = self._config['hue_ring_thickness']
        hue_inner_radius = self._hue_outer_radius * (1 - hue_ring_thickness)

        dist_from_centre = math.sqrt(x * x + y * y)
        if hue_inner_radius < dist_from_centre < self._hue_outer_radius:
            self._state = self._STATE_EDITING_HUE
            self._set_hue_from_coords(x, y)
            return

        if self._try_set_sv_from_coords(x, y, allow_any_point=False):
            self._state = self._STATE_EDITING_SV
            return

    def mouseReleaseEvent(self, event):
        self._state = self._STATE_IDLE
        self.colourSelected.emit()

    def _get_marker_colour(self, colour):
        intensity = self._style_mgr.get_colour_intensity(
                (colour.red(), colour.green(), colour.blue()))
        return QColor(0xff, 0xff, 0xff) if intensity < 127 else QColor(0, 0, 0)

    def _make_sv_gradients(self):
        radius = self._config['sv_gradient_radius']
        wh = radius * 2

        red = QImage(wh, wh, QImage.Format_ARGB32_Premultiplied)
        red.fill(0)

        # Make black -> white gradient
        painter = QPainter(red)
        painter.translate(radius, radius)
        painter.setPen(Qt.NoPen)

        white_grad = QLinearGradient(
                radius * math.cos(math.pi * 2 / 3), radius * math.sin(math.pi * 2 / 3),
                radius * (math.cos(math.pi * 4 / 3) + 1) / 2,
                radius * math.sin(math.pi * 4 / 3) / 2)
        white_grad.setColorAt(0, QColor(0xff, 0xff, 0xff))
        white_grad.setColorAt(1, QColor(0, 0, 0))
        painter.fillRect(-radius, -radius, wh, wh, QBrush(white_grad))
        painter.end()

        # Use the black -> white gradient as a basis for the other images
        yellow = QImage(red)
        green = QImage(red)
        magenta = QImage(red)

        # Add colour gradients with additive blending
        def fill_one_component_column(image, x, start_y, end_y, alpha_add, shift):
            mask = 0xffffffff ^ (0xff << shift)
            for y in range(start_y, end_y):
                argb = image.pixel(x, y)
                c = (argb >> shift) & 0xff
                c = min(0xff, c + alpha_add)
                argb = (argb & mask) + (c << shift)
                image.setPixel(x, y, argb)

        def fill_two_component_column(
                image, x, start_y, end_y, alpha_add, shift1, shift2):
            mask = 0xffffffff ^ (0xff << shift1) ^ (0xff << shift2)
            for y in range(start_y, end_y):
                argb = image.pixel(x, y)
                c1 = (argb >> shift1) & 0xff
                c2 = (argb >> shift2) & 0xff
                c1 = min(0xff, c1 + alpha_add)
                c2 = min(0xff, c2 + alpha_add)
                argb = (argb & mask) + (c1 << shift1) + (c2 << shift2)
                image.setPixel(x, y, argb)

        start_x = int(radius + radius * math.cos(math.pi * 2 / 3))
        end_x = wh
        col_count = end_x - start_x
        for x in range(start_x, end_x):
            col_index = (x - start_x)
            alpha_add_norm = min(max(0, col_index / col_count), 1)
            alpha_add = int(alpha_add_norm * 0xff)
            start_y = col_index // 2
            end_y = wh - (col_index // 2)

            fill_one_component_column(red, x, start_y, end_y, alpha_add, 16)
            fill_one_component_column(green, x, start_y, end_y, alpha_add, 8)
            fill_two_component_column(yellow, x, start_y, end_y, alpha_add, 16, 8)
            fill_two_component_column(magenta, x, start_y, end_y, alpha_add, 16, 0)

        # Create remaining gradients from existing ones
        cyan = yellow.rgbSwapped()
        blue = red.rgbSwapped()

        self._sv_gradients = [red, yellow, green, cyan, blue, magenta]

    def _clear_cache(self):
        self._hue_ring = None
        self._sv_triangle = None

    def paintEvent(self, event):
        width = self.width()
        height = self.height()

        hue_outer_radius = min(width, height) // 2
        if hue_outer_radius < 1:
            return

        hue_inner_radius = hue_outer_radius * (1 - self._config['hue_ring_thickness'])

        if hue_outer_radius != self._hue_outer_radius:
            self._hue_outer_radius = hue_outer_radius
            self._clear_cache()

        painter = QPainter(self)
        painter.translate(width // 2, height // 2)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        hue_ring = self._get_hue_ring()
        sv_triangle = self._get_sv_triangle(hue_inner_radius)

        painter.drawImage(-hue_inner_radius, -hue_inner_radius, sv_triangle)
        painter.drawImage(-hue_outer_radius, -hue_outer_radius, hue_ring)

        # Draw markers
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setBrush(Qt.NoBrush)

        # Hue marker
        hue_colour = QColor.fromHsvF(self._hue, 1, 1)
        hue_marker_colour = self._get_marker_colour(hue_colour)
        hue_angle = self._hue * 2 * math.pi
        hue_dir = QPointF(math.cos(hue_angle), -math.sin(hue_angle))
        hue_pen = QPen()
        hue_pen.setColor(hue_marker_colour)
        hue_pen.setWidth(self._config['hue_marker_thickness'])
        hue_pen.setCapStyle(Qt.FlatCap)
        painter.setPen(hue_pen)
        painter.drawLine(hue_dir * hue_inner_radius, hue_dir * hue_outer_radius)

        # SV marker
        sv_marker_colour = self._get_marker_colour(self._colour)
        sv_marker_radius = self._config['sv_marker_radius']

        sv_colour_pos, sv_black_pos, sv_white_pos = self._get_sv_corners()

        sv_val_start_pos = utils.lerp_val(sv_black_pos, sv_white_pos, self._value)
        sat_max_length, sat_dir = self._length_and_dir(sv_colour_pos - sv_white_pos)
        sat_length = sat_max_length * self._value
        sv_final_pos = sv_val_start_pos + (sat_dir * (sat_length * self._saturation))

        sv_pen = QPen()
        sv_pen.setColor(sv_marker_colour)
        sv_pen.setWidth(self._config['sv_marker_thickness'])
        painter.setPen(sv_pen)
        painter.drawEllipse(sv_final_pos, sv_marker_radius, sv_marker_radius)

    def _get_hue_ring(self):
        if self._hue_ring:
            return self._hue_ring

        diam = self._hue_outer_radius * 2
        cut_diam = diam * (1 - self._config['hue_ring_thickness'])
        cut_offset = (diam - cut_diam) / 2

        self._hue_ring = QImage(diam, diam, QImage.Format_ARGB32_Premultiplied)
        self._hue_ring.fill(0)

        hues = QConicalGradient()
        hues.setCenter(QPointF(self._hue_outer_radius, self._hue_outer_radius))
        hues.setColorAt(0,     QColor(0xff, 0,    0))
        hues.setColorAt(1 / 6, QColor(0xff, 0xff, 0))
        hues.setColorAt(2 / 6, QColor(0,    0xff, 0))
        hues.setColorAt(3 / 6, QColor(0,    0xff, 0xff))
        hues.setColorAt(4 / 6, QColor(0,    0,    0xff))
        hues.setColorAt(5 / 6, QColor(0xff, 0,    0xff))
        hues.setColorAt(1,     QColor(0xff, 0,    0))

        painter = QPainter(self._hue_ring)
        painter.setPen(Qt.NoPen)
        painter.setRenderHint(QPainter.Antialiasing)

        painter.setBrush(QBrush(hues))
        painter.drawEllipse(QRectF(0, 0, diam, diam))

        painter.setBrush(QColor(0xff, 0xff, 0xff))
        painter.setCompositionMode(QPainter.CompositionMode_DestinationOut)
        painter.drawEllipse(QRectF(cut_offset, cut_offset, cut_diam, cut_diam))

        return self._hue_ring

    def _get_sv_triangle(self, inner_radius):
        if self._sv_triangle:
            return self._sv_triangle

        # Get hue gradient
        hue_scaled = self._hue * 6
        hue_index1 = int(math.floor(hue_scaled)) % 6
        hue_index2 = (hue_index1 + 1) % 6
        hue2_alpha = hue_scaled - math.floor(hue_scaled)

        hue_grad = QImage(self._sv_gradients[hue_index1])
        hue2_grad = QImage(self._sv_gradients[hue_index2])

        hue_painter = QPainter(hue_grad)
        hue_painter.setOpacity(hue2_alpha)
        hue_painter.drawImage(0, 0, hue2_grad)
        hue_painter.end()

        # Draw the triangle
        wh = inner_radius * 2
        self._sv_triangle = QImage(wh, wh, QImage.Format_ARGB32)
        self._sv_triangle.fill(0)

        painter = QPainter(self._sv_triangle)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)
        painter.translate(inner_radius, inner_radius)
        painter.rotate(-self._hue * 360)
        painter.setPen(Qt.NoPen)

        ir = inner_radius

        painter.drawImage(QRectF(-ir, -ir, wh, wh), hue_grad)

        triangle = QPolygonF([
            QPointF(ir, 0),
            QPointF(ir * math.cos(math.pi * 2 / 3), ir * math.sin(math.pi * 2 / 3)),
            QPointF(ir * math.cos(math.pi * 4 / 3), ir * math.sin(math.pi * 4 / 3))])
        mask = QPolygonF(QRectF(-ir - 1, -ir - 1, wh + 2, wh + 2)).subtracted(triangle)
        painter.setBrush(QColor(0, 0, 0))
        painter.setCompositionMode(QPainter.CompositionMode_DestinationOut)
        painter.drawPolygon(mask)

        return self._sv_triangle

    def minimumSizeHint(self):
        return get_abs_window_size(0.15, 0.25)


class ColourCodeValidator(QValidator):

    def __init__(self):
        super().__init__()

    def validate(self, contents, pos):
        if (not contents) or (contents[0] != '#') or (len(contents) > 7):
            return (QValidator.Invalid, contents, pos)
        if not all(c in string.hexdigits for c in contents[1:]):
            return (QValidator.Invalid, contents, pos)
        if len(contents) < 7:
            return (QValidator.Intermediate, contents, pos)
        return (QValidator.Acceptable, contents, pos)


class ColourComparison(QWidget):

    def __init__(self):
        super().__init__()
        self._orig_colour = QColor(0, 0, 0)
        self._new_colour = QColor(0, 0, 0)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def update_style(self, style_mgr):
        self.setMinimumHeight(style_mgr.get_scaled_size(3))

    def set_original_colour(self, colour):
        self._orig_colour = colour
        self.update()

    def set_new_colour(self, colour):
        self._new_colour = colour
        self.update()

    def paintEvent(self, event):
        width = self.width()
        height = self.height()

        painter = QPainter(self)
        painter.fillRect(0, 0, width // 2, height, self._orig_colour)
        painter.fillRect(width // 2, 0, width // 2, height, self._new_colour)


class ColourEditor(QWidget):

    colourModified = Signal(str, str, name='colourModified')

    def __init__(self):
        super().__init__()
        self._key = None
        self._orig_colour = None
        self._new_colour = None

        self._selector = ColourSelector()

        self._code_editor = QLineEdit()
        self._code_editor.setValidator(ColourCodeValidator())

        self._comparison = ColourComparison()

        self._revert_button = QPushButton('Revert')
        self._accept_button = QPushButton('Done')

        # Code editor layout
        self._code_layout = QHBoxLayout()
        self._code_layout.setContentsMargins(0, 0, 0, 0)
        self._code_layout.setSpacing(2)
        self._code_layout.addWidget(QLabel('Code:'))
        self._code_layout.addWidget(self._code_editor, 1)

        # Colour comparison layout
        orig_label = QLabel('Original')
        orig_label.setAlignment(Qt.AlignHCenter)
        orig_label.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)
        new_label = QLabel('New')
        new_label.setAlignment(Qt.AlignHCenter)
        new_label.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)

        self._comp_label_layout = QHBoxLayout()
        self._comp_label_layout.setContentsMargins(0, 0, 0, 0)
        self._comp_label_layout.setSpacing(2)
        self._comp_label_layout.addWidget(orig_label)
        self._comp_label_layout.addWidget(new_label)
        self._comp_layout = QVBoxLayout()
        self._comp_layout.setContentsMargins(0, 0, 0, 0)
        self._comp_layout.setSpacing(2)
        self._comp_layout.addLayout(self._comp_label_layout)
        self._comp_layout.addWidget(self._comparison)

        # Button layout
        self._button_layout = QHBoxLayout()
        self._button_layout.setContentsMargins(0, 0, 0, 0)
        self._button_layout.setSpacing(2)
        self._button_layout.addWidget(self._revert_button)
        self._button_layout.addWidget(self._accept_button)

        # Top layout
        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._selector)
        v.addLayout(self._code_layout)
        v.addLayout(self._comp_layout)
        v.addLayout(self._button_layout)
        self.setLayout(v)

        self._selector.colourChanged.connect(self._change_colour)
        self._selector.colourSelected.connect(self._select_colour)

        self._code_editor.textEdited.connect(self._change_code)

        self._revert_button.clicked.connect(self._revert)

        self._accept_button.clicked.connect(self.hide)

    def update_style(self, style_mgr):
        self._selector.update_style(style_mgr)
        self._comparison.update_style(style_mgr)

        spacing = style_mgr.get_scaled_size_param('small_padding')
        self._code_layout.setSpacing(spacing)
        self._comp_label_layout.setSpacing(spacing)
        self._comp_layout.setSpacing(spacing)
        self._button_layout.setSpacing(spacing)

        margin = style_mgr.get_scaled_size_param('medium_padding')
        top_spacing = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(top_spacing)

    def set_colour(self, key, colour):
        self._key = key
        self._orig_colour = colour
        self._new_colour = colour

        desc = _COLOUR_DESCS_DICT[key]
        if not desc.startswith(('Note on ', 'Note off ', 'Hit ')):
            desc = desc[0].lower() + desc[1:]
        self.setWindowTitle('Colour of ' + desc)

        self._selector.set_colour(self._orig_colour)

        code = utils.get_str_from_colour(self._orig_colour)
        old_block = self._code_editor.blockSignals(True)
        self._code_editor.setText(code)
        self._code_editor.blockSignals(old_block)

        self._comparison.set_original_colour(self._orig_colour)
        self._comparison.set_new_colour(self._new_colour)

        self._revert_button.setEnabled(False)

    def _update_colour(self, new_colour):
        self._new_colour = new_colour
        self._selector.set_colour(self._new_colour)
        self._comparison.set_new_colour(self._new_colour)
        self._revert_button.setEnabled(self._new_colour != self._orig_colour)

        old_block = self._code_editor.blockSignals(True)
        new_code = utils.get_str_from_colour(self._new_colour)
        if new_code != self._code_editor.text():
            self._code_editor.setText(new_code)
        self._code_editor.blockSignals(old_block)

    def _change_colour(self):
        colour = self._selector.get_colour()
        self._update_colour(colour)

    def _select_colour(self):
        colour = self._selector.get_colour()
        self._update_colour(colour)
        text = utils.get_str_from_colour(colour)
        self.colourModified.emit(self._key, text)

    def _change_code(self, text):
        if self._code_editor.hasAcceptableInput():
            self._update_colour(utils.get_colour_from_str(text))
            self.colourModified.emit(self._key, text)

    def _revert(self):
        orig_code = utils.get_str_from_colour(self._orig_colour)
        self.colourModified.emit(self._key, orig_code)
        self.hide()

    def sizeHint(self):
        return QSize(256, 64)


class Colours(QTreeView, Updater):

    def __init__(self):
        super().__init__()
        self._model = None

        self._colour_editor = ColourEditor()

        header = self.header()
        header.setStretchLastSection(False)
        self.setHeaderHidden(True)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_all)

        self._model = ColoursModel()
        self.add_to_updaters(self._model)
        self.setModel(self._model)

        for index in self._model.get_colour_indices():
            node = index.internalPointer()
            if isinstance(node, ColourModel):
                key = node.get_key()
                button = ColourButton(key)
                button.colourSelected.connect(self._open_colour_editor)
                self.setIndexWidget(index, button)

        self.expandAll()

        self.resizeColumnToContents(0)
        self.setColumnWidth(1, 48)

        self._colour_editor.colourModified.connect(self._update_colour)

        self._update_all()

    def _update_all(self):
        self._update_buttons()
        self._colour_editor.update_style(self._ui_model.get_style_manager())

    def _update_buttons(self):
        style_mgr = self._ui_model.get_style_manager()

        for index in self._model.get_colour_indices():
            button = self.indexWidget(index)
            if button:
                key = button.get_key()
                colour = style_mgr.get_style_param(key)
                button.set_colour(colour)
                button.update_style(style_mgr)

    def _open_colour_editor(self, key):
        style_mgr = self._ui_model.get_style_manager()
        cur_colour_str = style_mgr.get_style_param(key)
        cur_colour = utils.get_colour_from_str(cur_colour_str)
        self._colour_editor.set_colour(key, cur_colour)
        self._colour_editor.show()
        self._colour_editor.activateWindow()
        self._colour_editor.raise_()

    def _update_colour(self, key, colour_code):
        style_mgr = self._ui_model.get_style_manager()
        style_mgr.set_style_param(key, colour_code)
        self._updater.signal_update('signal_style_changed')

    def hideEvent(self, event):
        self._colour_editor.hide()


