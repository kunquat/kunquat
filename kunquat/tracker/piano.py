
import kunquat.tracker.kqt_limits as lim
from itertools import cycle

class Piano():
    def __init__(self, scale, playback_manager, note_input):
        self._scale = scale
        self._playback_manager = playback_manager
        self._pressed = {}
        self._channel = cycle(xrange(lim.COLUMNS_MAX))
        self._note_input = note_input
        self._inst_num = 0;

    def press(self, note, input_octave):
        octave = self._note_input.base_octave + input_octave
        cents = self._scale.get_cents(note, octave)
        ch = self._channel.next()
        if cents in self._pressed:
            return
        self._pressed[cents] = ch
        self._playback_manager.play_event(ch, ['.i', self._inst_num])
        self._playback_manager.play_event(ch, ['n+', cents])

    def release(self, note, input_octave):
        octave = self._note_input.base_octave + input_octave
        cents = self._scale.get_cents(note, octave)
        if cents not in self._pressed:
            return
        ch = self._pressed.pop(cents)
        self._playback_manager.play_event(ch, ['n-', None])

