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


gtk.rc_parse_string("""
	style "tab-close-button-style" {
		GtkWidget::focus-padding = 0
		GtkWidget::focus-line-width = 0
		xthickness = 0
		ythickness = 0
	}
	widget "*.tab-close-button" style "tab-close-button-style"
	""")


class Songs(gtk.Notebook):

	def set_close_size(self, button, prev_style):
		x, y = gtk.icon_size_lookup_for_settings(button.get_settings(),
				gtk.ICON_SIZE_MENU)
		button.set_size_request(x + 2, y + 2)

	def close_song(self, button, id):
		liblo.send(self.engine, '/kunquat/del_song', id)

	def create_tab_head(self, title, id):
		if not title:
			title = '(untitled #%d)' % id
		label = gtk.Label('%d: %s' % (id, title))
		image = gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU)
		close = gtk.Button()
		close.set_relief(gtk.RELIEF_NONE)
		close.set_focus_on_click(False)
		close.add(image)
		close.set_name('tab-close-button')
		close.connect('style-set', self.set_close_size)
		close.connect('clicked', self.close_song, id)
		head = gtk.HBox(spacing=5)
		head.pack_start(label)
		head.pack_start(close, False, False)
		return head

	def new_song(self, path, args, types):
		content = Song.Song(self.engine, self.server, args[0])
		head = self.create_tab_head('', args[0])
		self.append_page(content, head)
		content.request_info()
		content.show()
		head.show_all()

	def songs(self, path, args, types):
		all_songs = set(args)
		shown_songs = set([self.get_nth_page(i).song_id for i in range(self.get_n_pages())])
		for id in all_songs - shown_songs:
			content = Song.Song(self.engine, self.server, id)
			head = self.create_tab_head('', id)
			self.append_page(content, head)
			content.request_info()
			content.show()
			head.show_all()

	def find_song(self, id):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == id:
				return content
		return None

	def song_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.song_info(path, args[1:], types[1:])
				label = self.get_tab_label(content).get_children()[0]
				if label:
					if not args[1]:
						args[1] = '(untitled #%d)' % content.song_id
					label.set_label(str(content.song_id) + ': ' + args[1])
				return
		new_song(path, [args[0]], ['i'])
		song_info(path, args, types)

	def subsong_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.subsong_info(path, args[1:], types[1:])
				return

	def del_song(self, path, args, types):
		if len(args) == 0:
			return
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				self.remove_page(i)
				break

	def order_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.order_info(path, args[1:], types[1:])
				return

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

	def pat_meta(self, path, args, types):
		self.pat_info(path, args, types)

	def event_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.event_info(path, args[1:], types[1:])
				return
#		new_song(path, [args[0]], ['i'])
#		event_info(path, args, types)

	def events_sent(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.events_sent(path, args[1:], types[1:])
				return
		return

	def note_table_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.note_table_info(path, args[1:], types[1:])
				return

	def note_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.note_info(path, args[1:], types[1:])
				return

	def note_mod_info(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.note_mod_info(path, args[1:], types[1:])
				return

	def notes_sent(self, path, args, types):
		for i in range(self.get_n_pages()):
			content = self.get_nth_page(i)
			if content.song_id == args[0]:
				content.notes_sent(path, args[1:], types[1:])
				return

	def player_state(self, path, args, types):
		content = self.find_song(args[0])
		content.player_state(path, args[1:], types[1:])
		return False

	def handle_osc(self, path, args, types):
		ps = path.split('/')
		assert ps[1] == 'kunquat_gtk', 'Incorrect OSC path'
		gobject.idle_add(Songs.__dict__[ps[2]], self, path, args, types)

	def __init__(self, engine, server):
		self.engine = engine
		self.server = server

		self.server.add_method('/kunquat_gtk/new_song', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/songs', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/song_info', 'isdi', self.handle_osc)
		self.server.add_method('/kunquat_gtk/subsong_info', 'iidd', self.handle_osc)
		self.server.add_method('/kunquat_gtk/del_song', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/order_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/ins_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/ins_type_desc', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/ins_type_field', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/pat_info', 'iihi', self.handle_osc)
		self.server.add_method('/kunquat_gtk/pat_meta', 'iihi', self.handle_osc)
		self.server.add_method('/kunquat_gtk/event_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/events_sent', 'ii', self.handle_osc)
		self.server.add_method('/kunquat_gtk/note_table_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/note_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/note_mod_info', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/notes_sent', 'ii', self.handle_osc)
		self.server.add_method('/kunquat_gtk/player_state', 'is', self.handle_osc)

		gtk.Notebook.__init__(self)


