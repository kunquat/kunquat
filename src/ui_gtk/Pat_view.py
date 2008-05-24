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
import gtk, gobject, cairo, pango
from gtk import gdk
import math


RELTIME_FULL_PART = 882161280
COLUMNS = 64


def time_add(t1, t2):
	res = (t1[0] + t2[0], t1[1] + t2[1])
	if res[1] >= RELTIME_FULL_PART:
		res = (res[0] + 1, res[1] - RELTIME_FULL_PART)
	elif res[1] < 0:
		res = (res[0] - 1, res[1] + RELTIME_FULL_PART)
	return res

def time_sub(t1, t2):
	return time_add(t1, (-t2[0], -t2[1]))


class Pat_info:
	
	def __init__(self, num, len):
		self.num = num
		self.len = len
		self.cols = [None for _ in range(COLUMNS)]


class Pat_view(gtk.Widget):

	def pat_info(self, path, args, types):
		self.pdata = Pat_info(args[0], (args[1], args[2]))

	def event_info(self, path, args, types):
		if self.pdata.cols[args[1]] == None:
			self.pdata.cols[args[1]] = {}
		self.pdata.cols[args[1]][(args[2], args[3], args[4])] = args[5:]

	def events_sent(self, path, args, types):
		self.do_redraw()

#	def handle_motion_notify(self, widget, event):
#		print('motion')
#		return True

	def handle_button_press(self, widget, event):
		print('button')
		self.grab_focus()
		return True

	def handle_focus_in(self, widget, event):
		print('focus in')
		return True

	def handle_focus_out(self, widget, event):
		print('focus out')
		return True

	def handle_key(self, widget, event):
		print('press')
		if not event.type == gdk.KEY_PRESS:
			return False
		print('keyval %s' % gdk.keyval_name(event.keyval))
		return True

	def do_realize(self):
		self.set_flags(self.flags()
				| gtk.REALIZED
				| gtk.SENSITIVE
				| gtk.PARENT_SENSITIVE
				| gtk.CAN_FOCUS
				| gtk.HAS_FOCUS)
		self.window = gdk.Window(
				self.get_parent_window(),
				width = self.allocation.width,
				height = self.allocation.height,
				window_type = gdk.WINDOW_CHILD,
				wclass = gdk.INPUT_OUTPUT,
				event_mask = self.get_events()
						| gdk.EXPOSURE_MASK
						| gdk.KEY_PRESS
#						| gdk.POINTER_MOTION_MASK
#						| gdk.POINTER_MOTION_HINT_MASK
						| gdk.BUTTON_PRESS_MASK
						| gdk.FOCUS_CHANGE)
		self.window.set_user_data(self)
		self.style.attach(self.window)
		self.style.set_background(self.window, gtk.STATE_NORMAL)
		self.window.move_resize(*self.allocation)
		self.connect('key-press-event', self.handle_key)
		self.connect('focus-in-event', self.handle_focus_in)
		self.connect('focus-out-event', self.handle_focus_out)
		self.connect('button-press-event', self.handle_button_press)
