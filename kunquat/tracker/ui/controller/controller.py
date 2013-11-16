# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import re
import sys
import json
import time
import tarfile

from kunquat.tracker.ui.controller.store import Store
from kunquat.tracker.ui.controller.session import Session
from kunquat.tracker.ui.controller.updater import Updater

#TODO: figure a place for the events
EVENT_SELECT_INSTRUMENT = '.i'
EVENT_NOTE_ON = 'n+'
EVENT_NOTE_OFF = 'n-'


class Controller():

    def __init__(self):
        self._frontend = None
        self._push_amount = None
        self._audio_levels = (0, 0)
        self._store = None
        self._session = None
        self._updater = None
        self._audio_engine = None

    def set_store(self, store):
        self._store = store

    def get_store(self):
        return self._store

    def set_session(self, session):
        self._session = session

    def get_session(self):
        return self._session

    def set_updater(self, updater):
        self._updater = updater

    def get_updater(self):
        return self._updater

    def set_frontend(self, frontend):
        self._frontend = frontend

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def _remove_prefix(self, path, prefix):
        preparts = prefix.split('/')
        keyparts = path.split('/')
        for pp in preparts:
            kp = keyparts.pop(0)
            if pp != kp:
                 return None
        return '/'.join(keyparts)

    def get_task_load_module(self, module_path):
        values = dict()
        if module_path[-4:] in ['.kqt', '.bz2']:
            prefix = 'kqtc00'
            tfile = tarfile.open(module_path, format=tarfile.USTAR_FORMAT)
            members = tfile.getmembers()
            member_count = len(members)
            #assert self._frontend
            #self._frontend.update_import_progress(0, member_count)
            for i, entry in zip(range(member_count), members):
                yield
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
            self._updater.signal_update()
            self._audio_engine.set_data(values)

    def play(self):
        self._audio_engine.nanoseconds(0)

    def set_active_note(self, channel_number, instrument_id, pitch):
        parts = instrument_id.split('_')
        second = parts[1]
        instrument_number = int(second)
        instrument_event = (EVENT_SELECT_INSTRUMENT, instrument_number)
        self._audio_engine.fire_event(channel_number, instrument_event)
        note_on_event = (EVENT_NOTE_ON, pitch)
        self._audio_engine.fire_event(channel_number, note_on_event)

    def update_output_speed(self, fps):
        self._session.set_output_speed(fps)
        self._updater.signal_update()

    def update_render_speed(self, fps):
        self._session.set_render_speed(fps)
        self._updater.signal_update()

    def update_render_load(self, load):
        self._session.set_render_load(load)
        self._updater.signal_update()

    def update_audio_levels(self, levels):
        self._session.set_audio_levels(levels)
        self._updater.signal_update()

    def update_ui_lag(self, lag):
        self._session.set_ui_lag(lag)
        self._updater.signal_update()

    def update_selected_instrument(self, channel, instrument):
        self._session.set_selected_instrument(channel, instrument)
        self._updater.signal_update()

    def update_active_note(self, channel, pitch):
        self._session.set_active_note(channel, pitch)
        self._updater.signal_update()


def create_controller():
    store = Store()
    session = Session()
    updater = Updater()
    controller = Controller()
    controller.set_store(store)
    controller.set_session(session)
    controller.set_updater(updater)
    return controller

