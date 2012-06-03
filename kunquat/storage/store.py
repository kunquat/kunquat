

class Store(object):

    def __init__(self, path):
        self._memory = {}

    def __getitem__(self, key):
        return self._memory[key]

    def __setitem__(self, key, value):
        self._memory[key] = value

    def commit(self):
        pass

    def flush(self):
        pass