#		self.connect('motion-notify-event', self.handle_motion_notify)

	def do_unrealize(self):
		self.window.set_user_data(None)

	def do_size_request(self, requisition):
		pass

	def do_size_allocate(self, allocation):
		self.allocation = allocation
		if self.flags() & gtk.REALIZED:
			self.window.move_resize(*allocation)

	def do_expose_event(self, event):
		cr = self.window.cairo_create()
		cr.rectangle(event.area.x, event.area.y,
				event.area.width, event.area.height)
		cr.clip()
		self.draw(cr, *self.window.get_size())

	def draw(self, cr, width, height):
		cr.set_source_rgb(*self.ptheme['Background colour'])
		cr.rectangle(0, 0, width, height)
		cr.fill()
		cr.set_line_width(1)

		# Calculate space, make sure cursor is visible
		if self.cursor[0] < (0, 0, 0):
			self.cursor = ((0, 0, 0), self.cursor[1])
		if self.cursor[1] < 0:
			self.cursor = (self.cursor[0], 0)
		if self.cursor[1] < self.view_corner[1]:
			self.view_corner = (self.view_corner[0], self.cursor[1])
		if self.cursor[0] < self.view_corner[0]:
			self.view_corner = (self.cursor[0][:2], self.view_corner[1])
		col_space = width - self.ruler_width
		col_count = col_space // self.col_width
		col_last = self.view_corner[1] + col_count - 1
		if self.cursor[1] > col_last:
			diff = self.cursor[1] - col_last
			self.view_corner = (self.view_corner[0], self.view_corner[1] + diff)
		beat_space = height - self.col_font_size
		beat_count = divmod(beat_space, self.pixels_per_beat)
		beat_count = (int(beat_count[0]),
				int(beat_count[1] * RELTIME_FULL_PART // self.pixels_per_beat))
		beat_last = time_add(self.view_corner[0], beat_count)
		if self.cursor[0][:2] > beat_last:
			diff = time_sub(self.cursor[0][:2], beat_last)
			self.view_corner[0] = time_add(self.view_corner[0], diff)

		self.draw_ruler(cr, height, self.view_corner[0], beat_last)

		# Draw columns
		for col_num in range(self.view_corner[1], self.view_corner[1] + col_count):
			self.draw_column(cr, col_num,
					self.ruler_width + (col_num - self.view_corner[1]) * self.col_width,
					0, height, self.view_corner[0], beat_last)

	def time_to_pixels(self, time):
		px = float(time[0] * RELTIME_FULL_PART + time[1])
		return px * self.pixels_per_beat / RELTIME_FULL_PART

	def draw_ruler(self, cr, height, start, end):
		if not self.pdata:
			return
		pat_start = max((0, 0), time_sub((0, 0), start))
		pat_end = min(self.pdata.len, end)
		cr.set_source_rgb(*self.ptheme['Ruler bg colour'])
		cr.rectangle(0, self.time_to_pixels(pat_start) + self.col_font_size,
				self.ruler_width, self.time_to_pixels(pat_end))
		cr.fill()
		cr.set_source_rgb(*self.ptheme['Ruler fg colour'])
		end = pat_end
		for beat in range(max(start[0], 0), end[0] + 1):
			if not start <= (beat, 0) <= end:
				continue
			pl = self.create_pango_layout('%d' % beat)
			pl.set_font_description(self.ruler_font)
			rw, rh = pl.get_size()
			rw //= pango.SCALE
			rh //= pango.SCALE
			distance = self.time_to_pixels(time_sub((beat, 0), start))
			distance = distance + self.col_font_size - (rh / 2)
			cr.move_to(self.ruler_width - rw - 2, distance)
			cr.update_layout(pl)
			cr.show_layout(pl)
		cr.set_source_rgb(*self.ptheme['Border colour'])
		cr.move_to(self.ruler_width - 0.5, 0)
		cr.rel_line_to(0, height)
		cr.stroke()

	def draw_column(self, cr, num, x, y, height, start, end):
		if self.pdata.cols[num]:
			visible = [(k, v) for (k, v) in self.pdata.cols[num].iteritems()
					if start <= k <= end]
			visible.sort()
			for (k, v) in visible:
				self.draw_event(cr, k, v, x, y + self.col_font_size, height, start, end)

		if self.cursor[1] == num:
			distance = self.time_to_pixels(time_sub(self.cursor[0][:2], start))
			distance = distance + self.col_font_size
			cr.set_line_width(1.8)
			cr.move_to(x, distance)
			cr.rel_line_to(self.col_width, 0)
			cr.stroke()
			cr.set_line_width(1)

		cr.set_source_rgb(*self.ptheme['Column header bg colour'])
		cr.rectangle(x, y, self.col_width, self.col_font_size)
		cr.fill()

		cr.set_source_rgb(*self.ptheme['Column header fg colour'])
		pl = None
		if num == 0:
			pl = self.create_pango_layout('Global')
		else:
			pl = self.create_pango_layout('%02d' % num)
		pl.set_font_description(self.col_font)
		cw, ch = pl.get_size()
		cw //= pango.SCALE
		ch //= pango.SCALE
		cr.move_to(x + 0.5 * self.col_width - 0.5 * cw, y)
		cr.update_layout(pl)
		cr.show_layout(pl)

		cr.set_source_rgb(*self.ptheme['Border colour'])
		cr.move_to(x + self.col_width - 0.5, 0)
		cr.rel_line_to(0, height)
		cr.stroke()

	def draw_event(self, cr, pos, data, x, y, height, start, end):
		if not start <= pos <= end:
			return
		distance = self.time_to_pixels(time_sub(pos, start)) + y
		cr.set_source_rgb(1, 0.9, 0.8)
		cr.move_to(x, distance)
		cr.rel_line_to(self.col_width, 0)
		cr.stroke()

	def do_redraw(self):
		if self.window:
			alloc = self.get_allocation()
			rect = gdk.Rectangle(alloc.x, alloc.y,
					alloc.width, alloc.height)
			self.window.invalidate_rect(rect, True)
			self.window.process_updates(True)
		return True

	def __init__(self, engine, server, song_id):
		gtk.Widget.__init__(self)
		self.engine = engine
		self.server = server
		self.song_id = song_id

		self.pdata = None

		self.ptheme = {
				'Ruler font': 'Sans 10',
				'Column font': 'Sans 10',
				'Column width': 7,
				'Beat height': 80,
				'Background colour': (0, 0, 0),
				'Border colour': (0.6, 0.6, 0.6),
				'Ruler bg colour': (0, 0.1, 0.2),
				'Ruler fg colour': (0.7, 0.7, 0.7),
				'Column header bg colour': (0, 0.3, 0),
				'Column header fg colour': (0.8, 0.8, 0.8),
				}

		self.ruler_font = pango.FontDescription(self.ptheme['Ruler font'])
		self.ruler_font_size = self.ruler_font.get_size() // pango.SCALE
		self.ruler_width = self.ruler_font_size * 3
		self.col_font = pango.FontDescription(self.ptheme['Column font'])
		self.col_font_size = int(self.col_font.get_size() * 1.6) // pango.SCALE
		self.col_width = self.ptheme['Column width'] * self.col_font_size

		self.pixels_per_beat = self.ptheme['Beat height']
		# position format is ((beat, part), channel)
		fs_beats = float(self.col_font_size) / self.pixels_per_beat
		fs_time = (long(math.floor(fs_beats)),
				int((fs_beats - math.floor(fs_beats)) * RELTIME_FULL_PART))
		self.view_corner = (time_sub((0, 0), fs_time), 0)
		self.cursor = ((0, 0, 0), 0)


gobject.type_register(Pat_view)


