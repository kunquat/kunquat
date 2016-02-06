# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


def get_proc_params(ui_model, au_id, proc_id):
    module = ui_model.get_module()
    au = module.get_audio_unit(au_id)
    proc = au.get_processor(proc_id)
    proc_params = proc.get_type_params()
    return proc_params


