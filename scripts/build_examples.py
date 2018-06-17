# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os
import os.path

from . import command


def build_examples(builder):
    build_dir = os.path.join('build', 'examples')
    command.make_dirs(builder, build_dir, echo='')

    example_dir = os.path.join('examples')

    files = [
        'Asturias.kqt',
    ]

    echo_prefix = '\n   Building example files\n\n'

    for path in files:
        src_path = os.path.join(example_dir, path)
        dest_path = os.path.join(build_dir, path)
        echo = echo_prefix + 'Copying {}'.format(path)
        if command.copy(builder, src_path, dest_path, echo):
            echo_prefix = ''

    dirs = {
        #'kqtc00': 'example.kqt',
        'kqti00': 'example_ins.kqti',
    }

    for src, dest in dirs.items():
        dest_path = os.path.join(build_dir, dest)
        echo = echo_prefix + 'Building {}'.format(dest)

        # Mega hack to ensure Fabricate tracks correct input files
        script_path = os.path.join('scripts', 'write_example_file.py')
        in_dir = os.path.join(example_dir, src)
        in_files = []
        for dir_path, _, fnames in os.walk(in_dir, src):
            for fname in fnames:
                fpath = os.path.join(dir_path, fname)
                in_files.append(fpath)

        if command.PythonCommand().run(
                builder, script_path, dest_path, in_dir, *in_files, echo=echo):
            echo_prefix = ''

        # The proper way to do it (probably) as soon as Fabricate supports it
        '''
        if command.run_command(
                builder,
                'tar',
                'cj', '--format=ustar',
                '-f', dest_path,
                '--directory', example_dir,
                src,
                echo=echo):
            echo_prefix = ''
        '''

    # Copy the example instrument to share
    # TODO: remove once we figure out the instrument stuff
    ins_name = 'example_ins.kqti'
    default_ins_path = os.path.join(build_dir, ins_name)
    share_target = os.path.join('share', 'kunquat', 'instruments', ins_name)
    command.copy(builder, default_ins_path, share_target, echo='')


