# -*- coding: utf-8 -*-


from distutils.core import setup
import sys

import options


inc_scripts = []
inc_data = []
if options.enable_player:
    inc_scripts.append('player/kunquat-player')
    inc_data.append(('share/man/man1', 'player/kunquat-player.1'))

setup(name='kunquat',
      version='0.3.1',
      author='Tomi Jylhä-Ollila',
      author_email='tomi.jylha-ollila@iki.fi',
      url='http://launchpad.net/kunquat/',
      description='A music sequencer.',
      license='CC0',
      packages=['kunquat', 'kunquat.extra'],
      scripts=inc_scripts,
      data_files=inc_data,
     )


