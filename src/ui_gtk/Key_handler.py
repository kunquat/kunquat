# coding=utf-8


# Copyright 2009 Tomi Jylhä-Ollila
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
import gtk.gdk
import gobject

import liblo


class Key_handler(gtk.Window):

    def handle_key(self, widget, event):
#       print(event.keyval)
        pass

    def set_key(self, key, action):
        if key in self.keys_to_act:
            pass # key conflict

    def __init__(self, engine, server):
        self.engine = engine
        self.server = server

        keys_to_act = {}
        act_to_keys = {}

        gtk.Window.__init__(self)


