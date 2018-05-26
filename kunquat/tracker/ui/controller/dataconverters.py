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

import re

from kunquat.tracker.version import KUNQUAT_VERSION


class VersionError(ValueError):
    pass


class UnsupportedVersionError(VersionError):

    def get_message(self, data_kind, version_entry):
        ver_string = None
        if (type(version_entry) == list) and (len(version_entry) == 2):
            _, maybe_ver_string = version_entry
            if type(maybe_ver_string) == str:
                ver_string = maybe_ver_string

        msg = 'Could not load {} data due to version mismatch.'.format(data_kind)
        if not (KUNQUAT_VERSION and ver_string):
            msg += '\nThe data is probably created with a later version of Kunquat.'
        else:
            msg += '\nThe data is created with Kunquat version {}'.format(ver_string)
            if ver_string > KUNQUAT_VERSION:
                msg += ' (current version of Kunquat installed is {}).'.format(
                        KUNQUAT_VERSION)
            else:
                msg += (', which should be compatible with the version'
                    ' of Kunquat installed ({}).'
                    ' Please report this issue.'.format(KUNQUAT_VERSION))

        msg += '\n' + self.args[0]

        return msg


class ConversionInfo():

    def __init__(self, converters=[]):
        self._re = None
        self._converters = converters

    def set_key_pattern(self, key_pattern):
        assert key_pattern.endswith('.json')
        self._re = re.compile(key_pattern)

    def can_convert(self, key):
        return bool(self._re.match(key))

    def get_last_version(self):
        return len(self._converters)

    def convert_from_version(self, orig_key, orig_ver_data):
        if (orig_ver_data == None) or not orig_key.endswith('.json'):
            return orig_key, orig_ver_data

        if not (isinstance(orig_ver_data, list) and len(orig_ver_data) == 2):
            raise VersionError(
                    'Data to be converted is not valid versioned data, key "{}"'.format(
                        orig_key))

        orig_version, orig_payload = orig_ver_data
        if not (isinstance(orig_version, int) and orig_version >= 0):
            raise VersionError(
                    'Invalid data version number of key "{}"'.format(orig_key))

        if orig_version > self.get_last_version():
            raise UnsupportedVersionError(
                    'Unsupported data version of key "{}": {}'.format(
                        orig_key, orig_version))

        if orig_version == self.get_last_version():
            return (orig_key, orig_payload)

        conv = self._converters[orig_version]
        return (conv.convert_key(orig_key), conv.convert_data(orig_payload))


class Converter():

    def __init__(self):
        pass

    # Protected interface

    def convert_key(self, orig_key):
        return orig_key

    def convert_data(self, orig_data):
        return orig_data


class DataConverters():

    def __init__(self):
        self._conversion_infos = []
        self._default_conv = ConversionInfo()

    def add_conversion_info(self, info):
        self._conversion_infos.append(info)

    def convert_key_and_data(self, key, orig_ver_data):
        for convinfo in self._conversion_infos:
            if convinfo.can_convert(key):
                return convinfo.convert_from_version(key, orig_ver_data)

        return self._default_conv.convert_from_version(key, orig_ver_data)

    def wrap_with_latest_version(self, key, data):
        if (data == None) or not key.endswith('.json'):
            return data

        for convinfo in self._conversion_infos:
            if convinfo.can_convert(key):
                return [convinfo.get_last_version(), data]

        return [0, data]

    def strip_version(self, key, data):
        if (data == None) or not key.endswith('.json'):
            return data

        if not (isinstance(data, list) and len(data) == 2):
            raise VersionError('Data is not versioned')

        _, stripped = data
        return stripped


