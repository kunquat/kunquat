import tools

class Song():

    def __init__(self, composition, p, song_id):
        self.p = p
        self._song_id = song_id
        path = song_id
        self._composition = composition
        self._view = composition.get_view(path)

    def get_ref(self):
        ref = int(self._song_id.split('_')[1])
        return ref

    def get_name(self):
        name = self._view.get_json('m_name.json') or '-'
        return name

    def set_name(self, name):
        self._view.put('m_name.json', unicode(name))

    def get_pattern_instance(self, system):
        order_list = self.get_order_list()
        pattern_instance_ref = order_list[system]
        return self._composition.get_pattern_instance(pattern_instance_ref)

    def get_order_list(self):
        orderlist = self._view.get_json('p_order_list.json')
        return orderlist

    def set_order_list(self, order_list):
        self._view.put('p_order_list.json', order_list)

    def delete_system(self, system_number):
        ol = self.get_order_list()
        assert(len(ol) > 1) # last item should never be deleted
        pattern_instance = self.get_pattern_instance(system_number)
        pattern_instance.delete()
        del ol[system_number]
        self.set_order_list(ol)

    def delete(self):
        del self._view['p_manifest.json']

    def _add_pattern(self, pattern_instance, target):
        ol = (self.get_order_list() or []) + [pattern_instance]
        system_number = len(ol) - 1
        systems = tools.list_move(ol, system_number, target)
        self.set_order_list(systems)

    def new_pattern(self, target):
        pattern_instance = self._composition.new_pattern()
        self._add_pattern(pattern_instance, target)

    def reuse(self, system):
        old_pattern_instance = self.get_pattern_instance(system)
        new_pattern_instance = old_pattern_instance.duplicate()
        target = system + 1
        self._add_pattern(new_pattern_instance, target)

    def move_system(self, system_number, target):
        ol = self.get_order_list()
        systems = tools.list_move(ol, system_number, target)
        self.set_order_list(systems)

    def system_count(self):
        order_list = self.get_order_list()
        count = len(order_list)
        return count

    def update_order_list(self, order_list_json):
        import json
        self._order_list = json.loads(order_list_json)
        try:
            songlist = self.p._sheet._subsongs
        except:
            return
        songlist.update_model()
        
