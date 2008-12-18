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

import liblo


class Envelope(gtk.Widget):

	def handle_key(self, widget, event):
		pass

	def handle_focus_in(self, widget, event):
		pass

	def handle_focus_out(self, widget, event):
		pass

	def handle_button(self, widget, event):
		pass

	def handle_motion(self, widget, event):
		x = event.x - self.ruler_width
		y = self.margin_top + self.view_height - event.y
		self.pointer_coords = (x / self.zoom_x, y / self.zoom_y)
		self.queue_draw()
		return True

	def do_realize(self):
		self.set_flags(self.flags()
				| gtk.REALIZED
				| gtk.SENSITIVE
				| gtk.PARENT_SENSITIVE
				| gtk.CAN_FOCUS)
		self.window = gdk.Window(
				self.get_parent_window(),
				width = self.allocation.width,
				height = self.allocation.height,
				window_type = gdk.WINDOW_CHILD,
				wclass = gdk.INPUT_OUTPUT,
				event_mask = self.get_events()
						| gdk.EXPOSURE_MASK
						| gdk.KEY_PRESS
						| gdk.BUTTON_PRESS_MASK
						| gdk.POINTER_MOTION_MASK
						| gdk.POINTER_MOTION_HINT_MASK
						| gdk.FOCUS_CHANGE)
		self.window.set_user_data(self)
		self.style.attach(self.window)
		self.style.set_background(self.window, gtk.STATE_NORMAL)
		self.window.move_resize(*self.allocation)
		self.connect('key-press-event', self.handle_key)
		self.connect('focus-in-event', self.handle_focus_in)
		self.connect('focus-out-event', self.handle_focus_out)
		self.connect('button-press-event', self.handle_button)
		self.connect('motion-notify-event', self.handle_motion)

	def do_unrealize(self):
		self.window.destroy()
		self.window.set_user_data(None)

	def do_size_request(self, requisition):
		if not self.scale_y and self.view_height <= 128:
			requisition.height = self.view_height + self.ruler_height + self.margin_top
		if not self.scale_x and self.view_width <= 256:
			requisition.width = self.view_width + self.ruler_width + self.margin_right

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

		max_x = 0
		max_y = 0
		for node in self.nodes:
			if max_x < abs(node[0]):
				max_x = abs(node[0])
			if max_y < abs(node[1]):
				max_y = abs(node[1])

		view_width = width - self.ruler_width - self.margin_right
		view_height = height - self.ruler_height - self.margin_top
		if self.scale_x and max_x > (view_width + 1) / self.zoom_x:
			self.zoom_x = 2**math.floor(math.log((view_width + 1) / max_x, 2))
		if self.scale_y and max_y > (view_height + 1) / self.zoom_y:
			self.zoom_y = 2**math.floor(math.log((view_height + 1) / max_y, 2))

		coords_str = '---'
		if self.pointer_coords:
			coords_str = '(%f, %f)' % self.pointer_coords
		pl = self.create_pango_layout(coords_str)
		pl.set_font_description(self.ruler_font)
		w, h = pl.get_size()
		w //= pango.SCALE
		h //= pango.SCALE
		cr.set_source_rgb(*self.ptheme['Node colour'])
		cr.move_to(width - w - self.margin_right, self.margin_top)
		cr.update_layout(pl)
		cr.show_layout(pl)

		self.draw_ruler_y(cr, width, height)
		self.draw_ruler_x(cr, width, height)

		cr.set_source_rgb(*self.ptheme['Line colour'])
		prev = None
		for node in self.nodes:
			if prev:
				prev_y = self.margin_top + self.view_height - (prev[1] * self.zoom_y)
				prev_x = self.ruler_width + (prev[0] * self.zoom_x)
				y = self.view_height - (node[1] * self.zoom_y)
				x = (node[0] * self.zoom_x)
				cr.move_to(prev_x, prev_y)
				cr.rel_line_to(x, y)
				cr.stroke()
			prev = node

		for node in self.nodes:
			self.draw_node(cr, self.ruler_width + (node[0] * self.zoom_x) + 0.5,
					self.margin_top + self.view_height -
					(node[1] * self.zoom_y) - 0.5,
					False)

	def draw_ruler_y(self, cr, width, height):
		cr.set_source_rgb(*self.ptheme['Border colour'])
		cr.move_to(self.ruler_width + 0.5, self.margin_top)
		cr.rel_line_to(0, self.view_height)
		cr.stroke()

	def draw_ruler_x(self, cr, width, height):
		cr.set_source_rgb(*self.ptheme['Border colour'])
		cr.move_to(self.ruler_width, self.view_height + self.margin_top - 0.5)
		cr.rel_line_to(self.view_width, 0)
		cr.stroke()

	def draw_node(self, cr, x, y, selected):
		if selected:
			cr.set_source_rgb(*self.ptheme['Node highlight colour'])
		else:
			cr.set_source_rgb(*self.ptheme['Node colour'])
		cr.rectangle(x - self.node_width / 2, y - self.node_width / 2,
				self.node_width, self.node_height)
		cr.fill()

	def do_redraw(self):
		if self.window:
			alloc = self.get_allocation()
			rect = gdk.Rectangle(alloc.x, alloc.y,
					alloc.width, alloc.height)
			self.window.invalidate_rect(rect, True)
			self.window.process_updates(True)
		return True

	def __init__(self, engine, server, song_id,
			x_desc, y_desc):
		gtk.Widget.__init__(self)
		self.engine = engine
		self.server = server

		self.ptheme = {
			'Ruler font': 'Sans 8',
			'Background colour': (0, 0, 0),
			'Border colour': (0.6, 0.6, 0.6),
			'Ruler bg colour': (0, 0.1, 0.2),
			'Ruler fg colour': (0.7, 0.7, 0.7),
			'Line colour': (0.7, 0.5, 0.4),
			'Node colour': (1, 0.9, 0.8),
			'Node highlight colour': (1, 1, 1),
		}

		self.ruler_font = pango.FontDescription(self.ptheme['Ruler font'])
		self.ruler_font_size = self.ruler_font.get_size() // pango.SCALE
		self.ruler_width = self.ruler_font_size * 5
		self.ruler_height = self.ruler_font_size * 3

		self.min_x, self.max_x, self.step_x, self.scale_x = x_desc
		self.min_y, self.max_y, self.step_y, self.scale_y = y_desc

		self.zoom_x = 1.0 / self.step_x
		self.zoom_y = 1.0 / self.step_y

		self.view_height = (self.max_y - self.min_y) // self.step_y
		self.view_width = (self.max_x - self.min_x) // self.step_x

		self.margin_top = 8
		self.margin_right = 8

		self.node_width = 3.0
		self.node_height = 3.0

		self.nodes = []

		self.pointer_coords = None


gobject.type_register(Envelope)


