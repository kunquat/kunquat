# coding=utf-8


# Author: Tomi Jylhä-Ollila, Finland 2010-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.


import os
import os.path
import shutil
import subprocess


def valid_optimise(key, val, env):
    if int(val) < 0 or int(val) > 3:
        raise Exception, 'Invalid optimisation value %s (must be in range 0..3)' % val


opts = Variables(['options.py'])
opts.AddVariables(
    PathVariable('prefix', 'Installation prefix.', '/usr/local', PathVariable.PathIsDirCreate),
    ('optimise', 'Optimisation level (0..3).', 2, valid_optimise),
    BoolVariable('enable_debug', 'Build in debug mode.', True),
    BoolVariable('enable_kunquat_assert', 'Enable internal assert code.', True),
    BoolVariable('enable_profiling', 'Build profiling code.', False),
    BoolVariable('enable_libkunquat', 'Enable libkunquat.', True),
    BoolVariable('enable_libkunquat_dev', 'Install development files.', True),
    BoolVariable('enable_python_bindings', 'Install Python bindings.', True),
    BoolVariable('enable_tests', 'Build and run libkunquat tests.', True),
    BoolVariable('enable_tests_mem_debug', 'Run tests with memory debugging'
                 ' (requires valgrind, disables assert tests)', False),
    BoolVariable('enable_player', 'Enable kunquat-player.', True),
    BoolVariable('enable_tracker', 'Enable kunquat-tracker.', True),
    BoolVariable('enable_export', 'Enable kunquat-export (requires libsndfile).', False),
    BoolVariable('enable_examples', 'Build example Kunquat files.', True),
    BoolVariable('with_wavpack', 'Build WavPack support (recommended).', True),
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


env['prefix'] = os.path.abspath(os.path.expanduser(env['prefix']))

def InstallCreatePath(env, path, source):
    if 'install' in COMMAND_LINE_TARGETS and not os.path.exists(path):
        os.makedirs(path)
    return env.Install(path, source)

env.AddMethod(InstallCreatePath, "InstallCreatePath")


if env['enable_debug']:
    env.Append(CCFLAGS = ['-g'])
else:
    env.Append(CCFLAGS = ['-DNDEBUG'])

if env['enable_profiling']:
    env.Append(CCFLAGS = ['-pg'])
    env.Append(LINKFLAGS = ['-pg'])

if env['optimise'] > 0 and env['optimise'] <= 3:
    oflag = '-O%s' % env['optimise']
    env.Append(CCFLAGS = [oflag])
#    env.Append(CCFLAGS = '-funroll-loops')


python_cmd = None
if env['enable_python_bindings']:
    test_cmds = ['python', 'python2.7', 'python2']
    for cmd in test_cmds:
        try:
            output = subprocess.check_output(
                    [cmd, '--version'], stderr=subprocess.STDOUT)
        except OSError, subprocess.CalledProcessError:
            output = ''
        if output.startswith('Python 2.7'):
            python_cmd = cmd
            break
    else:
        print('Python bindings were requested but Python 2.7 could not be found.')
        Exit(1)


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

    if env['enable_kunquat_assert'] and env['enable_debug']:
        conf.env.Append(CCFLAGS = '-DENABLE_KUNQUAT_ASSERT')
        if conf.CheckHeader('execinfo.h', language='C'):
            conf.env.Append(CCFLAGS = '-DHAS_EXECINFO')
            conf.env.Append(CCFLAGS = '-rdynamic')
            if env['enable_profiling']:
                conf.env.Append(LINKFLAGS = '-rdynamic')
            else:
                conf.env.Append(SHLINKFLAGS = '-rdynamic')

    if env['with_wavpack']:
        conf.env.Append(CCFLAGS = '-Dushort=uint16_t')
        conf.env.Append(CCFLAGS = '-Duint=uint32_t')
        if conf.CheckLibWithHeader('wavpack', 'wavpack/wavpack.h', 'C'):
            conf.env.Append(CCFLAGS = '-DWITH_WAVPACK')
        else:
            conf_errors.append('WavPack support was requested'
                               ' but WavPack was not found.')
    else:
        print('Warning: WavPack support is disabled!'
              ' Sample support will be very minimal.')

    #if not conf.CheckLibWithHeader('archive', ['archive_entry.h', 'archive.h'], 'C'):
    #    conf_errors.append('libarchive was not found.')

    if env['enable_player']:
        if not env['enable_python_bindings']:
            print('Warning: kunquat-player was requested'
                  ' without Python bindings -- disabling kunquat-player.')
            env['enable_player'] = False

    if env['enable_tracker']:
        if not env['enable_python_bindings']:
            print('Warning: kunquat-tracker was requested'
                  ' without Python bindings -- disabling kunquat-tracker.')
            env['enable_tracker'] = False

    if env['enable_export']:
        if not env['enable_python_bindings']:
            print('Warning: kunquat-export was requested'
                  ' without Python bindings -- disabling kunquat-export.')
            env['enable_export'] = False

    if env['enable_tests']:
        if not conf.CheckLibWithHeader('check', 'check.h', 'C'):
            conf_errors.append('Building of unit tests was requested'
                               ' but Check was not found.')
        if not conf.CheckLibWithHeader('pthread', 'pthread.h', 'C'):
            conf_errors.append('Building of unit tests requires libpthread.')
        if not conf.CheckLibWithHeader('rt', 'time.h', 'C'):
            conf_errors.append('Building of unit tests requires librt.')

    if env['enable_profiling']:
        if not conf.CheckLibWithHeader('m_p', 'math.h', 'C'):
            conf_errors.append('Profiling math library not found.')
    else:
        if not conf.CheckLibWithHeader('m', 'math.h', 'C'):
            conf_errors.append('Math library was not found.')

    if conf_errors:
        print('\nCouldn\'t configure Kunquat due to the following error%s:\n' %
              ('s' if len(conf_errors) != 1 else ''))
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
    SConscript('share/SConscript')
    if env['enable_python_bindings'] and 'install' in COMMAND_LINE_TARGETS:
        if 'install' in COMMAND_LINE_TARGETS:
            if not env.GetOption('clean'):
                args = [python_cmd,
                        'py-setup.py',
                        'install',
                        '--record=py-installed',
                        '--prefix=%s' % env['prefix']]
                if not env['enable_export']:
                    args.append('--disable-export')
                if not env['enable_player']:
                    args.append('--disable-player')
                if not env['enable_tracker']:
                    args.append('--disable-tracker')
                ret = subprocess.call(args)
                if ret:
                    Exit(ret)
            elif os.path.exists('py-installed'):
                f = open('py-installed')
                kunquat_dirs = []
                for path in f:
                    path = path.strip()
                    dir = os.path.dirname(path)
                    if os.path.basename(dir) == 'kunquat' or\
                       os.path.basename(os.path.dirname(dir)) == 'kunquat':
                        kunquat_dirs.append(dir)
                    os.remove(path)
                f.close()
                os.remove('py-installed')
                dir_set = set(kunquat_dirs)
                kunquat_dirs = list(dir_set)
                kunquat_dirs.sort()
                kunquat_dirs.reverse()
                for dir in kunquat_dirs:
                    try:
                        os.rmdir(dir)
                    except OSError:
                        print('Note: Directory %s is not empty,'
                              ' so it is not removed' % dir)
    if env.GetOption('clean'):
        if os.path.exists('build'):
            shutil.rmtree('build')
        if GetOption('full-clean') != None and os.path.exists('py-installed'):
            os.remove('py-installed')


