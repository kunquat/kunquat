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

import os
import sys
import subprocess

class ProcessManager():

    def __init__(self):
        self._children = []

    def set_controller(self, controller):
        pass

    def new_kunquat(self, module_path=None):
        kunquat_executable = sys.argv[0]
        executable_path = os.path.abspath(kunquat_executable)
        command = [executable_path]
        if module_path != None:
            command.append(module_path)
        handle = subprocess.Popen(command)
        self._children.append(handle)

