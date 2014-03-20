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
import glob
import os.path

import command


def install_libkunquat(builder, install_prefix):
    def _install_headers():
        install_include_dir = os.path.join(install_prefix, 'include', 'kunquat')
        command.make_dirs(builder, install_include_dir)

        source_dir = os.path.join('src', 'include', 'kunquat')
        header_paths = glob.glob(os.path.join(source_dir, '*.h'))
        header_names = sorted([os.path.basename(path) for path in header_paths])
        for name in header_names:
            in_path = os.path.join(source_dir, name)
            out_path = os.path.join(install_include_dir, name)
            command.copy(builder, in_path, out_path)

    def _install_man_pages():
        install_man_dir = os.path.join(install_prefix, 'share', 'man', 'man3')
        command.make_dirs(builder, install_man_dir)

        source_dir = os.path.join('src', 'include', 'kunquat')
        man_paths = glob.glob(os.path.join(source_dir, 'kunquat-*.3'))
        man_names = sorted([os.path.basename(path) for path in man_paths])
        for name in man_names:
            in_path = os.path.join(source_dir, name)
            out_path = os.path.join(install_man_dir, name)
            command.copy(builder, in_path, out_path)

        links = {
                'kunquat-handle.3': [
                    'kqt_new_Handle.3',
                    'kqt_Handle_set_data.3',
                    'kqt_Handle_get_error.3',
                    'kqt_Handle_clear_error.3',
                    'kqt_Handle_validate.3',
                    'kqt_del_Handle.3'
                    ],

                'kunquat-player-interface.3': [
                    'kqt_Handle_play.3',
                    'kqt_Handle_has_stopped.3',
                    'kqt_Handle_get_frames_available.3',
                    'kqt_Handle_get_audio.3',
                    'kqt_Handle_set_audio_rate.3',
                    'kqt_Handle_get_audio_rate.3',
                    'kqt_Handle_set_audio_buffer_size.3',
                    'kqt_Handle_get_audio_buffer_size.3',
                    'kqt_Handle_get_duration.3',
                    'kqt_Handle_set_position.3',
                    'kqt_Handle_get_position.3',
                    'kqt_Handle_fire_event.3',
                    'kqt_Handle_receive_events.3'
                    ],
            }

        for name in man_names:
            man_path = os.path.join(install_man_dir, name)
            if name in links:
                for link in links[name]:
                    link_path = os.path.join(install_man_dir, link)
                    command.link(builder, name, link_path)

    def _install_library():
        install_lib_dir = os.path.join(install_prefix, 'lib')
        command.make_dirs(builder, install_lib_dir)

        build_dir = os.path.join('build', 'src', 'lib')
        lib_names = ['libkunquat.so', 'libkunquat.so.0']
        for name in lib_names:
            in_path = os.path.join(build_dir, name)
            out_path = os.path.join(install_lib_dir, name)
            command.copy(builder, in_path, out_path)

    _install_headers()
    _install_man_pages()
    _install_library()


