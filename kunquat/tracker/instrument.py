
class Instrument():
    FILE_PREFIX = 'kqti00'

    def __init__(self, view):
        self._view = view

    def get_name(self):
        name = self._view.get_json('m_name.json') or '-'
        return name

    def to_tar(self, dest):
        self._view.to_tar(dest, prefix=self.FILE_PREFIX)

    def from_path(self, src):
        self._view.from_path(src, prefix=self.FILE_PREFIX)

    def delete(self):
        self._view.delall()



