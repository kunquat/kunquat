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


class Cli(gtk.VBox):

	def update_history(self, text):
		if not self.history:
			return
		if text == '':
			return
		buf = self.history.get_buffer()
		iter = buf.get_end_iter()
		offset = iter.get_offset()
		if iter.get_offset() != 0:
			buf.insert(iter, '\n')
		buf.insert(iter, text)
		mark = buf.create_mark('end', iter, False)
		self.history.scroll_to_mark(mark, 0)
		buf.delete_mark(mark)

	def cmd_entry(self, widget):
		self.reply.pop(0)
		if not self.history:
			return
		text = widget.get_text()
		if text == '':
			return
		self.update_history(text)
		args = text.split()
		msg = liblo.Message('/kunquat/' + args[0])
		for arg in args[1:]:
			if arg.isdigit():
				arg = int(arg)
			msg.add(arg)
		liblo.send(self.engine, msg)
		widget.set_text('')

	def notify(self, path, args, types):
		out = ' '.join([str(a) for a in args])
		self.reply.pop(0)
		self.reply.push(0, out)
		if self.history:
			out = '<kunquat> ' + out
			gobject.idle_add(self.update_history, out)

	def error(self, path, args, types):
		out = 'Error: ' + ' '.join([str(a) for a in args])
		self.reply.pop(0)
		self.reply.push(0, out)
		if self.history:
			out = '<kunquat> ' + out
			gobject.idle_add(self.update_history, out)

	def fallback(self, path, args, types):
		out = '<kunquat> ' + path + ' ' + ' '.join([t + ':' + str(a) for a, t in zip(args, types)])
		if self.history:
			gobject.idle_add(self.update_history, out)

	def set_log(self, log):
		self.history = log

	def handle_osc(self, path, args, types):
		ps = path.split('/')
		if ps[1] != 'kunquat_gtk':
			return
		gobject.idle_add(Cli.__dict__[ps[2]], self, path, args, types)

	def __init__(self, engine, server):
		self.engine = engine
		self.server = server

		self.server.add_method('/kunquat_gtk/notify', None, self.handle_osc)
		self.server.add_method('/kunquat_gtk/error', None, self.handle_osc)
		self.server.add_method(None, None, self.fallback)

		gtk.VBox.__init__(self)

		self.history = None

		self.reply = gtk.Statusbar()

		cmd = gtk.Entry()
		cmd.connect('activate', self.cmd_entry)

		self.pack_start(self.reply, False, False)
		self.reply.show()
		self.pack_end(cmd, False, False)
		cmd.show()


