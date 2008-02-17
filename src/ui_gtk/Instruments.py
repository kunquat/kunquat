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

	def name_changed(self, cell, path, new_text):
		liblo.send(self.engine, '/kunquat/ins_set_name', self.song_id, int(path) + 1, new_text)

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.HBox.__init__(self)

		self.ins_table = gtk.ListStore(gobject.TYPE_INT, gobject.TYPE_STRING)
		self.it_view = gtk.TreeView(self.ins_table)
#		selection = self.it_view.get_selection()
#		selection.connect('changed', self.set_instrument)

		for i in range(1, 256):
			iter = self.ins_table.append()
			self.ins_table.set(iter, 0, i)

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


