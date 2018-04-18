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

from collections import defaultdict
import glob
import os
import os.path
import stat

from . import command


def build_lib(
        builder, lib_name, version_major, include_dirs, src_dir, out_dir, options, cc):

    def compile_lib_dir(out_dir, src_dir, echo_prefix_list):
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
            compile_lib_dir(sub_out_dir, sub_src_dir, echo_prefix_list)

    def compile_lib(out_dir, src_dir):
        echo_prefix = '\n   Compiling {}\n' \
            '   Using {} flags: {}\n\n'.format(
                    lib_name, cc.get_name(), cc.get_compile_flags())
        compile_lib_dir(out_dir, src_dir, [echo_prefix])

    def link_lib(build_lib_dir):
        objs = []
        for (dirpath, _, filenames) in os.walk(build_lib_dir):
            objs.extend(os.path.join(dirpath, name)
                    for name in filenames if name.endswith('.o'))

        lib_so_name = '{}.so'.format(lib_name)
        lib_path = os.path.join(build_lib_dir, lib_so_name)

        version_major = 0
        soname_flag = '-Wl,-soname,{}.{}'.format(lib_so_name, version_major)

        echo = '\n   Linking {}'.format(lib_name)
        cc.link_lib(builder, objs, lib_path, version_major, echo=echo)
        os.chmod(lib_path, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)

        # Add symlink so that our tests will run
        symlink_path = lib_path + '.{}'.format(version_major)
        command.link(builder, lib_so_name, symlink_path, echo='')

    cc.set_pic(True)

    for d in include_dirs:
        cc.add_include_dir(d)

    compile_lib(out_dir, src_dir)
    link_lib(out_dir)


