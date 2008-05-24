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


class Song(gtk.VBox):

	def ins_info(self, path, args, types):
		self.instruments.ins_info(path, args, types)

	def pat_info(self, path, args, types):
		self.pattern.pat_info(path, args, types)

	def event_info(self, path, args, types):
		self.pattern.event_info(path, args, types)

	def events_sent(self, path, args, types):
		self.pattern.events_sent(path, args, types)

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		self.instruments = Instruments.Instruments(engine, server, song_id)
		self.pattern = Pattern.Pattern(engine, server, song_id)

		self.pack_start(self.instruments)
		self.instruments.show()

		self.pack_end(self.pattern)
		self.pattern.show()


