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

import re


class Note_table:

	def __init__(self,
			name,
			note_count,
			note_mod_count,
			ref_note,
			cur_ref_note,
			ref_pitch,
			oct_is_ratio,
			octave_ratio):
		self.name = name
		self.note_count = note_count
		self.note_mod_count = note_mod_count
		self.ref_note = ref_note
		self.cur_ref_note = cur_ref_note
		self.ref_pitch = ref_pitch
		self.oct_is_ratio = oct_is_ratio
		self.octave_ratio = octave_ratio
		self.notes = [None for _ in range(128)]
		self.note_mods = [None for _ in range(16)]

	def str_oct(self):
		if self.oct_is_ratio:
			if type(self.octave_ratio) == float:
				return str(self.octave_ratio)
			if type(self.octave_ratio[0]) == float:
				return str(self.octave_ratio[0])
			return str(self.octave_ratio[0]) + '/' + str(self.octave_ratio[1])
		return str(self.octave_ratio) + 'c'

	def str_note_ratio(self, mod, index):
		value = None
		if mod:
			if not self.note_mods[index]:
				return ''
			value = self.note_mods[index]
		else:
			if not self.notes[index]:
				return ''
			value = self.notes[index]
		if value[1]:
			if type(value[2]) == float:
				return str(value[2])
			return str(value[2][0]) + '/' + str(value[2][1])
		else:
			return str(value[2]) + 'c'


def parse_ratio(s):
	r = re.compile(r"""
			\s*
			(
			(                             # fraction
			 (\d*[1-9]\d*)                #    a non-zero numerator
			 \s*/\s*
			 (\d*[1-9]\d*)                #    a non-zero denominator
			)
			|
			(
			 (                            # positive float
			  (0*\.\d*[1-9]\d*)           #    < 1
			  |
			  (\d*[1-9]\d*(\.\d*)?)       #    >= 1
			 )
			 |
			 (                            # cents
			  ((-?\d+(\.\d*)?)(\s*(c)))   #    digits before the decimal point
			  |
			  ((-?\d*\.\d+)(\s*(c)))      #    digits after the decimal point
			 )
			)
			)
			\s*$
			""",
			re.VERBOSE)
	m = r.match(s)
	if not m:
		return None
#	return m.groups()
	return m.group(3, 4, 6, 12)


