
class Song():

    def __init__(self, composition, song_id):
        self._song_id = song_id
        path = song_id
        self._composition = composition
        self._view = composition.get_view(path)

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

    def system_count(self):
        order_list = self.get_order_list()
        count = len(order_list)
        return count
