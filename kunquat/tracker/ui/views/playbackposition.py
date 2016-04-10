# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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
import kunquat.tracker.ui.model.tstamp as tstamp
from . import utils


class PlaybackPosition(QWidget):

    _NUM_FONT = QFont(QFont().defaultFamily(), 16)
    _NUM_FONT.setWeight(QFont.Bold)

    _SUB_FONT = QFont(QFont().defaultFamily(), 8)
    _SUB_FONT.setWeight(QFont.Bold)

    _REM_FONT = QFont(QFont().defaultFamily(), 12)
    _REM_FONT.setWeight(QFont.Bold)

    _TITLE_FONT = QFont(QFont().defaultFamily(), 5)
    _TITLE_FONT.setWeight(QFont.Bold)

    _DEFAULT_CONFIG = {
        'padding_x'      : 9,
        'padding_y'      : 0,
        'spacing'        : 2,
        'icon_width'     : 19,
        'num_font'       : _NUM_FONT,
        'sub_font'       : _SUB_FONT,
        'rem_font'       : _REM_FONT,
        'title_font'     : _TITLE_FONT,
        'narrow_factor'  : 0.6,
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
        'title_colour'   : QColor(0x77, 0x77, 0x77),
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

        self._glyphs = {}
        self._glyph_sizes = {}

        self._titles = {}

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
        self._inf_image = self._get_infinity_image()

        self._glyphs = {}
        self._draw_glyphs()
        self._set_dimensions()

        self._draw_titles()

    def _draw_glyphs(self):
        font_names = ('num_font', 'sub_font', 'rem_font')

        for font_name in font_names:
            font = self._config[font_name]
            fm = QFontMetrics(font, self)

            chars = '.-0123456789'

            normal_width = max(fm.tightBoundingRect(c).width() for c in chars)
            height = fm.boundingRect('0').height()

            text_option = QTextOption(Qt.AlignCenter)

            for width_factor in (1, self._config['narrow_factor']):
                cur_glyphs = {}
                width = math.ceil(width_factor * normal_width)
                rect = QRectF(0, 0, normal_width, height)
                for c in chars:
                    pixmap = QPixmap(width - 1, height)
                    pixmap.fill(self._config['bg_colour'])
                    painter = QPainter(pixmap)
                    painter.scale(width_factor, 1)
                    painter.setFont(self._config[font_name])
                    painter.setPen(self._config['fg_colour'])
                    painter.drawText(rect, c, text_option)
                    cur_glyphs[c] = pixmap
                self._glyphs[(font_name, width_factor)] = cur_glyphs

                self._glyph_sizes[(font_name, width_factor)] = (width - 1, height)

    def _draw_titles(self):
        font = self._config['title_font']
        fm = QFontMetrics(font, self)
        text_option = QTextOption(Qt.AlignLeft | Qt.AlignTop)

        for title in ('track', 'system', 'pattern', 'row'):
            vis_text = title.upper()
            rect = fm.tightBoundingRect(vis_text)

            pixmap = QPixmap(rect.width(), rect.height())
            pixmap.fill(self._config['bg_colour'])
            painter = QPainter(pixmap)
            painter.setFont(font)
            painter.setPen(self._config['title_colour'])
            painter.drawText(0, rect.height(), vis_text)

            self._titles[title] = pixmap

    def _set_dimensions(self):
        default_fm = QFontMetrics(self._config['num_font'], self)

        padding_x = self._config['padding_x']
        spacing = self._config['spacing']
        track_width = self._get_field_width(self._config['track_digits'], 'num_font')
        system_width = self._get_field_width(self._config['system_digits'], 'num_font')
        pat_width = self._get_field_width(self._config['pat_digits'], 'num_font')
        inst_width = self._get_field_width(self._config['pat_inst_digits'], 'sub_font')
        beat_width = self._get_field_width(self._config['ts_beat_digits'], 'num_font')
        dot_width = default_fm.boundingRect('.').width()
        rem_width = self._get_field_width(self._config['ts_rem_digits'], 'rem_font')

        self._widths = [
            padding_x,
            self._config['icon_width'],
            spacing * 2,
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
            padding_x]

        padding_y = self._config['padding_y']
        self._min_height = default_fm.boundingRect('Ag').height() + padding_y * 2

    def _get_field_width(self, digit_counts, font_name):
        min_digits, max_digits = digit_counts
        normal_width, _ = self._glyph_sizes[(font_name, 1)]
        narrow_width, _ = self._glyph_sizes[(font_name, self._config['narrow_factor'])]
        min_width = min_digits * normal_width
        scaled_max_width = max_digits * narrow_width
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

    def _draw_str(
            self,
            painter,
            num_str,
            digit_counts,
            font_name,
            alignment=Qt.AlignCenter):
        min_digits, max_digits = digit_counts

        painter.save()

        char_count = len(num_str)
        width_factor = 1 if char_count <= min_digits else self._config['narrow_factor']

        glyphs = self._glyphs[(font_name, width_factor)]
        assert glyphs
        width, height = self._glyph_sizes[(font_name, width_factor)]

        total_width = width * char_count

        rect = painter.clipBoundingRect()
        if alignment == Qt.AlignCenter:
            start_x = (rect.width() - total_width) * 0.5
        elif alignment == Qt.AlignLeft:
            start_x = 0
        elif alignment == Qt.AlignRight:
            start_x = rect.width() - total_width
        else:
            assert False, 'Unexpected alignment: {}'.format(alignment)

        x = start_x
        y = -height // 2

        if font_name != 'num_font':
            _, default_height = self._glyph_sizes[('num_font', width_factor)]
            y += (default_height - height) // 2 - 1

        for c in num_str:
            glyph = glyphs[c]
            painter.drawPixmap(x, y, glyph)
            x += width

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

        title_y = self.height() * 0.3

        # State icon
        shift_x()
        self._draw_state_icon(painter)

        playback_manager = self._ui_model.get_playback_manager()
        track_num, system_num, row_ts = playback_manager.get_playback_position()

        # Track number
        shift_x()
        shift_x()
        self._draw_str(
                painter,
                str(track_num) if track_num >= 0 else '-',
                self._config['track_digits'],
                'num_font')

        painter.setClipping(False)
        painter.drawPixmap(0, title_y, self._titles['track'])

        # System number
        shift_x()
        shift_x()
        self._draw_str(
                painter,
                str(system_num) if system_num >= 0 else '-',
                self._config['system_digits'],
                'num_font')

        painter.setClipping(False)
        painter.drawPixmap(2, title_y, self._titles['system'])

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
        self._draw_str(
                painter,
                str(pat_num),
                self._config['pat_digits'],
                'num_font',
                Qt.AlignRight)

        shift_x()
        self._draw_str(
                painter,
                str(inst_num),
                self._config['pat_inst_digits'],
                'sub_font',
                Qt.AlignLeft)

        painter.setClipping(False)
        painter.drawPixmap(-20, title_y, self._titles['pattern'])

        # Timestamp
        beats, rem = row_ts
        rem_norm = int(
                float(rem / float(tstamp.BEAT)) * (10**self._config['ts_rem_digits'][0]))

        shift_x()
        shift_x()
        self._draw_str(
                painter,
                str(beats),
                self._config['ts_beat_digits'],
                'num_font',
                Qt.AlignRight)

        shift_x()
        self._draw_str(
                painter,
                '.',
                [1, 1],
                'num_font')

        shift_x()
        self._draw_str(
                painter,
                str(rem_norm),
                self._config['ts_rem_digits'],
                'rem_font',
                Qt.AlignLeft)

        painter.setClipping(False)
        painter.drawPixmap(-10, title_y, self._titles['row'])

        end = time.time()
        elapsed = end - start
        #print('Playback position view updated in {:.2f} ms'.format(elapsed * 1000))


