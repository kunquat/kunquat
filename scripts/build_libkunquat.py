# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
from collections import defaultdict
import glob
import os
import os.path
import stat

import command


def build_libkunquat(builder, options, cc, compile_flags, link_flags):
    build_dir = os.path.join('build', 'src')
    command.make_dirs(builder, build_dir)
    out_dir = os.path.join(build_dir, 'lib')

    def compile_libkunquat_dir(compile_flags, out_dir, src_dir):
        builder.run('mkdir', '-p', out_dir)

        source_paths = glob.glob(os.path.join(src_dir, '*.c'))
        sources = sorted([os.path.basename(path) for path in source_paths])

        for source in sources:
            src_path = os.path.join(src_dir, source)
            obj_name = source[:source.rindex('.')] + '.o'
            out_path = os.path.join(out_dir, obj_name)
            print('Compiling {}'.format(src_path))
            builder.run(cc, '-c', src_path, '-o', out_path, compile_flags)

        # Recurse to subdirectories, excluding test directories
        subdir_names = sorted([name for name in os.listdir(src_dir)
                if os.path.isdir(os.path.join(src_dir, name)) and name != 'test'])
        for name in subdir_names:
            sub_out_dir = os.path.join(out_dir, name)
            sub_src_dir = os.path.join(src_dir, name)
            compile_libkunquat_dir(compile_flags, sub_out_dir, sub_src_dir)

    def link_libkunquat(link_flags, build_lib_dir):
        objs = []
        for (dirpath, _, filenames) in os.walk(build_lib_dir):
            objs.extend(os.path.join(dirpath, name)
                    for name in filenames if name.endswith('.o'))

        lib_name = 'libkunquat.so'
        lib_path = os.path.join(build_lib_dir, lib_name)

        version_major = 0
        soname_flag = '-Wl,-soname,{}.{}'.format(lib_name, version_major)
        libkunquat_link_flags = link_flags + ['-shared', soname_flag]

        print('Linking libkunquat')
        builder.run(cc, '-o', lib_path, objs, libkunquat_link_flags)
        os.chmod(lib_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)

        # Add symlink so that our tests will run
        symlink_path = lib_path + '.{}'.format(version_major)
        builder.run('ln', '-s', lib_name, symlink_path)

    shared_flags = ['-fPIC']

    # TODO: clean up code so that subdirectories inside src/lib are not needed
    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'lib', 'events'),
            os.path.join('src', 'lib', 'generators'),
            os.path.join('src', 'lib', 'dsps'),
            os.path.join('src', 'include')
        ]
    include_flags = ['-I' + d for d in include_dirs]

    libkunquat_compile_flags = compile_flags + shared_flags + include_flags

    src_dir = os.path.join('src', 'lib')
    compile_libkunquat_dir(libkunquat_compile_flags, out_dir, src_dir)
    link_libkunquat(link_flags, out_dir)


