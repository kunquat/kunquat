
class Song():

    def __init__(self, composition, song_id):
        self._song_id = song_id
        path = song_id
        self._view = composition.get_view(path)

    def get_name(self):
        name = self._view.get_json('m_name.json') or '-'
        return name

    def get_order_list(self):
        songdata = self._view.get_json('p_song.json')
        order_list = songdata['patterns']
        return order_list
