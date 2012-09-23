
class Instrument():
    FILE_PREFIX = 'kqti00'

    def __init__(self, composition, slot):
        self._slot = slot
        path = self.get_id()
        self._view = composition.get_view(path)

    def get_id(self):
        return 'ins_{0:02x}'.format(self._slot)

    def get_name(self):
        name = self._view.get_json('m_name.json') or '-'
        return name

    def get_id_name(self):
        s = u'%s: %s' % (self._slot, self.get_name())
        return s

    def set_name(self, name):
        self._view.put('m_name.json', name)

    def to_tar(self, dest):
        self._view.to_tar(dest, prefix=self.FILE_PREFIX)

    def from_path(self, src):
        self._view.from_path(src, prefix=self.FILE_PREFIX)

    def delete(self):
        self._view.delall()



