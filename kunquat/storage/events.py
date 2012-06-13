
import UserDict

class Store_event(UserDict.DictMixin):
    '''
    >>> d = {'foo': 54, 'bar': 6}
    >>> e = Store_event(**d)
    >>> e.foo
    54
    >>> e['foo']
    54
    >>> e
    {'foo': 54, 'bar': 6}
    >>> dict(e)
    {'foo': 54, 'bar': 6}
    >>> Store_event(keys=4)
    Traceback (most recent call last):
        ...
    ValueError: a Storage_event field can not be named "keys"
    '''
    def __init__(self, **entries):
        for (key, value) in entries.items():
            try:
                getattr(self, key)
                raise ValueError('a Storage_event field can not be named "%s"' % key)
            except AttributeError:
                self.__dict__[key] = value

    def __getitem__(self, x):
        return self.__dict__[x]

    def keys(self):
        return self.__dict__.keys()
 

class Value_update(Store_event):
    pass
       
class Export_start(Store_event):
    pass

class Export_status(Store_event):
    pass

class Export_end(Store_event):
    pass

if __name__ == "__main__":
    import doctest
    doctest.testmod()
