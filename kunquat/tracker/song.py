import tools

class Song():

    def __init__(self, composition, song_id):
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

    def get_pattern_instance(self, system):
        order_list = self.get_order_list()
        pattern_instance_ref = order_list[system]
        return self._composition.get_pattern_instance(pattern_instance_ref)

    def get_order_list(self):
        orderlist = self._view.get_json('p_order_list.json')
        return orderlist

    def set_order_list(self, order_list):
        self._view.put('p_order_list.json', order_list)

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
            songlist_model = self.p._sheet._subsongs.model
        except:
            return
        songlist_model.update()
        
