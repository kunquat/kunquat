# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import os.path

from .build_lib import build_lib


def build_libkunquat(builder, options, cc):
    lib_name = 'libkunquat'
    version_major = 0
    include_dirs = [
        os.path.join('src', 'lib'),
        os.path.join('src', 'include'),
    ]
    src_dir = os.path.join('src', 'lib')
    out_dir = os.path.join('build', 'src', 'lib')

    build_lib(
            builder,
            lib_name,
            version_major,
            include_dirs,
            src_dir,
            out_dir,
            options,
            cc)


def build_libkunquatfile(builder, options, cc):
    lib_name = 'libkunquatfile'
    version_major = 0
    include_dirs = [
        os.path.join('src', 'file', 'lib'),
        os.path.join('src', 'file', 'include'),
        os.path.join('src', 'include'),
    ]
    src_dir = os.path.join('src', 'file', 'lib')
    out_dir = os.path.join('build', 'src', 'file', 'lib')

    cc.add_lib_dir(os.path.join('build', 'src', 'lib'))

    build_lib(
            builder,
            lib_name,
            version_major,
            include_dirs,
            src_dir,
            out_dir,
            options,
            cc)


