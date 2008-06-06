# coding=utf-8


# Copyright 2008 Tomi Jylhä-Ollila
#
# This file is part of Kunquat.
#
# Kunquat is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kunquat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.


import pygtk
pygtk.require('2.0')
import gtk
import gobject

import liblo

import Instruments
import Pattern


SONG_TITLE_MAX = 127


class Song(gtk.VBox):

	def song_info(self, path, args, types):
		self.mix_vol = args[1]
		self.init_subsong = args[2]
		self.title.handler_block(self.title_handler)
		self.title.set_text(args[0])
		self.title.handler_unblock(self.title_handler)

	def ins_info(self, path, args, types):
		self.instruments.ins_info(path, args, types)

	def pat_info(self, path, args, types):
		self.pattern.pat_info(path, args, types)

	def event_info(self, path, args, types):
		self.pattern.event_info(path, args, types)

	def events_sent(self, path, args, types):
		self.pattern.events_sent(path, args, types)

	def note_table_info(self, path, args, types):
		# TODO: send to note table editor once supported
		self.pattern.note_table_info(path, args, types)

	def note_info(self, path, args, types):
		self.pattern.note_info(path, args, types)

	def note_mod_info(self, path, args, types):
		self.pattern.note_mod_info(path, args, types)

	def notes_sent(self, path, args, types):
		self.pattern.notes_sent(path, args, types)

	def player_state(self, path, args, types):
		pass

	def set_play(self, button):
		liblo.send(self.engine, '/kunquat/play_song', self.song_id)

	def set_stop(self, button):
		liblo.send(self.engine, '/kunquat/stop_song', self.song_id)

	def title_entry(self, entry):
		text = entry.get_text()
		liblo.send(self.engine, '/kunquat/set_song_title', self.song_id, text)

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		self.mix_vol = 0.0
		self.init_subsong = 0

		play_button = gtk.Button(' Play ')
		play_button.connect('clicked', self.set_play)
		stop_button = gtk.Button(' Stop ')
		stop_button.connect('clicked', self.set_stop)

		info_bar = gtk.HBox()
		label = gtk.Label('Title:')
		self.title = gtk.Entry(SONG_TITLE_MAX)
		self.title_handler = self.title.connect('changed', self.title_entry)
		info_bar.pack_start(play_button, False, False)
		play_button.show()
		info_bar.pack_start(stop_button, False, False)
		stop_button.show()
		info_bar.pack_start(label, False, False)
		label.show()
		info_bar.pack_end(self.title)
		self.title.show()

		self.pack_start(info_bar, False, False)
		info_bar.show()

		nb = gtk.Notebook()

		liblo.send(self.engine, '/kunquat/get_song_info', self.song_id)

		self.instruments = Instruments.Instruments(engine, server, song_id)
		self.pattern = Pattern.Pattern(engine, server, song_id)

		label = gtk.Label('Pattern editor')
		nb.append_page(self.pattern, label)
		self.pattern.show()
		label.show()
		label = gtk.Label('Instruments')
		nb.append_page(self.instruments, label)
		self.instruments.show()
		label.show()

		self.pack_start(nb)
		nb.show()


