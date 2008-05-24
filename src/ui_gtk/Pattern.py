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

from Pat_view import Pat_view


class Pattern(gtk.VBox):

	def event_info(self, path, args, types):
		self.pat_view.event_info(path, args, types)

	def event_entry(self, widget):
		print('ch: %d, pos: %d' % (widget.channel, widget.pos))

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		"""
		self.pat_view = gtk.Table(64, 4, True)
		for i in range(64):
			for j in range(4):
				field = gtk.Entry()
				field.channel = j
				field.pos = i
				field.connect('activate', self.event_entry)
				self.pat_view.attach(field, j, j + 1, i, i + 1)
				field.show()
		"""
		self.pat_view = Pat_view(engine, server, song_id)
		self.pack_end(self.pat_view)
		self.pat_view.show()
		#pat_scroll = gtk.ScrolledWindow()
		#pat_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
		#pat_scroll.add_with_viewport(self.pat_view)
		#self.pat_view.show()

		#self.pack_end(pat_scroll)
		#pat_scroll.show()


