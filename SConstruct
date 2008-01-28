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


opts = Options(['options.py'])
opts.AddOptions(
	BoolOption('debug', 'Build in debug mode.', True),
	BoolOption('tests', 'Build and run tests.', True)
)


compile_flags = [
'-std=c99',
'-Wall',
'-Wextra',
'-Werror',
'-O2',
]

env = Environment(options = opts, CCFLAGS = compile_flags)

Help(opts.GenerateHelpText(env))


if env['debug']:
	env.Append(CCFLAGS = ['-g'])
else:
	env.Append(CCFLAGS = ['-DNDEBUG'])


if not env.GetOption('clean'):

	conf = Configure(env)

	if not conf.CheckType('int8_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Dint8_t=int_least8_t')
		conf.env.Append(CCFLAGS = '-DINT8_MIN=(-127)')
		conf.env.Append(CCFLAGS = '-DINT8_MAX=(127)')

	if not conf.CheckType('int16_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Dint16_t=int_least8_t')
		conf.env.Append(CCFLAGS = '-DINT16_MIN=(-32767)')
		conf.env.Append(CCFLAGS = '-DINT16_MAX=(32767)')

	if not conf.CheckType('int32_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Dint32_t=int_least32_t')
		conf.env.Append(CCFLAGS = '-DINT32_MIN=(-2147483647L)')
		conf.env.Append(CCFLAGS = '-DINT32_MAX=(2147483647L)')

	if not conf.CheckType('int64_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Dint64_t=int_least64_t')
		conf.env.Append(CCFLAGS = '-DINT64_MIN=(-9223372036854775807LL)')
		conf.env.Append(CCFLAGS = '-DINT64_MAX=(9223372036854775807LL)')

	if not conf.CheckType('uint8_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Duint8_t=uint_least8_t')
		conf.env.Append(CCFLAGS = '-DUINT8_MAX=(255)')

	if not conf.CheckType('uint16_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Duint16_t=uint_least16_t')
		conf.env.Append(CCFLAGS = '-DUINT16_MAX=(65535)')

	if not conf.CheckType('uint32_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Duint32_t=uint_least32_t')
		conf.env.Append(CCFLAGS = '-DUINT32_MAX=(4294967295UL)')

	if not conf.CheckType('uint64_t', '#include <stdint.h>'):
		conf.env.Append(CCFLAGS = '-Duint64_t=uint_least64_t')
		conf.env.Append(CCFLAGS = '-DUINT64_MAX=(18446744073709551615ULL)')

	if not conf.CheckLibWithHeader('m', 'math.h', 'C'):
		print 'Math library not found.'
		Exit(1)

	if not conf.CheckLibWithHeader('lo', 'lo/lo.h', 'C'):
		print 'liblo not found.'

	if not conf.CheckLibWithHeader('jack', 'jack/jack.h', 'C'):
		print 'JACK not found.'
		Exit(1)
	
	if env['tests'] and not conf.CheckLibWithHeader('check', 'check.h', 'C'):
		print 'Building of unit tests requires Check.'
		Exit(1)
		
	env = conf.Finish()


#print 'Root: ' + env.Dump('LIBS')


Export('env')

SConscript('src/SConscript')


