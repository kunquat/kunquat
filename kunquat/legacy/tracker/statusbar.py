from PyQt4 import QtCore, QtGui

class Statusbar(QtGui.QWidget):

    def __init__(self, p, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self.p = p
        self._busy = p.busy
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        self._status_bar = QtGui.QStatusBar()
        self._status_bar.showMessage('')

        self._progress_bar = QtGui.QProgressBar()
        #self._progress_bar.setAlignment(QtCore.Qt.AlignLeft)
        self._progress_bar.hide()

        layout.addWidget(self._status_bar, 1)
        layout.addWidget(self._progress_bar)

        self._step = 0
        self.setSizePolicy(QtGui.QSizePolicy.MinimumExpanding,
                           QtGui.QSizePolicy.Fixed)
        self._busy_timer = QtCore.QTimer(self)
        self._busy_timer.setSingleShot(True)
        QtCore.QObject.connect(self._busy_timer, QtCore.SIGNAL('timeout()'),
                               lambda: self._busy(True))
        self._tasks = 0

    def get_view(self):
        return self

    def in_progress(self):
        assert self._tasks >= 0
        return self._tasks > 0

    def sizeHint(self):
        h = max(self._status_bar.sizeHint().height(),
                self._progress_bar.sizeHint().height())
        return QtCore.QSize(200, h)

    def start_task(self, steps):
        self._tasks += 1
        if self._tasks > 1:
            self._progress_bar.setMaximum(self._progress_bar.maximum() +
                                          steps)
            return
        self._busy_timer.start(100)
        self._progress_bar.setRange(0, steps)
        self._progress_bar.reset()
        if steps > 1:
            self._progress_bar.show()

    def step(self, description):
        self._status_bar.showMessage(description)
        if self._step < self._progress_bar.maximum():
            self._progress_bar.setValue(self._step)
            self._step += 1
        #self._status_bar.update()

    def end_task(self):
        if self._tasks > 1:
            self._tasks -= 1
            return
        self._busy_timer.stop()
        self._busy_timer.setSingleShot(True)
        self._busy(False)
        self._status_bar.showMessage('')
        self._progress_bar.hide()
        self._step = 0
        self._tasks -= 1
        assert self._tasks >= 0
