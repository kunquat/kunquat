# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.keymapmanager import KeyboardAction
from .updater import Updater
from .utils import get_scaled_font


class TypewriterButton(QPushButton, Updater):

    def __init__(self, row, index):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._control_mgr = None
        self._typewriter_mgr = None

        self._row = row
        self._index = index
        self._key_name = None
        self._selected_control = None

        self.setFixedSize(QSize(60, 60))

        self.setFocusPolicy(Qt.NoFocus)

        self._char_images = {}
        self._name_images = {}

        self._led_state = (False, False, False)

        self.setEnabled(False)
        self.pressed.connect(self._press)
        self.released.connect(self._release)

    def _on_setup(self):
        self._control_mgr = self._ui_model.get_control_manager()
        self._typewriter_mgr = self._ui_model.get_typewriter_manager()

        self._button_model = self._typewriter_mgr.get_button_model(
                self._row, self._index)

        self.register_action('signal_octave', self._update_properties)
        self.register_action('signal_notation', self._update_properties)
        self.register_action('signal_selection', self._update_key_checked_properties)
        self.register_action('signal_hits', self._update_key_checked_properties)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_properties()

    def _clear_caches(self):
        self._char_images = {}
        self._name_images = {}

    def update_led_state(self, led_state):
        if self._led_state != led_state:
            self._led_state = led_state
            self.update()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        size = style_mgr.get_scaled_size_param('typewriter_button_size')
        self.setFixedSize(QSize(size, size))

        size = self.width(), self.height()
        self._button_model.flush_images(size)
        self._clear_caches()

        self.update()

    def _update_properties(self):
        name = self._button_model.get_name()
        if name != None:
            #self._notename.setText(name)

            keymap_mgr = self._ui_model.get_keymap_manager()
            keymap_row_index = (
                    keymap_mgr.get_typewriter_row_offsets()[self._row] + self._index)
            self._key_name = keymap_mgr.get_key_name((self._row, keymap_row_index))
            if self._key_name:
                play_type = 'hit' if keymap_mgr.is_hit_keymap_active() else 'note'
                self.setToolTip(
                        'Play {} {} ({})'.format(play_type, name, self._key_name))
            else:
                self.setToolTip('')

            self.setEnabled(True)
        else:
            #self._notename.setText('')
            self.setToolTip('')
            self.setEnabled(False)

        self._clear_caches()
        self.update()

    def _update_key_checked_properties(self):
        keymap_mgr = self._ui_model.get_keymap_manager()
        if keymap_mgr.is_hit_keymap_active():
            self._update_properties()

    def _press(self):
        selection = self._ui_model.get_selection()
        location = selection.get_location()
        sheet_mgr = self._ui_model.get_sheet_manager()
        control_id = sheet_mgr.get_inferred_active_control_id_at_location(location)

        self._control_mgr.set_control_id_override(control_id)
        self._button_model.start_tracked_note()
        self._control_mgr.set_control_id_override(None)

    def _release(self):
        self._button_model.stop_tracked_note()

    def _get_led_thickness(self):
        return 0.4

    def _get_button_background(self, enabled, pressed):
        size = self.width(), self.height()
        pixmap = self._button_model.get_background_image(size, enabled, pressed)

        if not pixmap:
            pixmap = QPixmap(*size)

            style_mgr = self._ui_model.get_style_manager()

            border_contrast = style_mgr.get_style_param('border_contrast')
            button_brightness = style_mgr.get_style_param(
                    'button_press_brightness' if pressed else 'button_brightness')

            def get_adjusted_colour(name, adjust):
                return QColor(style_mgr.get_adjusted_colour(name, adjust))

            bg_colour = QColor(style_mgr.get_style_param('bg_colour'))

            button_bg_colour = get_adjusted_colour('bg_colour', button_brightness)
            button_edge_light = get_adjusted_colour(
                'bg_colour', button_brightness + border_contrast)
            button_edge_dark = get_adjusted_colour(
                'bg_colour', button_brightness - border_contrast)

            edge_width = style_mgr.get_scaled_size_param('border_thin_width')
            edge_radius = style_mgr.get_scaled_size_param('border_thin_radius')
            diam = edge_radius * 2

            edge_tiny_radius = edge_radius / 3
            tiny_diam = edge_tiny_radius * 2

            if enabled:
                led_colour = QColor(
                        style_mgr.get_scaled_colour('active_indicator_colour', 0.3))
            else:
                led_colour = QColor(style_mgr.get_style_param('bg_sunken_colour'))

            led_thickness = style_mgr.get_scaled_size(self._get_led_thickness())

            margin = edge_width / 2
            led_top = margin + 0.5
            top = led_top + led_thickness
            left = margin + 0.5
            right = self.width() - margin + 0.5
            bottom = self.height() - margin + 0.5

            pixmap.fill(bg_colour)

            painter = QPainter(pixmap)
            painter.setRenderHint(QPainter.Antialiasing)

            # Background fill
            painter.setPen(Qt.NoPen)
            bg_rect = QRectF(left, top, right - 1, bottom - 1).adjusted(
                    margin, margin, -margin, -margin - led_thickness)
            painter.fillRect(bg_rect, button_bg_colour)

            # Neutral LED
            painter.save()
            painter.setPen(Qt.NoPen)
            painter.setBrush(QBrush(led_colour))
            led_path = QPainterPath()
            led_path.moveTo(QPointF(left, top))
            led_path.lineTo(QPointF(left, led_top + edge_radius))
            led_path.arcTo(QRectF(left, led_top, diam, diam), 180, -90)
            led_path.lineTo(QPointF(right - edge_radius, led_top))
            led_path.arcTo(QRectF(right - diam, led_top, diam, diam), 90, -90)
            led_path.lineTo(QPointF(right, top))
            led_path.lineTo(QPointF(left, top))
            painter.drawPath(led_path)
            painter.restore()

            # Dark shades
            painter.setPen(QPen(QBrush(button_edge_dark), edge_width, cap=Qt.FlatCap))
            dark_path = QPainterPath()
            dark_path.arcMoveTo(QRectF(left, bottom - diam, diam, diam), 225)
            dark_path.arcTo(QRectF(left, bottom - diam, diam, diam), 225, 45)
            dark_path.lineTo(QPointF(right - edge_radius, bottom))
            dark_path.arcTo(QRectF(right - diam, bottom - diam, diam, diam), 270, 90)
            dark_path.lineTo(QPointF(right, top + edge_tiny_radius))
            dark_path.arcTo(QRectF(right - tiny_diam, top, tiny_diam, tiny_diam), 0, 45)
            painter.drawPath(dark_path)

            # Light shades
            painter.setPen(QPen(QBrush(button_edge_light), edge_width, cap=Qt.FlatCap))
            light_path = QPainterPath()
            light_path.arcMoveTo(QRectF(left, bottom - diam, diam, diam), 225)
            light_path.arcTo(QRectF(left, bottom - diam, diam, diam), 225, -45)
            light_path.lineTo(QPointF(left, top + edge_tiny_radius))
            light_path.arcTo(QRectF(left, top, tiny_diam, tiny_diam), 180, -90)
            light_path.lineTo(QPointF(right - edge_tiny_radius, top))
            light_path.arcTo(
                    QRectF(right - tiny_diam, top, tiny_diam, tiny_diam), 90, -45)
            painter.drawPath(light_path)

            painter.end()

            self._button_model.set_background_image(size, enabled, pressed, pixmap)

        return pixmap

    def _get_character_image(self, pressed):
        if not self._key_name:
            return None

        if pressed not in self._char_images:
            style_mgr = self._ui_model.get_style_manager()
            font = get_scaled_font(style_mgr, 1.3)

            fm = QFontMetrics(font, self)
            char_rect = fm.boundingRect(self._key_name).adjusted(0, 0, 2, 2)
            baseline_offset = fm.tightBoundingRect('Á').height() + 1

            image_height = max(char_rect.height(), baseline_offset)

            char_image = QImage(
                    char_rect.width(), image_height, QImage.Format_ARGB32_Premultiplied)
            char_image.fill(0)

            painter = QPainter(char_image)
            painter.setCompositionMode(QPainter.CompositionMode_Plus)
            painter.setFont(font)

            button_brightness = style_mgr.get_style_param(
                    'button_press_brightness' if pressed else 'button_brightness')
            text_colour = QColor(style_mgr.get_adjusted_colour(
                    'disabled_fg_colour', button_brightness))
            painter.setPen(text_colour)
            painter.drawText(char_image.rect(), Qt.AlignCenter, self._key_name)

            painter.end()

            self._char_images[pressed] = char_image

        return self._char_images[pressed]

    def _get_name_image(self, pressed):
        name = self._button_model.get_name()
        if not name:
            return None

        if pressed not in self._name_images:
            style_mgr = self._ui_model.get_style_manager()
            font = get_scaled_font(style_mgr, 1)

            fm = QFontMetrics(font, self)
            name_rect = fm.boundingRect(name).adjusted(0, 0, 2, 2)

            name_image = QImage(name_rect.size(), QImage.Format_ARGB32_Premultiplied)
            name_image.fill(0)

            painter = QPainter(name_image)
            painter.setCompositionMode(QPainter.CompositionMode_Plus)
            painter.setFont(font)

            button_brightness = style_mgr.get_style_param(
                    'button_press_brightness' if pressed else 'button_brightness')
            if pressed:
                text_colour = QColor(style_mgr.get_adjusted_colour('fg_colour', button_brightness))
            else:
                text_colour = QColor(style_mgr.get_style_param('fg_colour'))
            painter.setPen(text_colour)
            painter.drawText(name_image.rect(), Qt.AlignCenter, name)

            painter.end()

            self._name_images[pressed] = name_image

        return self._name_images[pressed]

    def _get_led_image(self):
        if not any(self._led_state):
            return None

        size = self.width(), self.height()
        image = self._button_model.get_led_image(size, self._led_state)
        if not image:
            style_mgr = self._ui_model.get_style_manager()
            led_height = style_mgr.get_scaled_size(self._get_led_thickness())

            pre_height = self.height() / 2

            pre_image = QImage(
                    self.width(), pre_height, QImage.Format_ARGB32_Premultiplied)
            pre_image.fill(0)

            painter = QPainter(pre_image)
            painter.setCompositionMode(QPainter.CompositionMode_Plus)

            led_colour = QColor(style_mgr.get_style_param('active_indicator_colour'))

            half_width = self.width() / 2
            half_height = pre_height / 2
            detune_offset = 0.25

            def fill_with_gradient(gradient):
                gradient.setColorAt(0, led_colour)
                gradient.setColorAt(1, QColor(0, 0, 0, 0))
                brush = QBrush(gradient)
                painter.fillRect(QRectF(0, 0, self.width(), pre_height), brush)

            if self._led_state[0]:
                fill_with_gradient(QRadialGradient(
                        half_width, half_height, half_width,
                        self.width() * detune_offset, half_height))

            if self._led_state[1]:
                fill_with_gradient(QRadialGradient(
                        half_width, half_height, half_width,
                        half_width, half_height))

            if self._led_state[2]:
                fill_with_gradient(QRadialGradient(
                        half_width, half_height, half_width,
                        self.width() * (1 - detune_offset), half_height))

            painter.end()

            image = QImage(self.width(), led_height, QImage.Format_ARGB32_Premultiplied)
            image.fill(0)

            painter = QPainter(image)
            painter.setRenderHint(QPainter.SmoothPixmapTransform)
            painter.drawImage(QRectF(0, 0, self.width(), led_height), pre_image)
            painter.end()

            self._button_model.set_led_image(size, self._led_state, image)

        return image

    def paintEvent(self, event):
        painter = QPainter(self)

        enabled = (self._button_model.get_name() != None)
        pressed = self.isDown()

        pixmap = self._get_button_background(enabled, pressed)

        painter.drawPixmap(0, 0, pixmap)

        width, height = self.width(), self.height()

        char_image = self._get_character_image(pressed)
        if char_image:
            char_x = width * 0.1
            char_y = height * 0.1
            painter.drawImage(char_x, char_y, char_image)

        name_image = self._get_name_image(pressed)
        if name_image:
            name_x = (width - name_image.width()) // 2
            name_y = height * 0.54
            painter.drawImage(name_x, name_y, name_image)

        led_image = self._get_led_image()
        if led_image:
            painter.drawImage(0, 0, led_image)

        painter.end()


