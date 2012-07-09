from PyQt4.QtGui import QAbstractScrollArea, QTabWidget, QToolButton, QHBoxLayout, QVBoxLayout, QLabel, QWidget, QSpacerItem, QSizePolicy
#from PyQt4.QtCore import

from kunquat.qt.twbutton import TWButton

class TypewriterModel():
    def __init__(self):
        pass

    def rows(self):
        return [10, 11, 7, 7]

    def data(self, button, role):
        buttons = {
            (0,  0): {'color': 'gray', 'name': u''},
            (0,  1): {'color': 'dark', 'name': u'C#'},
            (0,  2): {'color': 'dark', 'name': u'D#'},
            (0,  3): {'color': 'gray', 'name': u''},
            (0,  4): {'color': 'dark', 'name': u'F#'},
            (0,  5): {'color': 'dark', 'name': u'G#'},
            (0,  6): {'color': 'dark', 'name': u'A#'},
            (0,  7): {'color': 'gray', 'name': u''},
            (0,  8): {'color': 'dark', 'name': u'C#'},
            (0,  9): {'color': 'dark', 'name': u'D#'},
            (0, 10): {'color': 'gray', 'name': u''},

            (1,  0): {'color': 'light', 'name': u'C'},
            (1,  1): {'color': 'light', 'name': u'D'},
            (1,  2): {'color': 'light', 'name': u'E'},
            (1,  3): {'color': 'light', 'name': u'F'},
            (1,  4): {'color': 'light', 'name': u'G'},
            (1,  5): {'color': 'light', 'name': u'A'},
            (1,  6): {'color': 'light', 'name': u'B'},
            (1,  7): {'color': 'light', 'name': u'C'},
            (1,  8): {'color': 'light', 'name': u'D'},
            (1,  9): {'color': 'light', 'name': u'E'},
            (1, 10): {'color': 'light', 'name': u'F'},

            (2,  0): {'color': 'gray', 'name': u''},
            (2,  1): {'color': 'dark', 'name': u'C#'},
            (2,  2): {'color': 'dark', 'name': u'D#'},
            (2,  3): {'color': 'gray', 'name': u''},
            (2,  4): {'color': 'dark', 'name': u'F#'},
            (2,  5): {'color': 'dark', 'name': u'G#'},
            (2,  6): {'color': 'dark', 'name': u'A#'},
            (2,  7): {'color': 'gray', 'name': u''},
            (2,  8): {'color': 'dark', 'name': u'C#'},
            (2,  9): {'color': 'dark', 'name': u'D#'},
            (2, 10): {'color': 'gray', 'name': u''},

            (3,  0): {'color': 'light', 'name': u'C'},
            (3,  1): {'color': 'light', 'name': u'D'},
            (3,  2): {'color': 'light', 'name': u'E'},
            (3,  3): {'color': 'light', 'name': u'F'},
            (3,  4): {'color': 'light', 'name': u'G'},
            (3,  5): {'color': 'light', 'name': u'A'},
            (3,  6): {'color': 'light', 'name': u'B'},
            (3,  7): {'color': 'light', 'name': u'C'},
            (3,  8): {'color': 'light', 'name': u'D'},
            (3,  9): {'color': 'light', 'name': u'E'},
            (3, 10): {'color': 'light', 'name': u'F'},
        }
        
        value = buttons[button][role]
        return value

class TypewriterView(QAbstractScrollArea):

    def __init__(self, parent = None):
        super(self.__class__, self).__init__(parent)
        twmodel = TypewriterModel()
        view = QWidget()
        rows = QVBoxLayout(view)
        rowc = 0
        for buttons in twmodel.rows():
            row = QWidget()
            rowl = QHBoxLayout(row)
            pad = QLabel()
            pad.setMinimumWidth(rowc * 45)
            pad.setMaximumWidth(rowc * 45)
            rowl.addWidget(pad)
            colc = 0
            for i in range(buttons):
                button = TWButton()
                button.set_color(twmodel.data((rowc, colc), 'color'))
                button.set_name(twmodel.data((rowc, colc), 'name'))
                rowl.addWidget(button)
                colc += 1
            rowl.addStretch(1)
            rows.addWidget(row)
            rowc += 1
        rows.addStretch(1)
        self.setViewport(view)

    def setModel(self, twmodel):
        self.model = twmodel
