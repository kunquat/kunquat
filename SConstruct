# coding=utf-8


# Copyright 2010 Tomi Jylhä-Ollila
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


import os


def valid_optimise(key, val, env):
    if int(val) < 0 or int(val) > 3:
        raise Exception, 'Invalid optimisation value %s (must be in range 0..3)' % val


opts = Variables(['options.py'])
opts.AddVariables(
    PathVariable('prefix', 'Installation prefix.', '/usr/local', PathVariable.PathIsDirCreate),
    ('optimise', 'Optimisation level (0..3).', 2, valid_optimise),
    BoolVariable('enable_debug', 'Build in debug mode.', True),
    BoolVariable('enable_libkunquat', 'Enable libkunquat.', True),
    BoolVariable('enable_libkunquat_dev', 'Install development files.', True),
    BoolVariable('enable_tests', 'Build and run libkunquat tests.', True),
    BoolVariable('enable_player', 'Enable kunquat-player.', True),
    BoolVariable('enable_export', 'Enable kunquat-export (requires libsndfile).', True),
    BoolVariable('enable_examples', 'Build example Kunquat files.', True),
    BoolVariable('with_wavpack', 'Build WavPack support (recommended).', True),
    BoolVariable('with_pulse', 'Build PulseAudio support.', True),
    BoolVariable('with_jack', 'Build JACK support.', True),
    BoolVariable('with_openal', 'Build OpenAL support.', True),
)


AddOption('--full-clean',
          dest='full-clean',
          nargs=0,
          help='Remove all generated files when cleaning')


compile_flags = [
    '-std=c99',
    '-pedantic',
    '-Wall',
    '-Wextra',
    '-Werror',
]

env = Environment(options = opts,
                  CCFLAGS = compile_flags,
                  HOME = os.environ['HOME'])

env.Alias('install', env['prefix'])

Help(opts.GenerateHelpText(env))


env['prefix'] = os.path.abspath(env['prefix'])

def InstallCreatePath(env, path, source):
    if 'install' in COMMAND_LINE_TARGETS and not os.path.exists(path):
        os.makedirs(path)
    return env.Install(path, source)

env.AddMethod(InstallCreatePath, "InstallCreatePath")


if env['enable_debug']:
    env.Append(CCFLAGS = ['-g'])
else:
    env.Append(CCFLAGS = ['-DNDEBUG'])

if env['optimise'] > 0 and env['optimise'] <= 3:
    oflag = '-O%s' % env['optimise']
    env.Append(CCFLAGS = [oflag])


audio_found = False


if not env.GetOption('clean') and not env.GetOption('help'):

    conf = Configure(env)

    conf_errors = []

    if not conf.CheckType('int8_t', '#include <stdint.h>'):
        conf.env.Append(CCFLAGS = '-Dint8_t=int_least8_t')
        conf.env.Append(CCFLAGS = '-DINT8_MIN=(-128)')
        conf.env.Append(CCFLAGS = '-DINT8_MAX=(127)')

    if not conf.CheckType('int16_t', '#include <stdint.h>'):
        conf.env.Append(CCFLAGS = '-Dint16_t=int_least8_t')
        conf.env.Append(CCFLAGS = '-DINT16_MIN=(-32768)')
        conf.env.Append(CCFLAGS = '-DINT16_MAX=(32767)')

    if not conf.CheckType('int32_t', '#include <stdint.h>'):
        conf.env.Append(CCFLAGS = '-Dint32_t=int_least32_t')
        conf.env.Append(CCFLAGS = '-DINT32_MIN=(-2147483648L)')
        conf.env.Append(CCFLAGS = '-DINT32_MAX=(2147483647L)')

    if not conf.CheckType('int64_t', '#include <stdint.h>'):
        conf.env.Append(CCFLAGS = '-Dint64_t=int_least64_t')
        conf.env.Append(CCFLAGS = '-DINT64_MIN=(-9223372036854775808LL)')
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
        conf_errors.append('Math library was not found.')

    if env['with_wavpack']:
        conf.env.Append(CCFLAGS = '-Dushort=uint16_t')
        conf.env.Append(CCFLAGS = '-Duint=uint32_t')
        if conf.CheckLibWithHeader('wavpack', 'wavpack/wavpack.h', 'C'):
            conf.env.Append(CCFLAGS = '-DWITH_WAVPACK')
        else:
            conf_errors.append('WavPack support was requested' +
                               ' but WavPack was not found.')
    else:
        print('Warning: WavPack support is disabled!' +
              ' Sample support will be very minimal.')

    if not conf.CheckLibWithHeader('archive', 'archive.h', 'C'):
        conf_errors.append('libarchive was not found.')

    if env['enable_player']:
        if not conf.CheckLibWithHeader('pthread', 'pthread.h', 'C'):
            conf_errors.append('kunquat-player was requested' +
                               ' but libpthread was not found.')

    if env['with_pulse']:
        if conf.CheckLibWithHeader('pulse-simple', 'pulse/simple.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DWITH_PULSE')
        else:
            print('Warning: PulseAudio support was requested' +
                  ' but PulseAudio was not found.')
            env['with_pulse'] = False

    if env['with_jack']:
        if conf.CheckLibWithHeader('jack', 'jack/jack.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DWITH_JACK')
        else:
            print('Warning: JACK support was requested' +
                  ' but JACK was not found.')
            env['with_jack'] = False

    if env['with_openal']:
        if conf.CheckLibWithHeader('openal', ['AL/al.h', 'AL/alc.h'], 'C') and\
           conf.CheckLibWithHeader('alut', 'AL/alut.h', 'C'):
            audio_found = True
            conf.env.Append(CCFLAGS = '-DWITH_OPENAL')
        else:
            print('Warning: OpenAL support was requested' +
                  ' but OpenAL was not found.')
            env['with_openal'] = False

    if env['enable_export']:
        if conf.CheckLibWithHeader('sndfile', 'sndfile.h', 'C'):
            env.ParseConfig('pkg-config --cflags --libs sndfile')
            conf.env.Append(CCFLAGS = '-DWITH_SNDFILE')
        else:
            print('Warning: kunquat-export was requested' +
                  ' but libsndfile was not found.')
            env['enable_export'] = False

    if env['enable_tests'] and not conf.CheckLibWithHeader('check', 'check.h', 'C'):
        conf_errors.append('Building of unit tests was requested' +
                           ' but Check was not found.')

    if conf_errors:
        print('\nCouldn\'t configure Kunquat due to the following error%s:\n' %
              's'[:len(conf_errors[1:])])
        print('\n'.join(conf_errors) + '\n')
        Exit(1)
        
    env = conf.Finish()

elif GetOption('full-clean') != None:
    if os.path.exists('.sconf_temp'):
        for file in os.listdir('.sconf_temp'):
            os.remove('.sconf_temp/' + file)
        os.rmdir('.sconf_temp')
    if os.path.exists('.sconsign.dblite'):
        os.remove('.sconsign.dblite')
    if os.path.exists('config.log'):
        os.remove('config.log')


#print('Root: ' + env.Dump('LIBS'))


if not env.GetOption('help'):
    Export('env')

    if env['enable_examples']:
        SConscript('examples/SConscript')

    SConscript('src/SConscript')


