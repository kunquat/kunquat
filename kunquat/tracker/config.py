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

import json
import os
import os.path
import sys


class _Entry():

    def __init__(self, validator, value):
        assert validator(value)
        self._value = value
        self._validator = validator

    def __repr__(self):
        return repr(self.get())

    def accepts(self, value):
        return self._validator(value)

    def get_modified(self, value):
        return _Entry(self._validator, value)

    def set(self, value):
        if not self.accepts(value):
            raise ValueError('Invalid value for config entry')
        self._value = value

    def get(self):
        return self._value


class _ConfigEncoder(json.JSONEncoder):

    def default(self, obj):
        assert isinstance(obj, _Entry)
        return obj.get()


class Config():

    _VERSION = 1
    _MAX_CHARS = 1048576

    def __init__(self):
        self._config_dir = os.path.abspath(
                os.path.expanduser(os.path.join('~', '.kunquat', 'tracker')))
        self._config_path = os.path.join(self._config_dir, 'config.json')
        self._modified = False

        def v_version(x):
            return isinstance(x, int) and 1 <= x < 1000

        def v_dir(x):
            return (x == None) or (isinstance(x, str) and len(x) < 1024)

        self._config = {
            'version'    : _Entry(v_version, self._VERSION),
            'dir_modules': _Entry(v_dir, None),
        }

    def get_value(self, key):
        return self._config[key].get()

    def set_value(self, key, value):
        if value != self._config[key].get():
            self._modified = True
            self._config[key].set(value)

    def _get_validated_config(self, unsafe_data):
        safe_data = {}

        # Check that we have a JSON dictionary
        try:
            decoded = json.loads(unsafe_data)
        except ValueError as e:
            print('Configuration is not valid JSON data: {}'.format(str(e)),
                    file=sys.stderr)
            return safe_data

        if type(decoded) != dict:
            print('Configuration is not a JSON dictionary', file=sys.stderr)
            return safe_data

        # Get configuration data
        stored_version = decoded.get('version', self._VERSION)
        if not self._config['version'].accepts(stored_version):
            stored_version = self._VERSION

        for k, v in decoded.items():
            if k == 'version':
                continue
            if k in self._config and self._config[k].accepts(v):
                safe_data[k] = self._config[k].get_modified(v)

        return safe_data

    def _load_config_data(self, f):
        chunks = []
        chars_read = 0
        while chars_read <= self._MAX_CHARS:
            chunk = f.read(4096)
            if not chunk:
                break
            chunks.append(chunk)
            chars_read += len(chunk)
        return chunks

    def try_load(self):
        chunks = []

        # Read file
        try:
            with open(self._config_path) as f:
                chunks = self._load_config_data(f)
        except FileNotFoundError:
            old_path = self._config_path + '.old'
            try:
                with open(old_path) as f:
                    chunks = self._load_config_data(f)
            except FileNotFoundError:
                # Nothing to do, using default settings
                return
            except OSError as e:
                # TODO: Should we report this failure?
                return
        except OSError as e:
            print('Could not open tracker configuration at {}: {}'.format(
                self._config_path, e.strerror),
                file=sys.stderr)
            return

        # Safety check
        chars_read = sum(len(c) for c in chunks)
        if chars_read > self._MAX_CHARS:
            print('Tracker configuration at {} is too large, ignoring'.format(
                self._config_path),
                file=sys.stderr)
            return

        # Apply valid settings
        config_data = self._get_validated_config(''.join(chunks))
        self._config.update(config_data)

    def try_save(self):
        if not self._modified:
            return

        # Make sure that our destination directory exists
        if not os.path.exists(self._config_dir):
            try:
                os.makedirs(self._config_dir)
            except OSError as e:
                print('Could not create directory for tracker configuration'
                        ' at {}: {}'.format(self._config_dir, e.strerror),
                        file=sys.stderr)
                return

        if not os.path.isdir(self._config_dir):
            print('Could not create directory for tracker configuration at {}: path'
                    ' already exists but is not a directory'.format(self._config_dir),
                    file=sys.stderr)
            return

        new_path = self._config_path + '.new'
        old_path = self._config_path + '.old'

        # Write new configuration file
        out_data = json.dumps(self._config, indent=4, cls=_ConfigEncoder, sort_keys=True)
        assert len(out_data) <= self._MAX_CHARS
        try:
            with open(new_path, 'w') as f:
                f.write(out_data)
        except OSError as e:
            print('Could not write new configuration file: {}'.format(e.strerror),
                    file=sys.stderr)
            return

        # Replace old configuration with the new one
        try:
            if os.path.exists(self._config_path):
                os.replace(self._config_path, old_path)
            os.replace(new_path, self._config_path)
            if os.path.exists(old_path):
                os.remove(old_path)
        except OSError as e:
            print('An error occurred while replacing old configuration file: {}'.format(
                e.strerror,
                file=sys.stderr))
            return


_config = Config()


def get_config():
    return _config


def try_load():
    _config.try_load()


def try_save():
    _config.try_save()


