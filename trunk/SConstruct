

from options import opts


compile_flags = [
'-std=c99',
'-Wall',
'-Wextra',
'-Werror',
'-O2',
]

if opts['debug']:
	compile_flags.append('-g')
else:
	compile_flags.append('-DNDEBUG')


env = Environment(CCFLAGS = compile_flags)


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

	if not conf.CheckLibWithHeader('jack', 'jack/jack.h', 'C'):
		print 'JACK not found.'
		Exit(1)
	
	if opts['unit_tests'] and not conf.CheckLibWithHeader('check', 'check.h', 'C'):
		print 'Building of unit tests requires Check.'
		Exit(1)
		
	env = conf.Finish()


#print 'Root: ' + env.Dump('LIBS')


Export('env', 'opts')

SConscript('src/SConscript')


