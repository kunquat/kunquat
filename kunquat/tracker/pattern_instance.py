
class Pattern_instance():

    def __init__(self, composition, pattern_instance_ref):
        self._composition = composition
        pattern, instance = pattern_instance_ref
        self.pattern = pattern
        self.instance = instance
        path = 'pat_%03d/instance_%03d' % (pattern, instance)
        self._view = composition.get_view(path)

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def get_ref(self):
        return (self.pattern, self.instance)

    def get_name(self):
        ambiguous_name = u'pattern {0}'.format(self.pattern)
        fullname = ambiguous_name + self.subscript(self.instance)
        pattern_instances = self._composition.get_pattern_instances()
        if len([p for (p, _) in pattern_instances if p == self.pattern]) > 1:
            return fullname
        return ambiguous_name

    def delete(self):
        del self._view['p_manifest.json']
