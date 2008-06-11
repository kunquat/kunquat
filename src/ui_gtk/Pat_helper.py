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


RELTIME_FULL_PART = 882161280
COLUMNS = 64
PATTERNS = 1024
INSTRUMENTS_MAX = 256


class Ev:

	def __init__(self):
		i = 0
		events = (
			('NONE', 0),
			'GENERAL_COND',
			('GENERAL_LAST', 63),
			'GLOBAL_SET_VAR',
			'GLOBAL_TEMPO',
			'GLOBAL_VOLUME',
			('GLOBAL_LAST', 127),
			'NOTE_ON',
			'NOTE_OFF',
			'LAST')
		for x in events:
			if type(x) == type(()):
				setattr(self, *x)
				i = x[1]
			else:
				setattr(self, x, i)
			i += 1

	def is_general(self, t):
		return self.NONE < t < self.GENERAL_LAST

	def is_global(self, t):
		return self.GENERAL_LAST < t < self.GLOBAL_LAST

	def is_ins(self, t):
		return self.GLOBAL_LAST < t < self.LAST

	def is_valid(self, t):
		return self.is_general(t) or self.is_global(t) or self.is_ins(t)


def time_normalise(t):
	if t[1] >= RELTIME_FULL_PART:
		t = (t[0] + 1, t[1] - RELTIME_FULL_PART)
	elif t[1] < 0:
		t = (t[0] - 1, t[1] + RELTIME_FULL_PART)
	return t

def time_add(t1, t2):
	res = (t1[0] + t2[0], t1[1] + t2[1])
	return time_normalise(res)

def time_sub(t1, t2):
	return time_add(t1, (-t2[0], -t2[1]))


def rgb_scale(r, g, b):
	r *= 65535
	g *= 65535
	b *= 65535
	r = max(0, min(65535, r))
	g = max(0, min(65535, g))
	b = max(0, min(65535, b))
	return (int(r), int(g), int(b))

def colour_for_bg(r, g, b, br, bg, bb):
	energy = 0.3 * br + 0.59 * bg + 0.11 * bb
	radd = gadd = badd = 0
	if energy <= 0.5:
		radd, gadd, badd = (1 - r, 1 - g, 1 - b)
		add = energy * 2
		radd *= add
		gadd *= add
		badd *= add
		return (r + radd, g + gadd, b + badd)
	r, g, b = (r - 0.5, g - 0.5, b - 0.5)
	add = (energy - 0.5) * 2
	r *= add
	g *= add
	b *= add
	return (r, g, b)


