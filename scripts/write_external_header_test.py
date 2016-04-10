#!/usr/bin/env python3
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

import sys


# Note: This file is run as a script. It works around the issue of Fabricate
#       not tracking files changed by shell commands properly.


def write_external_header_test(out_base, header_name):
    code = '#include <{}>\nint main(void) {{ return 0; }}\n'.format(header_name)
    out_path = out_base + '.c'
    with open(out_path, 'w') as out_file:
        out_file.write(code)


def main():
    if len(sys.argv) != 3:
        print('Usage: {} <out_base> <header_name>'.format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
    write_external_header_test(sys.argv[1], sys.argv[2])


if __name__ == '__main__':
    main()


