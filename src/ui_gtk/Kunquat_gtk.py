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

import sys

import Driver_select
import Songs
import Cli


gobject.threads_init()


class Kunquat_gtk():

	def connect(self, url):
		self.engine_url = url
		self.engine = liblo.Address(url)
		self.s = liblo.ServerThread(reg_methods = False)
		self.s.start()

	def quit(self, widget, event, data = None):
		return False

	def destroy(self, widget, data = None):
		self.s.stop()
		self.s = None
		self.engine = None
		self.cli = None
		self.songs = None
		self.drivers = None
		self.box = None
		self.tools = None
		self.contents = None
		self.window = None
		gtk.main_quit()

	def set_drivers_display(self, button, data = None):
		if button.get_active():
			self.drivers.show()
		else:
			self.drivers.hide()

	def hide_drivers(self, window, event, button):
		window.hide()
		button.set_active(False)
		return True

	def __init__(self, engine_url):
		self.connect(engine_url)

		self.drivers = Driver_select.Driver_select(self.engine, self.s)

		self.songs = Songs.Songs(self.engine, self.s)

		self.cli = Cli.Cli(self.engine, self.s)
		
		self.window = gtk.Window()
		self.window.set_title('Kunquat')
		self.window.connect('delete_event', self.quit)
		self.window.connect('destroy', self.destroy)

		self.contents = gtk.VBox()

		self.tools = gtk.Toolbar()
		driver_button = gtk.ToggleToolButton()
		driver_button.set_label('Drivers')
		driver_button.connect('toggled', self.set_drivers_display)
		self.tools.insert(driver_button, 0)
		driver_button.show()
		self.drivers.connect('delete_event', self.hide_drivers, driver_button)
		# Decrease reference count in order to prevent freeze at exit
		# -- what's the correct way to handle this?
		driver_button = None

		self.box = gtk.HBox()
		self.box.pack_start(self.songs)
		self.songs.show()
		self.box.pack_start(self.cli)
		self.cli.show()

		self.contents.pack_start(self.tools, expand = False)
		self.tools.show()
		self.contents.pack_end(self.box)
		self.box.show()

		self.window.add(self.contents)
		self.contents.show()

		self.window.show()

		liblo.send(self.engine,
				'/kunquat/register_host',
				'osc.udp://localhost:' + str(self.s.get_port()) + '/kunquat_gtk/')

		liblo.send(self.engine, '/kunquat/set_voices', 64)
		liblo.send(self.engine, '/kunquat/get_drivers')
		liblo.send(self.engine, '/kunquat/get_songs')

	def main(self):
		gtk.main()


if __name__ == '__main__':
	try:
		app = Kunquat_gtk('osc.udp://localhost:7770/')
	except liblo.AddressError, err:
		print 'Couldn\'t allocate the Address object: ', str(err)
		sys.exit()
	except liblo.ServerError, err:
		print 'Couldn\'t register as an OSC server: ', str(err)
		sys.exit()
	app.main()


