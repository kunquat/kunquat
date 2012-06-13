
import time
import json
import tarfile
import StringIO
from events import *


class View():

    def __init__(self, store, prefix=''):
        self._store = store
        self.prefix = prefix

    def __getitem__(self, key):
        return self.get(key)

    def __setitem__(self, key, value):
        self.put(key, value)

    def _path(self, key):
        path = '%s/%s' % (self.prefix, key)
        return path

    def put(self, key, value):
        path = self._path(key)
        self._store.put(path, value)

    def get(self, key):
        path = self._path(key)
        return self._store.get(path)

    def keys(self):
        return [key for (key, _) in self.items()]

    def items(self):
        path = '%s' % self.prefix
        memory = self._store._memory.items()
        valid = [(key[len(path):], value) for (key, value) in memory if key.startswith(path)]
        return valid

    def to_tar(self, path):
        compression = ''
        if path.endswith('.gz'):
            compression = 'gz'
        elif path.endswith('.bz2'):
            compression = 'bz2'
        self._store.signal(Export_start(keycount=len(self.keys())))
        tfile = tarfile.open(path, 'w:' + compression, format=tarfile.USTAR_FORMAT)
        for (key, value) in self.items():
            self._store.signal(Export_status(dest=path, key=key))
            serial = value if isinstance(value, str) else json.dumps(value)
            data = StringIO.StringIO(serial)
            info = tarfile.TarInfo()
            info.name = key
            info.size = len(serial)
            info.mtime = int(time.mktime(time.localtime(time.time())))
            tfile.addfile(info, fileobj=data)
        tfile.close()
        self._store.signal(Export_end())

