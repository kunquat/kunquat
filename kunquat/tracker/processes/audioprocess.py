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

import sys
import time
import traceback
from multiprocessing import Process, Queue
from queue import Empty

from kunquat.kunquat.kunquat import KunquatError, set_assert_hook
from kunquat.tracker.audio.audio_engine import create_audio_engine


HALT = None


class SimpleCommandQueue():

    def __init__(self):
        self._q = Queue()

    def put(self, command, *args):
        self._q.put((command, args))

    def get(self):
        return self._q.get_nowait()


class KunquatAssertionError(KunquatError):

    def __init__(self, info):
        super().__init__('Assertion failure in libkunquat')
        self.kunquat_desc_override = info


class AudioProcess(Process):

    def __init__(self):
        super().__init__(name='Kunquat Tracker audio')
        self._audio_engine = None
        self._ui_engine = None
        self._q = SimpleCommandQueue()

    def set_ui_engine(self, ui_engine):
        self._ui_engine = ui_engine

    # Audio engine access interface

    def fire_event(self, channel, event):
        self._q.put('fire_event', channel, event)

    def tfire_event(self, channel, event):
        self._q.put('tfire_event', channel, event)

    def set_data(self, transaction_id, transaction):
        self._q.put('set_data', transaction_id, transaction)

    def nanoseconds(self, nanos):
        self._q.put('nanoseconds', nanos)

    def reset_and_pause(self):
        self._q.put('reset_and_pause')

    def sync_call_post_action(self, action_name, args):
        self._q.put('sync_call_post_action', action_name, args)

    # Process interface

    def halt(self):
        self._q.put(HALT)

    def _on_assert(self, info):
        e = KunquatAssertionError(info)
        self._ui_engine.notify_kunquat_assertion(e)

        # We will get destroyed along with our communication channel after returning,
        # so give the UI some time to receive our message
        time.sleep(1)

    def run(self):
        # Create the audio engine inside the correct process
        self._audio_engine = create_audio_engine()
        self._audio_engine.set_ui_engine(self._ui_engine)

        set_assert_hook(self._on_assert)

        try:
            self._run_audio_engine()
        except KeyboardInterrupt:
            pass
        except Exception as e:
            # Send exception information to the UI process with our current traceback
            tb_desc = traceback.format_exc()
            e.kunquat_desc_override = tb_desc
            self._ui_engine.notify_kunquat_exception(e)

        self._audio_engine.close_device()

    def _run_audio_engine(self):
        running = True
        while running:
            while True:
                try:
                    command, args = self._q.get()
                except Empty:
                    break
                if command == HALT:
                    running = False
                else:
                    getattr(self._audio_engine, command)(*args)
            if running:
                self._audio_engine.produce_sound()


def create_audio_process():
    audio_process = AudioProcess()
    return audio_process


