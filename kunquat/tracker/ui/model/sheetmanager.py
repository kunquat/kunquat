# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class SheetManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._updater = None

        self

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def set_zoom(self, zoom):
        old_zoom = self._session.get_sheet_zoom()
        self._session.set_sheet_zoom(zoom)
        if self._session.get_sheet_zoom() != old_zoom:
            self._updater.signal_update(set(['signal_sheet_zoom']))

    def set_zoom_range(self, minimum, maximum):
        old_zoom = self._session.get_sheet_zoom()
        self._session.set_sheet_zoom_range(minimum, maximum)
        signals = set(['signal_sheet_zoom_range'])
        if self._session.get_sheet_zoom() != old_zoom:
            signals.add('signal_sheet_zoom')
        self._updater.signal_update(signals)

    def get_zoom(self):
        return self._session.get_sheet_zoom()

    def get_zoom_range(self):
        return self._session.get_sheet_zoom_range()

    def set_column_width(self, width):
        old_width = self._session.get_sheet_column_width()
        self._session.set_sheet_column_width(width)
        if self._session.get_sheet_column_width() != old_width:
            self._updater.signal_update(set(['signal_sheet_column_width']))

    def set_column_width_range(self, minimum, maximum):
        old_width = self._session.get_sheet_column_width()
        self._session.set_sheet_column_width_range(minimum, maximum)
        if self._session.get_sheet_column_width() != old_width:
            self._updater.signal_update(set(['signal_sheet_column_width']))

    def get_column_width(self):
        return self._session.get_sheet_column_width()

    def get_column_width_range(self):
        return self._session.get_sheet_column_width_range()


