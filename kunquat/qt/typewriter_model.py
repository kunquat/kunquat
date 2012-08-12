from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

class TypewriterModel():
    def __init__(self):
        self._buttons = {
            (0,  0): {'color': 'dark', 'name': u'C#', 'enabled': True },
            (0,  1): {'color': 'dark', 'name': u'D#', 'enabled': True },
            (0,  2): {'color': 'gray', 'name': u''  , 'enabled': False},
            (0,  3): {'color': 'dark', 'name': u'F#', 'enabled': True },
            (0,  4): {'color': 'dark', 'name': u'G#', 'enabled': True },
            (0,  5): {'color': 'dark', 'name': u'A#', 'enabled': True },
            (0,  6): {'color': 'gray', 'name': u''  , 'enabled': False},
            (0,  7): {'color': 'dark', 'name': u'C#', 'enabled': True },
            (0,  8): {'color': 'dark', 'name': u'D#', 'enabled': True },
            (0,  9): {'color': 'gray', 'name': u''  , 'enabled': False},
            (0, 10): {'color': 'gray', 'name': u'#F', 'enabled': True },

            (1,  0): {'color': 'light', 'name': u'C', 'enabled': True },
            (1,  1): {'color': 'light', 'name': u'D', 'enabled': True },
            (1,  2): {'color': 'light', 'name': u'E', 'enabled': True },
            (1,  3): {'color': 'light', 'name': u'F', 'enabled': True },
            (1,  4): {'color': 'light', 'name': u'G', 'enabled': True },
            (1,  5): {'color': 'light', 'name': u'A', 'enabled': True },
            (1,  6): {'color': 'light', 'name': u'B', 'enabled': True },
            (1,  7): {'color': 'light', 'name': u'C', 'enabled': True },
            (1,  8): {'color': 'light', 'name': u'D', 'enabled': True },
            (1,  9): {'color': 'light', 'name': u'E', 'enabled': True },
            (1, 10): {'color': 'light', 'name': u'F', 'enabled': True },

            (2,  0): {'color': 'gray', 'name': u''  , 'enabled': False},
            (2,  1): {'color': 'dark', 'name': u'C#', 'enabled': True },
            (2,  2): {'color': 'dark', 'name': u'D#', 'enabled': True },
            (2,  3): {'color': 'gray', 'name': u''  , 'enabled': False},
            (2,  4): {'color': 'dark', 'name': u'F#', 'enabled': True },
            (2,  5): {'color': 'dark', 'name': u'G#', 'enabled': True },
            (2,  6): {'color': 'dark', 'name': u'A#', 'enabled': True },
            (2,  7): {'color': 'gray', 'name': u''  , 'enabled': False},
            (2,  8): {'color': 'dark', 'name': u'C#', 'enabled': True },
            (2,  9): {'color': 'dark', 'name': u'D#', 'enabled': True },
            (2, 10): {'color': 'gray', 'name': u''  , 'enabled': False},

            (3,  0): {'color': 'light', 'name': u'C', 'enabled': True },
            (3,  1): {'color': 'light', 'name': u'D', 'enabled': True },
            (3,  2): {'color': 'light', 'name': u'E', 'enabled': True },
            (3,  3): {'color': 'light', 'name': u'F', 'enabled': True },
            (3,  4): {'color': 'light', 'name': u'G', 'enabled': True },
            (3,  5): {'color': 'light', 'name': u'A', 'enabled': True },
            (3,  6): {'color': 'light', 'name': u'B', 'enabled': True },
            (3,  7): {'color': 'light', 'name': u'C', 'enabled': True },
            (3,  8): {'color': 'light', 'name': u'D', 'enabled': True },
            (3,  9): {'color': 'light', 'name': u'E', 'enabled': True },
            (3, 10): {'color': 'light', 'name': u'F', 'enabled': True },
        }
        self._random_led_color = 0
        self._views = []

    def get_led_color(self, coord):
        DEFAULT = 0
        row, but = coord
        try:
            color = self._buttons[(row,but)]['led']
        except KeyError:
            return DEFAULT
        return color

    def set_led_color(self, coord, color):
        row, but = coord
        self._buttons[(row, but)]['led'] = color
        self.update_views()

    def get_random_led_color(self):
        return self._random_led_color

    def set_random_led_color(self, color):
        self._random_led_color = color
        self.update_views()

    def data(self, button, role):
        value = self._buttons[button][role]
        return value

    def register_view(self, view):
        view.setModel(self)
        self._views.append(view)

    def update_views(self):
        for view in self._views:
            view.update()
