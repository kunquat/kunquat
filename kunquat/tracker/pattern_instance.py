
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

    def sibling_ids(self):
        folders = [f.split('/')[1] for f in self._composition._store.keys() if len(f.split('/')) > 2 and f.split('/')[0] == 'pat_%03d' % self.pattern and f.split('/')[2] == 'p_manifest.json']
        foo =  set([f for f in folders if f.startswith('instance')])
        return foo

    def duplicate(self):
        existing = self.sibling_ids()
        instance_ids = ('instance_%03d' % i for i in xrange(1000))
        for instance_id in instance_ids:
            if instance_id not in existing:
                instance = int(instance_id.split('_')[1])
                pattern_instance = (self.pattern, instance)
                self._composition._init_instance(pattern_instance)
                return pattern_instance
        raise Exception

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
