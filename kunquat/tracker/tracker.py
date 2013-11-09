# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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

import re
import sys
import time
import json
import tarfile

from views.mainwindow import MainWindow

from kunquat.tracker.model.driver_manager import DriverManager
from kunquat.tracker.model.stat_manager import StatManager
from kunquat.tracker.model.ui_manager import UiManager
from kunquat.tracker.model.playback_manager import PlaybackManager
from kunquat.tracker.model.module import Module
from kunquat.tracker.model.uimodel import UiModel
from kunquat.tracker.model.updater import Updater

from kunquat.tracker.backend.store import Store

from kunquat.tracker.audio.drivers.silentaudio import Silentaudio
from kunquat.tracker.audio.drivers.pulseaudio import Pulseaudio
from kunquat.tracker.audio.drivers.pushaudio import Pushaudio
from kunquat.tracker.audio.drivers.nullaudio import Nullaudio

class Tracker():

    def __init__(self):
        self.previous = 0
        self.updater = Updater()
        self._store = Store()

    def create_ui_model(self):
        drivers = [Nullaudio, Pulseaudio, Pushaudio, Silentaudio]
        driver_manager = DriverManager()
        driver_manager.set_drivers(drivers)
        stat_manager = StatManager()
        ui_manager = UiManager()
        playback_manager = PlaybackManager()
        module = Module()
        ui_model = UiModel()
        ui_model.set_driver_manager(driver_manager)
        ui_model.set_stat_manager(stat_manager)
        ui_model.set_ui_manager(ui_manager)
        ui_model.set_playback_manager(playback_manager)
        ui_model.set_module(module)
        ui_model.set_updater(self.updater)
        ui_model.set_store(self._store)
        return ui_model

    def update(self):
        self.current = time.time()
        s = self.current - self.previous
        ms = s * 1000
        lag = ms - 10
        if lag > 1:
            print lag
        self.previous = self.current
        self.updater.perform_updates()

    def _remove_prefix(self, path, prefix):
        preparts = prefix.split('/')
        keyparts = path.split('/')
        for pp in preparts:
            kp = keyparts.pop(0)
            if pp != kp:
                 return None
        return '/'.join(keyparts)

    def load_module(self, module_path):
        values = dict()
        if module_path[-4:] in ['.kqt', '.bz2']:
            prefix = 'kqtc00'
            tfile = tarfile.open(module_path, format=tarfile.USTAR_FORMAT)
            members = tfile.getmembers()
            member_count = len(members)
            #assert self._frontend
            #self._frontend.update_import_progress(0, member_count)
            for i, entry in zip(range(member_count), members):
                QApplication.processEvents()
                tarpath = entry.name
                key = self._remove_prefix(tarpath, prefix)
                assert (key != None) #TODO broken file exception
                if entry.isfile():
                    value = tfile.extractfile(entry).read()

                    m = re.match('^ins_([0-9]{2})/p_manifest.json$', key)
                    if m:
                        instrument_number = int(m.group(1))
                        #self._frontend.update_instrument_existence(instrument_number, True)

                    m = re.match('^ins_([0-9]{2})/m_name.json$', key)
                    if m:
                        instrument_number = int(m.group(1))
                        name = json.loads(value)
                        #self._frontend.update_instrument_name(instrument_number, name)

                    if key.endswith('.json'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsone'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonf'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsoni'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonln'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonsh'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonsm'):
                        decoded = json.loads(value)
                    else:
                        decoded = value
                    values[key] = decoded
                #self._frontend.update_import_progress(i + 1, member_count)
            tfile.close()
            self._store.put(values)

    def main(self):
        app = QApplication(sys.argv)
        main_window = MainWindow()
        ui_model = self.create_ui_model()

        update_timer = QTimer()
        QObject.connect(update_timer,
                        SIGNAL('timeout()'),
                        self.update)
        update_timer.start(10)

        if len(sys.argv) > 1:
            module_path = sys.argv[1]
            self.load_module(module_path)

        main_window.set_ui_model(ui_model)
        main_window.show()
        app.exec_()

t = Tracker()
t.main()

