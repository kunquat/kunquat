# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
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
import os.path

import kunquat.tracker.cmdline as cmdline
from store import Store
from session import Session
from share import Share
from updater import Updater
from notechannelmapper import NoteChannelMapper

#TODO: figure a place for the events
EVENT_SELECT_CONTROL = '.i'
EVENT_NOTE_ON = 'n+'
EVENT_NOTE_OFF = 'n-'


class Controller():

    def __init__(self):
        self._push_amount = None
        self._audio_levels = (0, 0)
        self._store = None
        self._session = None
        self._share = None
        self._updater = None
        self._note_channel_mapper = None
        self._audio_engine = None

    def set_store(self, store):
        self._store = store

    def get_store(self):
        return self._store

    def set_session(self, session):
        self._session = session

    def get_session(self):
        return self._session

    def set_share(self, share):
        self._share = share

    def get_share(self):
        return self._share

    def set_updater(self, updater):
        self._updater = updater

    def get_updater(self):
        return self._updater

    def set_note_channel_mapper(self, note_channel_mapper):
        self._note_channel_mapper = note_channel_mapper
        self._note_channel_mapper.set_controller(self)

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine
        self._store.set_audio_engine(audio_engine)

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
            self.update_import_progress(0, member_count)
            for i, entry in zip(range(member_count), members):
                yield
                tarpath = entry.name
                key = self._remove_prefix(tarpath, prefix)
                assert (key != None) #TODO broken file exception
                if entry.isfile():
                    value = tfile.extractfile(entry).read()
                    if key.endswith('.json'):
                        decoded = json.loads(value)
                    else:
                        decoded = value
                    values[key] = decoded
                self.update_import_progress(i + 1, member_count)
            tfile.close()
            self._store.put(values)
            self._updater.signal_update(set(['signal_controls', 'signal_module']))

    def get_task_load_instrument(self, kqtifile):
        for _ in kqtifile.get_read_steps():
            yield
        contents = kqtifile.get_contents()

        # TODO: Validate contents

        ins_number = 0
        ins_prefix = 'ins_{:02x}'.format(ins_number)
        transaction = {}

        # TODO: Figure out a proper way of connecting the instrument
        connections = [['/'.join((ins_prefix, 'out_00')), 'out_00']]
        transaction['p_connections.json'] = connections

        control_map = [[0, ins_number]]
        transaction['p_control_map.json'] = control_map
        transaction['control_00/p_manifest.json'] = {}

        # Add instrument data to the transaction
        for (key, value) in contents.iteritems():
            dest_key = '/'.join((ins_prefix, key))
            transaction[dest_key] = value

        # Send data
        self._store.put(transaction)
        self._updater.signal_update(set(['signal_controls', 'signal_module']))

    def play(self):
        self._audio_engine.nanoseconds(0)

    def start_tracked_note(self, channel_number, control_id, pitch):
        note = self._note_channel_mapper.get_tracked_note(channel_number, False)
        self.set_active_note(note.get_channel(), control_id, pitch)
        return note

    def set_active_note(self, channel_number, control_id, pitch):
        parts = control_id.split('_')
        second = parts[1]
        control_number = int(second, 16)
        control_event = (EVENT_SELECT_CONTROL, control_number)
        self._audio_engine.fire_event(channel_number, control_event)
        note_on_event = (EVENT_NOTE_ON, pitch)
        self._audio_engine.fire_event(channel_number, note_on_event)

    def set_rest(self, channel_number):
        note_off_event = (EVENT_NOTE_OFF, None)
        self._audio_engine.fire_event(channel_number, note_off_event)

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

    def update_selected_control(self, channel, control_id):
        self._session.set_selected_control_id(channel, control_id)
        self._updater.signal_update()

    def update_active_note(self, channel, pitch):
        self._session.set_active_note(channel, pitch)
        self._updater.signal_update()

    def update_event_log_with(self, channel_number, event_type, event_value, context):
        self._session.log_event(channel_number, event_type, event_value, context)
        self._updater.signal_update()

    def update_import_progress(self, position, steps):
        self._session.set_progress_position(position)
        self._session.set_progress_steps(steps)
        self._updater.signal_update()


def create_controller():
    store = Store()
    session = Session()
    share_path = os.path.join(cmdline.get_install_prefix(), 'share', 'kunquat')
    share = Share(share_path)
    updater = Updater()
    note_channel_mapper = NoteChannelMapper()
    controller = Controller()
    controller.set_store(store)
    controller.set_session(session)
    controller.set_share(share)
    controller.set_updater(updater)
    controller.set_note_channel_mapper(note_channel_mapper)
    return controller


