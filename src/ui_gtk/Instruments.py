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


class Instruments(gtk.HBox):

	def ins_info(self, path, args, types):
		iter = self.it_view.get_model().get_iter(args[0] - 1)
		self.ins_table.set_value(iter, 1, args[2])
		self.ins_details[args[0] - 1] = args[1:]
		if self.cur_index == args[0]:
			self.set_details(args[0] - 1)

	def name_changed(self, cell, path, new_text):
		liblo.send(self.engine, '/kunquat/ins_set_name',
				self.song_id,
				int(path) + 1,
				new_text)

	def change_type(self, combobox):
		old_name = None
		if self.ins_details[self.cur_index - 1]:
			old_name = self.ins_details[self.cur_index - 1][1]
		liblo.send(self.engine, '/kunquat/new_ins',
				self.song_id,
				self.cur_index,
				combobox.get_active())
		if old_name:
			liblo.send(self.engine, '/kunquat/ins_set_name',
					self.song_id,
					self.cur_index,
					old_name)

	def select_ins(self, selection):
		_, cur = selection.get_selected_rows()
		if cur == []:
			return
		self.cur_index = cur[0][0] + 1
		self.set_details(cur[0][0])

	def set_details(self, index):
		if not self.ins_details[index]:
			self.types.handler_block(self.htypes)
			self.types.set_active(0)
			self.types.handler_unblock(self.htypes)
			return
		self.types.handler_block(self.htypes)
		self.types.set_active(self.ins_details[index][0])
		self.types.handler_unblock(self.htypes)
		if self.ins_details[index][0] <= 2:
			return

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		self.cur_index = 1
		self.ins_details = [None for _ in range(255)]
		
		self.types = gtk.combo_box_new_text()
		self.types.append_text('None')
		self.types.append_text('Debug')
		self.types.append_text('Sine')
		self.types.append_text('PCM')
		self.types.set_active(0)
		self.htypes = self.types.connect('changed', self.change_type)

		gtk.HBox.__init__(self)

		self.ins_table = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_STRING)
		self.it_view = gtk.TreeView(self.ins_table)
		selection = self.it_view.get_selection()
		selection.connect('changed', self.select_ins)

		for i in range(1, 256):
			iter = self.ins_table.append()
			self.ins_table.set(iter, 0, '%02X' % i)

		selection.select_path(0)

		cell = gtk.CellRendererText()
		column = gtk.TreeViewColumn('#', cell, text = 0)
		self.it_view.append_column(column)
		cell = gtk.CellRendererText()
		cell.set_property('editable', True)
		cell.connect('edited', self.name_changed)
		column = gtk.TreeViewColumn('Name', cell, text = 1)
		self.it_view.append_column(column)

		it_scroll = gtk.ScrolledWindow()
		it_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		it_scroll.add(self.it_view)
		self.it_view.show()

		self.pack_start(it_scroll)
		it_scroll.show()

		ins_details = gtk.VBox()
		ins_details.pack_start(self.types, False, False)
		self.types.show()
		self.pack_start(ins_details)
		ins_details.show()


