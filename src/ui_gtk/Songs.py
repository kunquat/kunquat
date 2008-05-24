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

import Song


class Songs(gtk.Notebook):

	def new_song(self, path, args, types):
		if types[0] == 's':
			return
		content = Song.Song(self.engine, self.server, args[0])
		label = gtk.Label(str(args[0]))
		self.append_page(content, label)
		content.show()
		label.show()

	def songs(self, path, args):
		all_songs = set(args)
		shown_songs = set([self.get_nth_page(i).song_id for i in range(self.get_n_pages())])
		for id in all_songs - shown_songs:
			content = Song.Song(self.engine, self.server, id)
			label = gtk.Label(str(id))
			self.append_page(content, label)
			content.show()
			label.show()

	def del_song(self, path, args, types):
		if len(args) == 0:
			return
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				self.remove_page(i)
				break

	def ins_info(self, path, args, types):
		if len(args) == 0:
			return
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.ins_info(path, args[1:], types[1:])
				return
		new_song(path, [args[0]], ['i'])
		ins_info(path, args, types)

	def pat_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.pat_info(path, args[1:], types[1:])
				return
		new_song(path, [args[0]], ['i'])
		pat_info(path, args, types)

	def event_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.event_info(path, args[1:], types[1:])
				return
		new_song(path, [args[0]], ['i'])
		event_info(path, args, types)

	def events_sent(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.events_sent(path, args[1:], types[1:])
				return
		return

	def __init__(self, engine, server):
		self.engine = engine
		self.server = server

		self.server.add_method('/kunquat_gtk/new_song', None, self.new_song)
		self.server.add_method('/kunquat_gtk/songs', None, self.songs)
		self.server.add_method('/kunquat_gtk/del_song', None, self.del_song)
		self.server.add_method('/kunquat_gtk/ins_info', None, self.ins_info)
		self.server.add_method('/kunquat_gtk/pat_info', 'iihi', self.pat_info)
		self.server.add_method('/kunquat_gtk/event_info', None, self.event_info)
		self.server.add_method('/kunquat_gtk/events_sent', 'ii', self.events_sent)

		gtk.Notebook.__init__(self)


