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


class Driver_select(gtk.HBox):

	def set_drivers(self, path, args):
		self.driver_list.clear()
		for arg in ['No sound'] + args:
			iter = self.driver_list.append()
			self.driver_list.set(iter, 0, arg)

	def __init__(self, engine):
		self.engine = engine

		gtk.HBox.__init__(self)

		self.driver_list = gtk.ListStore(gobject.TYPE_STRING)
		self.driver_view = gtk.TreeView(self.driver_list)
		
		iter = self.driver_list.append()
		self.driver_list.set(iter, 0, 'No sound')

		cell = gtk.CellRendererText()
		column = gtk.TreeViewColumn('Drivers', cell, text = 0)
		self.driver_view.append_column(column)

		driver_scroll = gtk.ScrolledWindow()
		driver_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		driver_scroll.add(self.driver_view)
		self.driver_view.show()

		self.pack_start(driver_scroll)
		driver_scroll.show()


