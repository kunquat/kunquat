# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from header import Header
from ruler import Ruler
from utils import *
from view import View
import tstamp


class Sheet(QAbstractScrollArea):

    def __init__(self, config={}):
        QAbstractScrollArea.__init__(self)
        self._ui_model = None
        self._updater = None

        # Widgets
        self.setViewport(View())

        self._corner = QWidget()

        self._ruler = Ruler()
        self._header = Header()

        # Config
        self._config = None
        self._set_config(config)

        # Layout
        g = QGridLayout()
        g.setSpacing(0)
        g.setMargin(0)
        g.addWidget(self._corner, 0, 0)
        g.addWidget(self._ruler, 1, 0)
        g.addWidget(self._header, 0, 1)
        g.addWidget(self.viewport(), 1, 1)
        self.setLayout(g)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self._col_width = self._config['col_width']
        self._px_per_beat = self._config['px_per_beat']

        # XXX: testing
        '''
        patterns = [
                {
                    'length': tstamp.Tstamp(0.5),
                    'columns': [[] for _ in xrange(COLUMN_COUNT)],
                },
                {
                    'length': tstamp.Tstamp(8),
                    'columns': [[] for _ in xrange(COLUMN_COUNT)],
                },
                ]
        patterns[0]['columns'][0].append([tstamp.Tstamp(0.46), ['cn+', '300']])
        patterns[1]['columns'][0].append([tstamp.Tstamp(0.03), ['.i', '0']])
        patterns[1]['columns'][0].append([tstamp.Tstamp(7.96), ['cn-', None]])
        pat_lengths = [p['length'] for p in patterns]
        self._total_height_px = (self._get_total_height(pat_lengths) +
                self._config['tr_height'])
        self._ruler.set_pattern_lengths(pat_lengths)
        self.viewport().set_patterns(patterns)
        '''

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all_patterns()

    def _update_all_patterns(self):
        module = self._ui_model.get_module()

        album = module.get_album()
        if not album:
            all_patterns = []
        else:
            track_count = album.get_track_count()
            songs = [album.get_song_by_track(i) for i in xrange(track_count)]
            all_patterns = []
            for song in songs:
                system_count = song.get_system_count()
                pattern_instances = [song.get_pattern_instance(i)
                        for i in xrange(system_count)]
                patterns = [pinst.get_pattern() for pinst in pattern_instances]
                all_patterns.extend(patterns)

        pat_lengths = [tstamp.Tstamp(p.get_length()) for p in all_patterns]

        self._total_height_px = (self._get_total_height(pat_lengths) +
                self._config['tr_height'])
        self._ruler.set_patterns(all_patterns)
        self.viewport().set_patterns(all_patterns)
        self._ruler.update()
        self.viewport().update()

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        for subcfg in ('ruler', 'header'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        self._header.set_config(self._config['header'])
        self._ruler.set_config(self._config['ruler'])

        header_height = self._header.minimumSizeHint().height()
        ruler_width = self._ruler.sizeHint().width()

        self.setViewportMargins(
                ruler_width,
                header_height,
                0, 0)

        self._corner.setFixedSize(
                ruler_width,
                header_height)
        self._header.setFixedHeight(header_height)
        self._ruler.setFixedWidth(ruler_width)

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        self._config['tr_height'] = fm.tightBoundingRect('Ag').height() + 1

        self.viewport().set_config(self._config)

    def _perform_updates(self, signals):
        if 'signal_module' in signals:
            self._update_all_patterns()

    def _get_total_height(self, pat_lengths):
        height = sum(pat_height(pl, self._px_per_beat) for pl in pat_lengths)
        height -= len(pat_lengths) - 1
        # TODO: add trigger row height
        return height

    def paintEvent(self, ev):
        self.viewport().paintEvent(ev)

    def resizeEvent(self, ev):
        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setPageStep(vp_height)
        vscrollbar.setRange(0, self._total_height_px - vp_height)

        vp_width = self.viewport().width()
        max_visible_cols = vp_width // self._col_width
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMN_COUNT - max_visible_cols)

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().value()

        self._header.set_first_column(hvalue)
        self._ruler.set_px_offset(vvalue)

        vp = self.viewport()
        vp.set_first_column(hvalue)
        vp.set_px_offset(vvalue)


