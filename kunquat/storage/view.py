
import os
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

    def get(self, key, parse='raw'):
        path = self._path(key)
        try:
            value = self._store._get(path)
        except KeyError:
            return None
        if parse == 'json':
            return json.loads(value)
        return value

    def get_json(self, key):
        return self.get(key, parse='json')

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
        self._store.signal(Export_start(prefix=self.prefix, path=path, key_names=self.keys()))
        tfile = tarfile.open(path, 'w:' + compression, format=tarfile.USTAR_FORMAT)
        for (key, value) in self.items():
            self._store.signal(Export_status(prefix=self.prefix, dest=path, key=key))
            serial = value if isinstance(value, str) else json.dumps(value)
            data = StringIO.StringIO(serial)
            info = tarfile.TarInfo()
            info.name = key
            info.size = len(serial)
            info.mtime = int(time.mktime(time.localtime(time.time())))
            tfile.addfile(info, fileobj=data)
        tfile.close()
        self._store.signal(Export_end(prefix=self.prefix, path=path))

    def from_tar(self, path):
        tfile = tarfile.open(path, format=tarfile.USTAR_FORMAT)
        self._store.signal(Import_start(prefix=self.prefix, path=path, key_names=tfile.getnames()))
        for entry in tfile.getmembers():
            key = entry.name
            self._store.signal(Import_status(prefix=self.prefix, dest=path, key=key))
            if entry.isfile():
                value = tfile.extractfile(entry).read()
                self.put(key, value)
        tfile.close()
        self._store.signal(Import_end(prefix=self.prefix, path=path))

    def from_dir(self, path):
        raise Exception('from_dir not implemented!')
        '''
        self._history.start_group('Import composition {0}'.format(path))
        tfile = None
        QtCore.QObject.emit(self, QtCore.SIGNAL('startTask(int)'), 0)
            self._clear()
        try:
            if not path or path[-1] != '/':
                path = path + '/'
            for dir_spec in os.walk(path):
                for fname in dir_spec[2]:
                    full_path = os.path.join(dir_spec[0], fname)
                    key = full_path[len(path):]
                    with open(full_path, 'rb') as f:
                        QtCore.QObject.emit(self,
                                QtCore.SIGNAL('step(QString)'),
                                'Importing {0} ...'.format(full_path))
                        if key[key.index('.'):].startswith('.json'):
                            self.set(key, json.loads(f.read()),
                                     autoconnect=False)
                        else:
                            self.set(key, f.read(), autoconnect=False)
        finally:
            if tfile:
                tfile.close()
            self._history.end_group()
            QtCore.QObject.emit(self, QtCore.SIGNAL('endTask()'))
        '''

    def from_path(self, path):
        if os.path.isdir(path):
            self.from_dir(path)
        else:
            self.from_tar(path)
                