class Note_tables(gtk.HBox):

	def note_table_info(self, path, args, types):
		iter = self.list_view.get_model().get_iter(args[0])
		self.table_list.set_value(iter, 1, args[1])
		oct_is_ratio = args[7]
		octave_ratio = args[8]
		if oct_is_ratio and types[8] == 'h':
			octave_ratio = (args[8], args[9])
		self.tables[args[0]] = Note_table(args[1],
				args[2],
				args[3],
				args[4],
				args[5],
				args[6],
				oct_is_ratio,
				octave_ratio)

	def note_info(self, path, args, types):
		if not self.tables[args[0]]:
			return
		is_ratio = args[3]
		pitch = ()
		retuned = ()
		if is_ratio and types[4] == 'h':
			pitch = (args[4], args[5])
			retuned = (args[6], args[7])
		else:
			pitch = args[4]
			retuned = args[5]
		self.tables[args[0]].notes[args[1]] = (args[2], is_ratio, pitch, retuned)

	def note_mod_info(self, path, args, types):
		if not self.tables[args[0]]:
			return
		is_ratio = args[3]
		pitch = ()
		if is_ratio:
			pitch = (args[4], args[5])
		else:
			pitch = argss[4]
		self.tables[args[0]].note_mods[args[1]] = (args[2], is_ratio, pitch)

	def notes_sent(self, path, args, types):
		if not self.tables[args[0]]:
			return
		if self.cur_index == args[0]:
			self.set_details(args[0])

	def change_note_table_name(self, cell, path, new_text):
		liblo.send(self.engine, '/kunquat/set_note_table_name',
				self.song_id,
				int(path),
				new_text)

	def select_table(self, selection):
		_, cur = selection.get_selected_rows()
		if not cur:
			return
		self.set_details(cur[0][0])

	def change_ref_note(self, combobox):
		note_index = combobox.get_active()
		if note_index < 0:
			return
		liblo.send(self.engine, '/kunquat/set_note_table_ref_note',
				self.song_id,
				self.cur_index,
				note_index)

	def change_ref_pitch(self, adj):
		pitch = adj.get_value()
		if pitch <= 0:
			return
		self.user_set_pitch_center = True
		liblo.send(self.engine, '/kunquat/set_note_table_ref_pitch',
				self.song_id,
				self.cur_index,
				pitch)

	def change_oct_ratio(self, entry, event):
		parsed = parse_ratio(entry.get_text())
		if not parsed:
			return
		is_ratio = not parsed[3]
		value = ()
		if is_ratio:
			if parsed[0]:
				value = (long(parsed[0]), long(parsed[1]))
			else:
				assert parsed[2]
				value = (('d', float(parsed[2])),)
		else:
			value = (('d', float(parsed[3])),)
		liblo.send(self.engine, '/kunquat/set_note_table_octave_ratio',
				self.song_id,
				self.cur_index,
				is_ratio,
				*value)
	
	def change_note_name(self, cell, path, new_text):
		if not new_text:
			return
		liblo.send(self.engine, '/kunquat/set_note_name',
				self.song_id,
				self.cur_index,
				int(path),
				new_text)

	def change_note_ratio(self, cell, path, new_text):
		parsed = parse_ratio(new_text)
		if not parsed:
			return
		is_ratio = not parsed[3]
		value = ()
		if is_ratio:
			if parsed[0]:
				value = (long(parsed[0]), long(parsed[1]))
			else:
				assert parsed[2]
				value = (('d', float(parsed[2])),)
		else:
			value = (('d', float(parsed[3])),)
		liblo.send(self.engine, '/kunquat/set_note_ratio',
				self.song_id,
				self.cur_index,
				int(path),
				is_ratio,
				*value)

	def set_details(self, index):
		if not self.tables[index]:
			self.ref_note.handler_block(self.href_note)
			self.ref_note.get_model().clear()
			self.ref_note.handler_unblock(self.href_note)
			self.ref_pitch.get_adjustment().handler_block(self.href_pitch)
			self.ref_pitch.set_value(440)
			self.ref_pitch.get_adjustment().handler_unblock(self.href_pitch)
			self.oct_ratio.set_text('2/1')
			for i in range(128):
				iter = self.notes_view.get_model().get_iter(i)
				self.notes_list.set_value(iter, 1, '')
				self.notes_list.set_value(iter, 2, '')
			self.cur_index = index
			return
		i = 0
		for note in self.tables[index].notes:
			iter = self.notes_view.get_model().get_iter(i)
			if not note:
				self.notes_list.set_value(iter, 1, '')
				self.notes_list.set_value(iter, 2, '')
			else:
				self.notes_list.set_value(iter, 1, note[0])
				self.notes_list.set_value(iter, 2,
						self.tables[index].str_note_ratio(False, i))
			i += 1
		self.ref_note.handler_block(self.href_note)
		ref_note_model = self.ref_note.get_model()
		iter = ref_note_model.get_iter_first()
		for note in self.tables[index].notes:
			if not note and not iter:
				break
			elif note and iter:
				ref_note_model.set_value(iter, 0, note[0])
				iter = ref_note_model.iter_next(iter)
			elif not iter:
				iter = ref_note_model.append()
				ref_note_model.set_value(iter, 0, note[0])
				iter = None
			elif not note:
				if not ref_note_model.remove(iter):
					iter = None
		self.ref_note.set_model(ref_note_model)
		self.ref_note.set_active(self.tables[index].ref_note)
		self.ref_note.handler_unblock(self.href_note)
		if not self.user_set_pitch_center:
			self.ref_pitch.get_adjustment().handler_block(self.href_pitch)
			self.ref_pitch.set_value(self.tables[index].ref_pitch)
			self.ref_pitch.get_adjustment().handler_unblock(self.href_pitch)
		else:
			self.user_set_pitch_center = False
		self.oct_ratio.set_text(self.tables[index].str_oct())
		self.cur_index = index

	def __init__(self, engine, server, song_id):
		self.engine = engine
		self.server = server
		self.song_id = song_id

		self.cur_index = 0
		self.tables = [None for _ in range(16)]

		self.user_set_pitch_center = False

		detail_box = gtk.VBox()
		label = gtk.Label('Table data')
		detail_box.pack_start(label, False, False)
		label.show()

		general_table = gtk.Table(3, 2)

		label = gtk.Label('Pitch center:')
		general_table.attach(label, 0, 1, 0, 1)
		label.show()
		ref_note_model = gtk.ListStore(str)
		self.ref_note = gtk.ComboBox(ref_note_model)
		cell = gtk.CellRendererText()
		self.ref_note.pack_start(cell, True)
		self.ref_note.add_attribute(cell, 'text', 0)
		self.href_note = self.ref_note.connect('changed', self.change_ref_note)
		general_table.attach(self.ref_note, 1, 2, 0, 1)
		self.ref_note.show()

		label = gtk.Label('Reference pitch:')
		general_table.attach(label, 0, 1, 1, 2)
		label.show()
		ref_pitch_adj = gtk.Adjustment(440, 1, 32767, 0.01)
		self.ref_pitch = gtk.SpinButton(ref_pitch_adj, climb_rate=0.5, digits=2)
		self.ref_pitch.set_snap_to_ticks(True)
		self.ref_pitch.set_numeric(True)
		self.href_pitch = ref_pitch_adj.connect('value-changed', self.change_ref_pitch)
		general_table.attach(self.ref_pitch, 1, 2, 1, 2)
		self.ref_pitch.show()

		label = gtk.Label('Octave ratio:')
		general_table.attach(label, 0, 1, 2, 3)
		label.show()
		self.oct_ratio = gtk.Entry()
		self.oct_ratio.connect('activate', self.change_oct_ratio, None)
		self.oct_ratio.connect('focus-out-event', self.change_oct_ratio)
		general_table.attach(self.oct_ratio, 1, 2, 2, 3)
		self.oct_ratio.set_text('2/1')
		self.oct_ratio.show()

		detail_box.pack_start(general_table, False, False)
		general_table.show()

		self.notes_list = gtk.ListStore(gobject.TYPE_INT,
				gobject.TYPE_STRING,
				gobject.TYPE_STRING)
		self.notes_view = gtk.TreeView(self.notes_list)
		#selection = self.notes_view.get_selection()
		#selection.connect('changed', self.select_note)

		for i in range(128):
			iter = self.notes_list.append()
			self.notes_list.set(iter, 0, i)

		#selection.select_path(0)

		cell = gtk.CellRendererText()
		column = gtk.TreeViewColumn('#', cell, text=0)
		self.notes_view.append_column(column)
		cell = gtk.CellRendererText()
		cell.set_property('editable', True)
		cell.connect('edited', self.change_note_name)
		column = gtk.TreeViewColumn('Name', cell, text=1)
		self.notes_view.append_column(column)
		cell = gtk.CellRendererText()
		cell.set_property('editable', True)
		cell.connect('edited', self.change_note_ratio)
		column = gtk.TreeViewColumn('Ratio', cell, text=2)
		self.notes_view.append_column(column)
		notes_scroll = gtk.ScrolledWindow()
		notes_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		notes_scroll.add(self.notes_view)
		self.notes_view.show()
		detail_box.pack_start(notes_scroll)
		notes_scroll.show()

		gtk.HBox.__init__(self)

		list_box = gtk.VBox()
		label = gtk.Label('List of tables')
		list_box.pack_start(label, False, False)
		label.show()

		self.table_list = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_STRING)
		self.list_view = gtk.TreeView(self.table_list)
		selection = self.list_view.get_selection()
		selection.connect('changed', self.select_table)

		for i in range(16):
			iter = self.table_list.append()
			self.table_list.set(iter, 0, '%X' % i)

		selection.select_path(0)

		cell = gtk.CellRendererText()
		column = gtk.TreeViewColumn('#', cell, text=0)
		self.list_view.append_column(column)
		cell = gtk.CellRendererText()
		cell.set_property('editable', True)
		cell.connect('edited', self.change_note_table_name)
		column = gtk.TreeViewColumn('Name', cell, text=1)
		self.list_view.append_column(column)

		list_scroll = gtk.ScrolledWindow()
		list_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		list_scroll.add(self.list_view)
		self.list_view.show()

		list_box.pack_start(list_scroll)
		list_scroll.show()

		self.pack_start(list_box)
		list_box.show()

		self.pack_start(detail_box)
		detail_box.show()


