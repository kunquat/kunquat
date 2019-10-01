# -*- coding: utf-8 -*-


from distutils.core import setup
import os
import os.path
import sys


req_version_major = 3
req_version_minor = 5

if (sys.version_info[0], sys.version_info[1]) < (req_version_major, req_version_minor):
    print('Error: Kunquat Python modules require Python {}.{} or later.'.format(
            req_version_major, req_version_minor),
            file=sys.stderr)
    sys.exit(1)


inc_pkgs = ['kunquat', 'kunquat.kunquat', 'kunquat.extras']
inc_scripts = []
inc_data = []
req_list = []

if '--disable-export' not in sys.argv:
    inc_scripts.append('export/kunquat-export')
    inc_data.append(('share/man/man1', ['export/kunquat-export.1']))
else:
    sys.argv.remove('--disable-export')

if '--disable-player' not in sys.argv:
    inc_scripts.append('player/kunquat-player')
    inc_data.append(('share/man/man1', ['player/kunquat-player.1']),
                    #('/etc/bash_completion.d',
                    #    ['player/kunquat-player-completion']),
                   )
else:
    sys.argv.remove('--disable-player')

if '--disable-tracker' not in sys.argv:
    req_list.append('PyQt5')

    mod_dirs = (dirpath for (dirpath, _, _) in
            os.walk(os.path.join('kunquat', 'tracker')))
    def get_mod_name(d):
        head, tail = os.path.split(d)
        return '.'.join((get_mod_name(head), tail)) if head else tail
    mod_names = (get_mod_name(d) for d in mod_dirs)

    inc_pkgs.extend(list(mod_names))
    inc_scripts.append('tracker/kunquat-tracker')
else:
    sys.argv.remove('--disable-tracker')

setup(name='kunquat',
      version='0.9.4',
      author='Tomi Jylhä-Ollila',
      author_email='tomi.jylha-ollila@iki.fi',
      url='http://kunquat.org/',
      description='A music sequencer.',
      license='CC0',
      requires=req_list,
      packages=inc_pkgs,
      scripts=inc_scripts,
      data_files=inc_data,
     )


