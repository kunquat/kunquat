# -*- coding: utf-8 -*-


from distutils.core import setup
import sys


inc_pkgs = ['kunquat', 'kunquat.extras', 'kunquat.storage']
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
    req_list.append('PyQt4')
    inc_pkgs[len(inc_pkgs):len(inc_pkgs)] = [
            'kunquat.tracker',
            'kunquat.tracker.effects',
            'kunquat.tracker.env',
            'kunquat.tracker.instruments',
            'kunquat.tracker.sheet',
            ]
    inc_scripts.append('tracker/kunquat-tracker')
else:
    sys.argv.remove('--disable-tracker')

setup(name='kunquat',
      version='0.5.4',
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


