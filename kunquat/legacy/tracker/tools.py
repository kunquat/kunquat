
def list_move(lst, index, target):
    '''
    >>> l = [0, 1, 2]
    >>> list_move(l, 0, 0)
    [0, 1, 2]
    >>> list_move(l, 0, 1)
    [0, 1, 2]
    >>> list_move(l, 0, 2)
    [1, 0, 2]
    >>> list_move(l, 0, 3)
    [1, 2, 0]
    >>> list_move(l, 1, 0)
    [1, 0, 2]
    >>> list_move(l, 1, 1)
    [0, 1, 2]
    >>> list_move(l, 1, 2)
    [0, 1, 2]
    >>> list_move(l, 1, 3)
    [0, 2, 1]
    >>> list_move(l, 2, 0)
    [2, 0, 1]
    >>> list_move(l, 2, 1)
    [0, 2, 1]
    >>> list_move(l, 2, 2)
    [0, 1, 2]
    >>> list_move(l, 2, 3)
    [0, 1, 2]
    '''
    def _list_move(lst, index, target):
        item = lst[index]
        for i, v in enumerate(lst):
            if i == target:
                yield item
            if i != index:
                yield v
        if target == len(lst):
                yield item
    result_generator = _list_move(lst, index, target)
    result = list(result_generator)
    return result

