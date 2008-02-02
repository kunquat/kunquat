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

	def cmd_entry(self, widget, entry):
		text = entry.get_text()
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
		entry.set_text('')

	def notify(self, path, args):
		out = '<kunquat>'
		for a in args:
			out = out + ' ' + str(a)
		gobject.idle_add(self.update_history, out)

	def fallback(self, path, args, types):
		out = '<kunquat> ' + path
		for a, t in zip(args, types):
			out = out + ' ' + str(t) + ':' + str(a)
		gobject.idle_add(self.update_history, out)

	def __init__(self, engine, server):
		self.engine = engine
		self.server = server
		
		self.server.add_method('/kunquat_gtk/notify', None, self.notify)
		self.server.add_method(None, None, self.fallback)

		gtk.VBox.__init__(self)

		self.history = gtk.TextView()
		self.history.set_editable(False)
		self.history.set_wrap_mode(gtk.WRAP_CHAR)

		hist_scroll = gtk.ScrolledWindow()
		hist_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_ALWAYS)
		hist_scroll.add(self.history)
		self.history.show()

		cmd = gtk.Entry()
		cmd.connect('activate', self.cmd_entry, cmd)

		self.pack_start(hist_scroll)
		hist_scroll.show()
		self.pack_end(cmd, False, False)
		cmd.show()


