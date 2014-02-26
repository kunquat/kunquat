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

from notation import Notation


class NotationManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._share = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()
        self._updater = controller.get_updater()

    def get_notation(self):
        name = self._session.get_notation_name()
        notations = self._share.get_notations()
        return Notation(notations[name])


