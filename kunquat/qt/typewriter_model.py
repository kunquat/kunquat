from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
from PyQt4.QtCore import Qt, QObject
from PyQt4 import QtCore

from kunquat.qt.twbutton import TWButton

from random import choice

class TypewriterModel():
    def __init__(self, p):
        self.p = p
        self._buttons = self.p._scale.buttons
        self._random_led_color = 0
        self._views = []

        die1_code = 0x2680
        dice_code = range(die1_code, die1_code + 6)
        self._dice = [unichr(i) for i in dice_code]
        self._die = self._dice[4]

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

    def roll_die(self):
        self._die = choice(self._dice)
        self.update_views()

    def get_die(self):
        return self._die

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
