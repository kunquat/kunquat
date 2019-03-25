# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


_OVERLAY_KERNEL_LISTS = [
    [['11',
      '11']],

    [[' 1',
      '111',
      ' 1']],

    [['11',
      '11'],
     [' 1',
      '1 1',
      ' 1']],

    [['11',
      '11',
      '11'],
     [' 11',
      '1  1',
      ' 11']],

    [['11',
      '11'],
     ['  1',
      ' 1 1',
      '1   1',
      ' 1 1',
      '  1']],

    [[' 11',
      '1  1',
      '1  1',
      ' 11'],
     [' 11',
      '1  1',
      '1  1',
      ' 11']],
]


def _get_points(overlay_kernel):
    offset = -((len(overlay_kernel) - 1) // 2)
    for row, line in enumerate(overlay_kernel):
        for col, c in enumerate(line):
            if c != ' ':
                yield QPoint(col + offset, row + offset)


_OVERLAY_POINT_LISTS = []
for oks in _OVERLAY_KERNEL_LISTS:
    point_lists = []
    for ok in oks:
        point_lists.append(list(_get_points(ok)))
    _OVERLAY_POINT_LISTS.append(point_lists)


def embolden_path(src_image, thickness):
    if thickness <= 1:
        return src_image

    width, height = src_image.width(), src_image.height()

    max_thickness = len(_OVERLAY_POINT_LISTS) + 1
    point_lists = _OVERLAY_POINT_LISTS[min(max_thickness, thickness) - 2]

    for pl in point_lists:
        image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
        image.fill(0)
        painter = QPainter(image)
        for point in pl:
            painter.drawImage(point, src_image)
        painter.end()
        src_image = image

    return src_image


