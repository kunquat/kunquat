# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import count
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.kunquat.limits import *
import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.ui.model.tstamp as tstamp
from playbutton import PlayButton
from playpatternbutton import PlayPatternButton
from recordbutton import RecordButton
from silencebutton import SilenceButton
from notationselect import NotationSelect
import utils


class TopControls(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._play_button = PlayButton()
        self._play_pattern_button = PlayPatternButton()
        self._play_from_cursor_button = PlayFromCursorButton()
        self._record_button = RecordButton()
        self._silence_button = SilenceButton()
        self._playback_pos = PlaybackPosition()
        self._interactivity_button = InteractivityButton()
        self._notation_select = NotationSelect()

        self.addWidget(self._play_button)
        self.addWidget(self._play_pattern_button)
        self.addWidget(self._play_from_cursor_button)
        if cmdline.get_experimental():
            self.addWidget(self._record_button)
        self.addWidget(self._silence_button)
        self.addSeparator()
        self.addWidget(self._playback_pos)
        self.addSeparator()
        self.addWidget(self._interactivity_button)
        self.addSeparator()
        self.addWidget(self._notation_select)

    def set_ui_model(self, ui_model):
        self._play_button.set_ui_model(ui_model)
        self._play_pattern_button.set_ui_model(ui_model)
        self._play_from_cursor_button.set_ui_model(ui_model)
        self._record_button.set_ui_model(ui_model)
        self._silence_button.set_ui_model(ui_model)
        self._playback_pos.set_ui_model(ui_model)
        self._interactivity_button.set_ui_model(ui_model)
        self._notation_select.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._notation_select.unregister_updaters()
        self._interactivity_button.unregister_updaters()
        self._playback_pos.unregister_updaters()
        self._silence_button.unregister_updaters()
        self._record_button.unregister_updaters()
        self._play_from_cursor_button.unregister_updaters()
        self._play_pattern_button.unregister_updaters()
        self._play_button.unregister_updaters()


class PlayFromCursorButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Play from Cursor')
        self.setToolTip('Play from Cursor (Alt + Comma)')
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('play_from_cursor')
        icon = QIcon(icon_path)
        self.setIcon(icon)
        QObject.connect(self, SIGNAL('clicked()'), self._ui_model.play_from_cursor)

    def unregister_updaters(self):
        pass


class PlaybackPosition(QWidget):

    _NUM_FONT = QFont(QFont().defaultFamily(), 16)
    _NUM_FONT.setWeight(QFont.Bold)

    _SUB_FONT = QFont(QFont().defaultFamily(), 8)
    _SUB_FONT.setWeight(QFont.Bold)

    _REM_FONT = QFont(QFont().defaultFamily(), 12)
    _REM_FONT.setWeight(QFont.Bold)

    _DEFAULT_CONFIG = {
        'padding'        : 5,
        'spacing'        : 2,
        'icon_width'     : 19,
        'num_font'       : _NUM_FONT,
        'sub_font'       : _SUB_FONT,
        'rem_font'       : _REM_FONT,
        'narrow_factor'  : 0.5,
        'track_digits'   : [1, len(str(TRACKS_MAX - 1))],
        'system_digits'  : [2, len(str(SYSTEMS_MAX - 1))],
        'pat_digits'     : [2, len(str(PATTERNS_MAX - 1))],
        'pat_inst_digits': [1, len(str(PAT_INSTANCES_MAX - 1))],
        'ts_beat_digits' : [2, 3],
        'ts_rem_digits'  : [1, 1],
        'bg_colour'      : QColor(0, 0, 0),
        'fg_colour'      : QColor(0x66, 0xdd, 0x66),
        'stopped_colour' : QColor(0x55, 0x55, 0x55),
        'record_colour'  : QColor(0xdd, 0x44, 0x33),
        'infinite_colour': QColor(0xff, 0xdd, 0x55),
    }

    _STOPPED = 'stopped'
    _PLAYING = 'playing'
    _RECORDING = 'recording'

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None

        self._widths = None
        self._min_height = 0

        self._state = (self._STOPPED, False)

        self._state_icons = {}
        self._inf_image = None

        self._baseline_offsets = {}

        self._config = None
        self._set_config({})

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _set_config(self, config):
        self._config = self._DEFAULT_CONFIG
        self._config.update(config)

        self._state_icons = {}
        self._set_dimensions()

        self._inf_image = self._get_infinity_image()

    def _set_dimensions(self):
        default_fm = QFontMetrics(self._config['num_font'], self)
        sub_fm = QFontMetrics(self._config['sub_font'], self)
        rem_fm = QFontMetrics(self._config['rem_font'], self)

        padding = self._config['padding']
        spacing = self._config['spacing']
        track_width = self._get_field_width(self._config['track_digits'], default_fm)
        system_width = self._get_field_width(self._config['system_digits'], default_fm)
        pat_width = self._get_field_width(self._config['pat_digits'], default_fm)
        inst_width = self._get_field_width(self._config['pat_inst_digits'], sub_fm)
        beat_width = self._get_field_width(self._config['ts_beat_digits'], default_fm)
        dot_width = default_fm.boundingRect('.').width()
        rem_width = self._get_field_width(self._config['ts_rem_digits'], rem_fm)

        self._widths = [
            padding,
            self._config['icon_width'],
            spacing,
            track_width,
            spacing,
            system_width,
            spacing,
            pat_width,
            inst_width,
            spacing,
            beat_width,
            dot_width,
            rem_width,
            padding]

        self._min_height = default_fm.boundingRect('Ag').height() + padding * 2

    def _get_field_width(self, digit_counts, fm):
        min_digits, max_digits = digit_counts
        min_str = '9' * min_digits
        max_str = '9' * max_digits
        min_rect = fm.boundingRect(min_str)
        max_rect = fm.boundingRect(max_str)
        min_width = min_rect.width()
        scaled_max_width = max_rect.width() * self._config['narrow_factor']
        return max(min_width, scaled_max_width)

    def _get_current_state(self):
        playback_manager = self._ui_model.get_playback_manager()
        is_playing = playback_manager.is_playback_active()
        is_infinite = playback_manager.get_infinite_mode()
        is_recording = playback_manager.is_recording()

        playback_state = 'playing' if is_playing else 'stopped'
        if is_recording:
            playback_state = 'recording'

        return (playback_state, is_infinite)

    def _perform_updates(self, signals):
        state = self._get_current_state()
        if (state != self._state) or (state[0] != self._STOPPED):
            self._state = state
            self.update()

    def minimumSizeHint(self):
        return QSize(sum(self._widths), self._min_height)

    def _get_icon_rect(self, width_norm):
        icon_width = self._config['icon_width']
        side_length = icon_width * width_norm
        offset = (icon_width - side_length) * 0.5
        rect = QRectF(offset, offset, side_length, side_length)
        return rect

    def _draw_stop_icon_shape(self, painter, colour):
        painter.setPen(Qt.NoPen)
        painter.setBrush(colour)
        painter.setRenderHint(QPainter.Antialiasing)

        rect = self._get_icon_rect(0.85)

        painter.drawRect(rect)

    def _draw_play_icon_shape(self, painter, colour):
        painter.setPen(Qt.NoPen)
        painter.setBrush(colour)
        painter.setRenderHint(QPainter.Antialiasing)

        rect = self._get_icon_rect(0.85)

        middle_y = rect.y() + (rect.height() / 2.0)
        shape = QPolygonF([
            QPointF(rect.x(), rect.y()),
            QPointF(rect.x(), rect.y() + rect.height()),
            QPointF(rect.x() + rect.width(), rect.y() + (rect.height() * 0.5))])

        painter.drawPolygon(shape)

    def _draw_record_icon_shape(self, painter, colour):
        painter.setPen(Qt.NoPen)
        painter.setBrush(colour)
        painter.setRenderHint(QPainter.Antialiasing)

        rect = self._get_icon_rect(0.85)

        painter.drawEllipse(rect)

    def _get_infinity_image(self):
        icon_width = self._config['icon_width']

        image = QImage(icon_width, icon_width, QImage.Format_ARGB32_Premultiplied)
        image.fill(0)
        painter = QPainter(image)
        painter.setPen(Qt.NoPen)
        painter.setRenderHint(QPainter.Antialiasing)

        center_y = icon_width * 0.5
        height_norm = 0.4

        def draw_drop_shape(round_x_norm, sharp_x_norm):
            round_x = round_x_norm * icon_width
            sharp_x = sharp_x_norm * icon_width
            width = abs(round_x - sharp_x)
            height = width * height_norm

            points = []
            point_count = 15
            for i in xrange(point_count):
                norm_t = i / float(point_count - 1)
                if norm_t < 0.5:
                    t = utils.lerp_val(math.pi, math.pi * 1.5, norm_t * 2)
                else:
                    t = utils.lerp_val(math.pi * 4.5, math.pi * 5, (norm_t - 0.5) * 2)

                norm_x = min(max(0, math.cos(t) + 1), 1)
                norm_y = math.sin(t * 2)
                x = utils.lerp_val(round_x, sharp_x, norm_x)
                y = center_y + (norm_y * height)

                points.append(QPointF(x, y))

            shape = QPolygonF(points)
            painter.drawPolygon(shape)

        # Outline
        painter.setBrush(QColor(0, 0, 0))
        draw_drop_shape(-0.03, 0.68)
        draw_drop_shape(1.03, 0.32)

        # Coloured fill
        painter.setBrush(self._config['infinite_colour'])
        draw_drop_shape(0, 0.65)
        draw_drop_shape(1, 0.35)

        # Hole of the coloured fill
        painter.setBrush(QColor(0, 0, 0))
        draw_drop_shape(0.1, 0.45)
        draw_drop_shape(0.9, 0.55)

        # Transparent hole
        painter.setBrush(QColor(0xff, 0xff, 0xff))
        painter.setCompositionMode(QPainter.CompositionMode_DestinationOut)
        draw_drop_shape(0.13, 0.42)
        draw_drop_shape(0.87, 0.58)

        painter.end()

        return image

    def _draw_state_icon(self, painter):
        if self._state not in self._state_icons:
            pixmap = QPixmap(self._config['icon_width'], self._config['icon_width'])
            pixmap.fill(self._config['bg_colour'])
            img_painter = QPainter(pixmap)

            playback_state, is_infinite = self._state
            if playback_state == self._STOPPED:
                self._draw_stop_icon_shape(img_painter, self._config['stopped_colour'])
            elif playback_state == self._PLAYING:
                self._draw_play_icon_shape(img_painter, self._config['fg_colour'])
            elif playback_state == self._RECORDING:
                self._draw_record_icon_shape(img_painter, self._config['record_colour'])

            if is_infinite:
                assert self._inf_image
                img_painter.drawImage(0, 0, self._inf_image)

            self._state_icons[self._state] = pixmap

        icon = self._state_icons[self._state]
        painter.drawPixmap(0, -icon.height() // 2, icon)

    def _get_baseline_offset(self, font):
        key = font.pointSize()
        if key not in self._baseline_offsets:
            fm = QFontMetrics(font)
            self._baseline_offsets[key] = fm.tightBoundingRect('0').height()

        return self._baseline_offsets[key]

    def _draw_number_str(
            self,
            painter,
            num_str,
            digit_counts,
            font,
            colour,
            alignment=Qt.AlignCenter):
        min_digits, max_digits = digit_counts

        painter.save()

        if len(num_str) > min_digits:
            painter.scale(self._config['narrow_factor'], 1)

        if font != self._config['num_font']:
            # Match the text baseline of the default font
            default_baseline = self._get_baseline_offset(self._config['num_font'])
            cur_baseline = self._get_baseline_offset(font)
            rect = painter.clipBoundingRect()
            rect.translate(0, default_baseline - cur_baseline - 1)
            painter.setClipRect(rect)

        painter.setFont(font)
        painter.setPen(colour)
        text_option = QTextOption(alignment)
        rect = painter.clipBoundingRect()
        painter.drawText(rect, num_str, text_option)

        painter.restore()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        painter.translate(QPoint(0, self.height() // 2))

        width_index = count()
        def shift_x():
            cur_width_index = next(width_index)
            painter.translate(QPoint(self._widths[cur_width_index], 0))
            cur_width = self._widths[cur_width_index + 1]
            painter.setClipRect(QRectF(
                0, -self.height() * 0.5, cur_width, self.height()))

        # State icon
        shift_x()
        self._draw_state_icon(painter)

        playback_manager = self._ui_model.get_playback_manager()
        track_num, system_num, row_ts = playback_manager.get_playback_position()

        # Track number
        shift_x()
        shift_x()
        self._draw_number_str(
                painter,
                str(track_num) if track_num >= 0 else '-',
                self._config['track_digits'],
                self._config['num_font'],
                self._config['fg_colour'])

        # System number
        shift_x()
        shift_x()
        self._draw_number_str(
                painter,
                str(system_num) if system_num >= 0 else '-',
                self._config['system_digits'],
                self._config['num_font'],
                self._config['fg_colour'])

        # Pattern instance
        pat_num = 0
        inst_num = 0
        album = self._ui_model.get_module().get_album()
        if album.get_existence():
            song = album.get_song_by_track(track_num)
            if song.get_existence():
                pinst = song.get_pattern_instance(system_num)
                pat_num = pinst.get_pattern_num()
                inst_num = pinst.get_instance_num()

        shift_x()
        shift_x()
        self._draw_number_str(
                painter,
                str(pat_num),
                self._config['pat_digits'],
                self._config['num_font'],
                self._config['fg_colour'],
                Qt.AlignRight | Qt.AlignVCenter)

        shift_x()
        self._draw_number_str(
                painter,
                str(inst_num),
                self._config['pat_inst_digits'],
                self._config['sub_font'],
                self._config['fg_colour'],
                Qt.AlignLeft | Qt.AlignVCenter)

        # Timestamp
        beats, rem = row_ts
        rem_norm = int(
                float(rem / float(tstamp.BEAT)) * (10**self._config['ts_rem_digits'][0]))

        shift_x()
        shift_x()
        self._draw_number_str(
                painter,
                str(beats),
                self._config['ts_beat_digits'],
                self._config['num_font'],
                self._config['fg_colour'],
                Qt.AlignRight | Qt.AlignVCenter)

        shift_x()
        painter.setFont(self._config['num_font'])
        painter.setPen(self._config['fg_colour'])
        painter.drawText(painter.clipBoundingRect(), '.', QTextOption(Qt.AlignCenter))

        shift_x()
        self._draw_number_str(
                painter,
                str(rem_norm),
                self._config['ts_rem_digits'],
                self._config['rem_font'],
                self._config['fg_colour'],
                Qt.AlignLeft | Qt.AlignVCenter)

        end = time.time()
        elapsed = end - start
        print('Playback position view updated in {:.2f} ms'.format(elapsed * 1000))


class InteractivityButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Interactivity')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        QObject.connect(self, SIGNAL('clicked()'), self._show_ia_window)

    def unregister_updaters(self):
        pass

    def _show_ia_window(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_interactivity_controls()


