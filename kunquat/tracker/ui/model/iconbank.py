# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class IconBank():

    def __init__(self):
        self._share = None

    def set_controller(self, controller):
        self._share = controller.get_share()

    def get_kunquat_logo_path(self):
        return self._share.get_kunquat_logo_path()

