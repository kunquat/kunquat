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


def valid_optimise(key, val, env):
    if int(val) < 0 or int(val) > 3:
        raise Exception, 'Invalid optimisation value %s (must be in range 0..3)' % val


opts = Variables(['options.py'])
opts.AddVariables(
    ('optimise', 'Optimisation level (0..3).', 0, valid_optimise),
    BoolVariable('enable_debug', 'Build in debug mode.', True),
    BoolVariable('enable_tests', 'Build and run tests.', True),
    BoolVariable('enable_demo', 'Enable the command line demo', True),
    BoolVariable('with_jack', 'Build JACK support.', True),
    BoolVariable('with_ao', 'Build libao support.', True),
    BoolVariable('with_openal', 'Build OpenAL support.', True),
)


compile_flags = [
    '-std=c99',
    '-pedantic',
    '-Wall',
    '-Wextra',
    '-Werror',
]

env = Environment(options = opts, CCFLAGS = compile_flags)

Help(opts.GenerateHelpText(env))


if env['enable_debug']:
    env.Append(CCFLAGS = ['-g'])
else:
    env.Append(CCFLAGS = ['-DNDEBUG'])

if env['optimise'] > 0 and env['optimise'] <= 3:
    oflag = '-O%s' % env['optimise']
    env.Append(CCFLAGS = [oflag])


audio_found = False


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
        print('Error: Math library not found.')
        Exit(1)

    conf.env.Append(CCFLAGS = '-Dushort=uint16_t')
    conf.env.Append(CCFLAGS = '-Duint=uint32_t')
    if not conf.CheckLibWithHeader('wavpack', 'wavpack/wavpack.h', 'C'):
        print('Error: WavPack not found.')
        Exit(1)

    if not conf.CheckLibWithHeader('archive', 'archive.h', 'C'):
        print('Error: libarchive not found.')
        Exit(1)

    if env['with_jack']:
        if conf.CheckLibWithHeader('jack', 'jack/jack.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DENABLE_JACK')
        else:
            print('Warning: JACK driver was requested but JACK was not found.')
            env['with_jack'] = False
    
    need_pthread = env['with_ao'] or env['with_openal']
    pthread_found = False
    if need_pthread and not conf.CheckLibWithHeader('pthread', 'pthread.h', 'C'):
        names = []
        if env['with_ao']:
            names += ['ao']
            env['with_ao'] = False
        if env['with_openal']:
            names += ['OpenAL']
            env['with_openal'] = False
        dr = 'driver requires'
        if len(names) > 1:
            dr = 'drivers require'
        print('Warning: The requested %s %s pthread which was not found.' %
                (' and '.join(names), dr))

    if env['with_ao']:
        if conf.CheckLibWithHeader('ao', 'ao/ao.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DENABLE_AO')
        else:
            print('Warning: libao driver was requested but libao was not found.')
            env['with_ao'] = False

    if env['with_openal']:
        if conf.CheckLibWithHeader('openal', ['AL/al.h', 'AL/alc.h'], 'C') and\
           conf.CheckLibWithHeader('alut', 'AL/alut.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DENABLE_OPENAL')
        else:
            print('Warning: openal driver was requested but openal was not found.')
            env['with_openal'] = False

    if env['enable_tests'] and not conf.CheckLibWithHeader('check', 'check.h', 'C'):
        print('Error: Building of unit tests was requested but Check was not found.')
        Exit(1)
        
    env = conf.Finish()


#print 'Root: ' + env.Dump('LIBS')


Export('env')

if env['enable_demo']:
    SConscript('demo/SConscript')

SConscript('src/SConscript')


