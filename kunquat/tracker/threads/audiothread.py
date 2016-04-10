# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from kunquat.tracker.audio.audio_engine import create_audio_engine

from .command import Command
from .commandqueue import CommandQueue
from .eventpump import EventPump
from .monitoringthread import MonitoringThread

HALT = None

class AudioThread(MonitoringThread):

    def __init__(self):
        MonitoringThread.__init__(self, name="Audio")
        self._ui_engine = None
        self._engine = None
        self._q = CommandQueue()
        self._unprocessed_commands = []
        self._pump = EventPump()
        self._pump.set_blocker(self._q.block)
        self._pump.set_signaler(self._signal)

    def set_handler(self, engine):
        self._engine = engine

    def set_ui_engine(self, ui_engine):
        self._ui_engine = ui_engine
        self._engine.set_ui_engine(ui_engine)

    def _close_device(self):
        if self._engine:
            self._engine.close_device()
            time.sleep(0.1)

    def _signal(self):
        next_command = self._q.get()
        self._unprocessed_commands.append(next_command)

    def _count_commands(self):
        return len(self._unprocessed_commands)

    def _next_command(self):
        if len(self._unprocessed_commands) < 1:
            return None
        next_command = self._unprocessed_commands.pop(0)
        return next_command

    # Libkunquat interface

    def fire_event(self, channel, event):
        self._q.push('fire_event', channel, event)

    def tfire_event(self, channel, event):
        self._q.push('tfire_event', channel, event)

    def set_data(self, transaction_id, transaction):
        self._q.push('set_data', transaction_id, transaction)

    def nanoseconds(self, nanos):
        self._q.push('nanoseconds', nanos)

    def reset_and_pause(self):
        self._q.push('reset_and_pause')

    def sync_call_post_action(self, action_name, args):
        self._q.push('sync_call_post_action', action_name, args)

    # Thread interface

    def halt(self):
        self._q.push(HALT)

    def run_monitored(self):
        self._pump.start()
        running = True
        while running:
            #command == None or command.name != HALT:
            count = self._count_commands()
            for _ in range(count):
                command = self._next_command()
                if command.name == HALT:
                    running = False
                if running:
                    getattr(self._engine, command.name)(*command.args)
            if running:
                self._engine.produce_sound()
        self._pump.set_signaler(None)
        self._close_device()

def create_audio_thread():
    audio_engine = create_audio_engine()
    audio_thread = AudioThread()
    audio_thread.set_handler(audio_engine)
    return audio_thread

