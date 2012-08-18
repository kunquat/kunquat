
import kunquat.tracker.kqt_limits as lim
from itertools import cycle

class Piano():
    def __init__(self, p):
        self.p = p 
        self._pressed = {}
        self._channel = cycle(xrange(lim.COLUMNS_MAX))
        self._inst_num = 0;
        
    def play(self, cents):
        ch = self._channel.next()
        self._pressed[cents] = ch
        self.p._playback.play_event(ch, ['.i', self._inst_num])
        self.p._playback.play_event(ch, ['n+', cents])

    def press(self, note, input_octave, cursor = None):
        octave = self.p._note_input.base_octave + input_octave
        cents = self.p._scale.get_cents(note, octave)
        if cents in self._pressed:
            return
        if cursor == None:
            play_note = True
        else:
            play_note = cursor.note_on_cents(cents)
        if play_note:
            self.play(cents)

    def release(self, note, input_octave):
        octave = self.p._note_input.base_octave + input_octave
        cents = self.p._scale.get_cents(note, octave)
        if cents not in self._pressed:
            return
        ch = self._pressed.pop(cents)
        self.p._playback.play_event(ch, ['n-', None])

