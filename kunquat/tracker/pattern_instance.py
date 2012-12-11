
class Pattern_instance():

    def __init__(self, composition, pattern_instance_ref):
        self._composition = composition
        pattern, instance = pattern_instance_ref
        self.pattern = pattern
        self.instance = instance

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def get_ref(self):
        return (self.pattern, self.instance)

    def get_name(self, song = None):
        ambiguous_name = u'pattern {0}'.format(self.pattern)
        fullname = ambiguous_name + self.subscript(self.instance)
        if song == None:
            return fullname
        order_list = song.get_order_list()
        if len([p for (p, _) in order_list if p == self.pattern]) > 1:
            return fullname
        return ambiguous_name

    
