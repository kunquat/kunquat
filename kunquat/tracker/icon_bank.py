import os

class Icon_bank():
    def __init__(self, prefix):
        share_dir = os.path.join(prefix, 'share')
        icon_dir = os.path.join(share_dir, 'icons')
        self.icon_dir = icon_dir
        self.kunquat_icon = os.path.join(icon_dir, 'kunquat.svg')
        self.events_icon = os.path.join(icon_dir, 'events.png')
        self.music_icon = os.path.join(icon_dir, 'music.png')
        self.instruments_icon = os.path.join(icon_dir, 'instruments.png')
        self.state_icon = os.path.join(icon_dir, 'state.png')

