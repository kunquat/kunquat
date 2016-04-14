# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict
import glob
import os
import os.path
import stat

from . import command


def build_libkunquat(builder, options, cc):
    build_dir = os.path.join('build', 'src')
    out_dir = os.path.join(build_dir, 'lib')

    def compile_libkunquat_dir(out_dir, src_dir, echo_prefix_list):
        source_paths = glob.glob(os.path.join(src_dir, '*.c'))
        sources = sorted([os.path.basename(path) for path in source_paths])

        for source in sources:
            src_path = os.path.join(src_dir, source)
            obj_name = source[:source.rindex('.')] + '.o'
            out_path = os.path.join(out_dir, obj_name)
            echo = echo_prefix_list[0] + 'Compiling {}'.format(src_path)
            if cc.compile(builder, src_path, out_path, echo=echo):
                echo_prefix_list[0] = ''

        # Recurse to subdirectories
        subdir_names = sorted([name for name in os.listdir(src_dir)
                if os.path.isdir(os.path.join(src_dir, name))])
        for name in subdir_names:
            sub_out_dir = os.path.join(out_dir, name)
            sub_src_dir = os.path.join(src_dir, name)
            compile_libkunquat_dir(sub_out_dir, sub_src_dir, echo_prefix_list)

    def compile_libkunquat(out_dir, src_dir):
        echo_prefix = '\n   Compiling libkunquat\n' \
            '   Using {} flags: {}\n\n'.format(cc.get_name(), cc.get_compile_flags())
        compile_libkunquat_dir(out_dir, src_dir, [echo_prefix])

    def link_libkunquat(build_lib_dir):
        objs = []
        for (dirpath, _, filenames) in os.walk(build_lib_dir):
            objs.extend(os.path.join(dirpath, name)
                    for name in filenames if name.endswith('.o'))

        lib_name = 'libkunquat.so'
        lib_path = os.path.join(build_lib_dir, lib_name)

        version_major = 0
        soname_flag = '-Wl,-soname,{}.{}'.format(lib_name, version_major)

        echo = '\n   Linking libkunquat'
        cc.link_lib(builder, objs, lib_path, version_major, echo=echo)
        os.chmod(lib_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)

        # Add symlink so that our tests will run
        symlink_path = lib_path + '.{}'.format(version_major)
        command.link(builder, lib_name, symlink_path, echo='')

    cc.set_pic(True)

    include_dirs = [
            os.path.join('src', 'lib'),
            os.path.join('src', 'include')
        ]
    for d in include_dirs:
        cc.add_include_dir(d)

    src_dir = os.path.join('src', 'lib')
    compile_libkunquat(out_dir, src_dir)
    link_libkunquat(out_dir)


