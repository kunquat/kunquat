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


class Song_opts(gtk.VBox):

	def song_info(self, path, args, types):
		self.mix_vol.get_adjustment().handler_block(self.hmix_vol)
		self.mix_vol.set_value(args[1])
		self.mix_vol.get_adjustment().handler_unblock(self.hmix_vol)

	def format_dbfs(self, scale, value):
		return '%.2f dBFS' % value

	def set_mix_vol(self, adj):
		assert adj.get_value() <= 0, 'Attempted to send an invalid volume.'
		liblo.send(self.engine, '/kunquat/set_mix_vol',
				self.song_id,
				('d', adj.get_value()))

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		gtk.VBox.__init__(self)

		label = gtk.Label('Mixing volume')
		adj = gtk.Adjustment(-8, -96, 0, 0.01, 1)
		self.hmix_vol = adj.connect('value-changed', self.set_mix_vol)
		self.mix_vol = gtk.HScale(adj)
		self.mix_vol.set_digits(2)
		self.mix_vol.connect('format-value', self.format_dbfs)
		box = gtk.HBox()
		box.pack_start(label, False, False)
		box.pack_start(self.mix_vol)

		self.pack_start(box, False, False)
		box.show_all()


