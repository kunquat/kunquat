

import pygtk
pygtk.require('2.0')
import gtk, gobject, cairo, pango
from gtk import gdk


RELTIME_FULL_PART = 882161280
COLUMNS = 64


def time_add(t1, t2):
	res = (t1[0] + t2[0], t1[1] + t2[1])
	if res[1] >= RELTIME_FULL_PART:
		res[0] += 1
		res[1] -= RELTIME_FULL_PART
	elif res[1] < 0:
		res[0] -= 1
		res[1] += RELTIME_FULL_PART
	return res

def time_sub(t1, t2):
	return time_add(t1, (-t2[0], -t2[1]))


class Pat_info:
	
	def __init__(self, length):
		self.length = length
		self.columns = [None for _ in range(COLUMNS)]


class Pat_view(gtk.Widget):

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
		cr.set_source_rgb(0, 0, 0)
		cr.rectangle(0, 0, width, height)
		cr.fill()
		cr.set_line_width(1)

		# Calculate space, make sure cursor is visible
		if self.cursor[1] < self.view_corner[1]:
			self.view_corner = (self.view_corner[0], self.cursor[1])
		if self.cursor[0] < self.view_corner[0]:
			self.view_corner = (self.cursor[0], self.view_corner[1])
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
		if self.cursor[0] > beat_last:
			diff = time_sub(self.cursor[0], beat_last)
			self.view_corner[0] = time_add(self.view_corner[0], diff)

		self.draw_ruler(cr, height, self.view_corner[0], beat_last)

		# Draw columns
		for col_num in range(self.view_corner[1], self.view_corner[1] + col_count):
			self.draw_column(cr, col_num,
					self.ruler_width + (col_num - self.view_corner[1]) * self.col_width,
					0, height)

	def draw_ruler(self, cr, height, start, end):
		cr.set_source_rgb(0.6, 0.6, 0.6)
		for beat in range(start[0], end[0] + 1):
			if not start <= (beat, 0) <= end:
				continue
			pl = self.create_pango_layout('%d' % beat)
			pl.set_font_description(self.ruler_font)
			rw, rh = pl.get_size()
			rw //= pango.SCALE
			rh //= pango.SCALE
			dtime = time_sub((beat, 0), start)
			distance = dtime[0] * RELTIME_FULL_PART + dtime[1]
			distance = distance * self.pixels_per_beat / RELTIME_FULL_PART
			distance = distance + self.col_font_size - (rh / 2)
			cr.move_to(self.ruler_width - rw, distance)
			cr.update_layout(pl)
			cr.show_layout(pl)
		cr.set_source_rgb(0.6, 0.6, 0.6)
		cr.move_to(self.ruler_width - 0.5, 0)
		cr.rel_line_to(0, height)
		cr.stroke()

	def draw_column(self, cr, num, x, y, height):
		# header and right border
		cr.set_source_rgb(0, 0.3, 0)
		cr.rectangle(x, y, self.col_width, self.col_font_size)
		cr.fill()

		cr.set_source_rgb(0.8, 0.8, 0.8)
		pl = self.create_pango_layout('%02d' % num)
		pl.set_font_description(self.col_font)
		cw, ch = pl.get_size()
		cw //= pango.SCALE
		ch //= pango.SCALE
		cr.move_to(x + 0.5 * self.col_width - 0.5 * cw, y)
		cr.update_layout(pl)
		cr.show_layout(pl)

		cr.set_source_rgb(0.6, 0.6, 0.6)
		cr.move_to(x + self.col_width - 0.5, 0)
		cr.rel_line_to(0, height)
		cr.stroke()

	#def draw_event(self, XXX):

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

		self.ruler_font = pango.FontDescription('Sans 10')
		self.ruler_font_size = self.ruler_font.get_size() // pango.SCALE
		self.ruler_width = self.ruler_font_size * 4
		self.col_width = 100
		self.col_font = pango.FontDescription('Sans 10')
		self.col_font_size = (self.col_font.get_size() * 1.6) // pango.SCALE

		self.pixels_per_beat = 64
		# position format is ((beat, part), channel)
		self.view_corner = ((0, 0), 0)
		self.cursor = ((0, 0), 0)


gobject.type_register(Pat_view)


