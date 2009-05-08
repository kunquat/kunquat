# coding=utf-8


# Copyright 2009 Tomi Jylhä-Ollila
#
# This file is part of Kunquat.
#
# Kunquat is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kunquat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.


import pygtk
pygtk.require('2.0')
import gtk
import gobject

import liblo

from Envelope import Envelope


class sample_map:

    def __init__(self, source, style, strengths):
        self.source = source
        self.style = style
        self.maps = [{} for _ in range(strengths)]
        self.strengths = []


class Instruments(gtk.HBox):

    def ins_info(self, path, args, types):
        iter = self.it_view.get_model().get_iter(args[0] - 1)
        self.ins_table.set_value(iter, 1, args[2])
        self.ins_params[args[0] - 1] = args[1:]
        if args[1] <= 2:
            self.ui_params[args[0] - 1] = None
        elif args[1] == 3:
            styles_start = args.index('__styles') + 1
            maps_start = args.index('__maps') + 1
            sample_descs = args[3:styles_start - 1]
            style_descs = args[styles_start:maps_start - 1]
            map_descs = args[maps_start:]
#           print(style_descs)
#           print(map_descs)
            if (not self.ui_params[args[0] - 1] or
                    self.ui_params[args[0] - 1].get_name() != 'pcm'):
                self.ui_params[args[0] - 1] = self.build_pcm_details(args[0])
            pcm_details = self.ui_params[args[0] - 1]
            samples = pcm_details.get_nth_page(2)
            sample_details = samples.get_children()[1]
            sample_path = sample_details.get_children()[0]
            sample_path_str = sample_path.get_children()[1]
            sample_freq = sample_details.get_children()[1]
            sample_freq_val = sample_freq.get_children()[1]

            sample_list_scroll = samples.get_children()[0]
            sample_list_view = sample_list_scroll.get_child()
            sample_selection = sample_list_view.get_selection()
            _, sr = sample_selection.get_selected_rows()
            if sr:
                pcm_details.sample_info['cur'] = sr[0][0]

            found_samples = set()
            for i in range(0, len(sample_descs), 3):
                found_samples.add(sample_descs[i])
                pcm_details.sample_info['samples'][sample_descs[i]] = sample_descs[i + 1:i + 3]
                if sample_descs[i] == pcm_details.sample_info['cur']:
                    path = sample_descs[i + 1]
                    if sample_path_str.get_text() != path:
                        sample_path_str.set_text(path)
                    if not sample_freq_val.user_set:
                        sample_freq_val.handler_block(sample_freq_val.shid)
                        sample_freq_val.set_value(sample_descs[i + 2])
                        sample_freq_val.handler_unblock(sample_freq_val.shid)
                    else:
                        sample_freq_val.user_set = False
            not_found = set(range(512)) - found_samples
            for i in not_found:
                if i in pcm_details.sample_info['samples']:
                    del pcm_details.sample_info['samples'][i]
                    if i == pcm_details.sample_info['cur']:
                        sample_path_str.set_text('')
                        if not sample_freq_val.user_set:
                            sample_freq_val.handler_block(sample_freq_val.shid)
                            sample_freq_val.set_value(44100)
                            sample_freq_val.handler_unblock(sample_freq_val.shid)
                        else:
                            sample_freq_val.user_set = False

            while map_descs:
                source = map_descs.pop(0)
                style = map_descs.pop(0)
                strengths = map_descs.pop(0)
                smap = sample_map(source, style, strengths)
                for m in smap.maps:
                    smap.strengths.append(map_descs.pop(0))
                    freq_levels = map_descs.pop(0)
                    for f in range(freq_levels):
                        min_freq = map_descs.pop(0)
                        rand_choices = map_descs.pop(0)
                        for c in range(rand_choices):
                            sample_num = map_descs.pop(0)
                            freq_scale = map_descs.pop(0)
                            vol_scale = map_descs.pop(0)
                            if (min_freq, c) not in m:
                                m[(min_freq, c)] = []
                            m[(min_freq, c)].append((sample_num, freq_scale, vol_scale))
                pcm_details.sample_info['maps'][(source, style)] = smap

            map_view = pcm_details.sample_info['map_view']
            map_model = map_view.get_model()
            iter = map_model.get_iter_first()
            map = pcm_details.sample_info['maps'][(0, 0)].maps[0]
            map_list = map.items()
            if map_list:
                map_list.sort()
            for (k, v) in map_list:
                freq, _ = k
                desc = ','.join([str(x) for x in v[0]])
                if not iter:
                    iter = map_model.append()
                map_model.set_value(iter, 0, freq)
                map_model.set_value(iter, 1, desc)
                iter = map_model.iter_next(iter)
            while iter:
                if not map_model.remove(iter):
                    iter = None
                    
        if self.cur_index == args[0]:
            self.select_ins(self.it_view.get_selection())

    def build_pcm_details(self, ins_num):
        pcm_details = gtk.Notebook()

        force = gtk.VBox()
        pcm_details.append_page(force, gtk.Label('Force'))

        volume = gtk.VBox()
        on_volume_env = Envelope(self.engine, self.server, self.song_id,
                (0, 1024, 0.001, True), (0, 1, 0.01, False))
        volume.pack_start(on_volume_env, False, False)
        pcm_details.append_page(volume, gtk.Label('Volume'))

        samples = gtk.HBox()

        sample_store = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_STRING)
        sample_list = gtk.TreeView(sample_store)
        sample_list.set_enable_search(False)
        for i in range(512):
            iter = sample_store.append()
            sample_store.set(iter, 0, '%03X' % i)
        cell = gtk.CellRendererText()
        column = gtk.TreeViewColumn('#', cell, text=0)
        sample_list.append_column(column)
        cell = gtk.CellRendererText()
        cell.set_property('editable', True)
#       cell.connect('edited', TODO)
        column = gtk.TreeViewColumn('Name', cell, text=1)
        sample_list.append_column(column)
        sample_scroll = gtk.ScrolledWindow()
        sample_scroll.add(sample_list)
        samples.pack_start(sample_scroll)

        sample_details = gtk.VBox()

        sample_path = gtk.HBox()
        sample_details.pack_start(sample_path, False, False)
        sample_path_label = gtk.Label('Path')
        sample_path.pack_start(sample_path_label, False, False)
        sample_path_str = gtk.Entry()
        sample_path.pack_start(sample_path_str)
        sample_path_browse = gtk.Button('Browse...')
        sample_path_browse.connect('clicked', self.browse_sample,
                ins_num, pcm_details)
        sample_path.pack_start(sample_path_browse, False, False)
        sample_path_remove = gtk.Button('Remove')
        sample_path_remove.connect('clicked', self.remove_sample,
                ins_num, pcm_details)
        sample_path.pack_start(sample_path_remove, False, False)

        sample_freq = gtk.HBox()
        sample_details.pack_start(sample_freq, False, False)
        sample_freq_label = gtk.Label('440 Hz frequency')
        sample_freq.pack_start(sample_freq_label, False, False)
        adj = gtk.Adjustment(44100, 1, 2147483647, 0.01, 1)
        sample_freq_val = gtk.SpinButton(adj, digits=2)
        shid = sample_freq_val.connect('value-changed', self.change_mid_freq,
                ins_num, pcm_details)
        sample_freq_val.shid = shid
        sample_freq_val.user_set = False
        sample_freq.pack_start(sample_freq_val, False, False)

        samples.pack_start(sample_details)

        pcm_details.append_page(samples, gtk.Label('Samples'))
        pcm_details.sample_info = {}
        pcm_details.sample_info['path'] = sample_path_str
        pcm_details.sample_info['freq'] = sample_freq_val
        pcm_details.sample_info['cur'] = -1
        pcm_details.sample_info['samples'] = {}

        mappings = gtk.VBox()
        map_store = gtk.ListStore(gobject.TYPE_DOUBLE, gobject.TYPE_STRING)
        map_view = gtk.TreeView(map_store)
        map_view.set_enable_search(False)
        cell = gtk.CellRendererText()
        cell.set_property('editable', True)
        cell.connect('edited', self.pcm_set_mapping, ins_num, pcm_details, 0)
        column = gtk.TreeViewColumn('Freq', cell, text=0)
        map_view.append_column(column)
        cell = gtk.CellRendererText()
        cell.set_property('editable', True)
        cell.connect('edited', self.pcm_set_mapping, ins_num, pcm_details, 1)
        column = gtk.TreeViewColumn('Sample(s)', cell, text=1)
        map_view.append_column(column)
        map_scroll = gtk.ScrolledWindow()
        map_scroll.add(map_view)
        mappings.pack_start(map_scroll)

        map_context = gtk.Menu()
        map_insert = gtk.MenuItem('Insert')
        map_remove = gtk.MenuItem('Remove')
        map_context.append(map_insert)
        map_context.append(map_remove)
        map_insert.show()
        map_remove.show()
        map_insert.connect('activate', self.pcm_new_mapping, ins_num, pcm_details)
        map_remove.connect('activate', self.pcm_del_mapping, ins_num, pcm_details)
        map_view.connect('button-press-event', self.pcm_mapping_context, pcm_details)

        pcm_details.sample_info['map_context'] = map_context
        pcm_details.sample_info['map_view'] = map_view
        pcm_details.sample_info['maps'] = {}

        pcm_details.append_page(mappings, gtk.Label('Sample mapping'))

        pcm_details.set_name('pcm')

        selection = sample_list.get_selection()
        selection.connect('changed', self.change_sample, pcm_details)
        selection.select_path(0)
        return pcm_details

    def pcm_mapping_context(self, treeview, event, pcm_details):
        if event.button != 3:
            return False
        x = int(event.x)
        y = int(event.y)
        time = event.time
        pinfo = treeview.get_path_at_pos(x, y)
        treeview.grab_focus()
        if pinfo is not None:
            path, col, cellx, celly = pinfo
            treeview.set_cursor(path, col, 0)
        pcm_details.sample_info['map_context'].popup(None, None, None,
                event.button,
                time)
        return True

    def pcm_new_mapping(self, menuitem, ins_num, pcm_details):
#       print('new mapping for %d' % ins_num)
        map_view = pcm_details.sample_info['map_view']
        list = map_view.get_model()
        cur = map_view.get_cursor()
        pos = 0
        if cur[0]:
            pos = cur[0][0]
        list.insert(pos)

    def pcm_del_mapping(self, menuitem, ins_num, pcm_details):
        map_view = pcm_details.sample_info['map_view']
        cur = map_view.get_cursor()
        if not cur[0]:
            return
        pos = cur[0][0]
        list = map_view.get_model()
        iter = list.get_iter(pos)
        if not iter:
            return
        freq = list.get_value(iter, 0)
#       print('remove mapping %f of %d' % (freq, ins_num))
        if freq <= 0:
            list.remove(iter)
            return
        liblo.send(self.engine, '/kunquat/ins_pcm_del_mapping',
                self.song_id,
                ins_num,
                0, 0, 0, # source, style, strength
                freq,
                0) # random choice entry

    def pcm_set_mapping(self, cell, path, new_text, ins_num, pcm_details, col):
        list = pcm_details.sample_info['map_view'].get_model()
        iter = list.get_iter(path)
        old_freq = list.get_value(iter, 0)
        freq = old_freq
        desc = list.get_value(iter, 1)
        if col == 0:
            freq = float(new_text)
        elif col == 1:
            desc = new_text
        if freq <= 0:
            list.set_value(iter, 1, desc)
            return
        if not desc:
            list.set_value(iter, 0, freq)
            return
        desc = desc.strip()
        values = desc.split(',')
        if len(values) < 3:
            list.set_value(iter, 0, freq)
            return
        sample = int(values[0])
        freq_scale = float(values[1])
        vol_scale = float(values[2])
        if sample < 0 or sample >= 512 or freq_scale <= 0 or vol_scale <= 0:
            list.set_value(iter, 0, freq)
            return
#       print('old mapping %f -> %s' % (freq, desc))
#       print('set_mapping %s for %d at %d' % (new_text, ins_num, col))
        if col == 0 and old_freq != freq:
            liblo.send(self.engine, '/kunquat/ins_pcm_del_mapping',
                    self.song_id,
                    ins_num,
                    0, 0, 0,
                    old_freq,
                    0)
        liblo.send(self.engine, '/kunquat/ins_pcm_set_mapping',
                self.song_id,
                ins_num,
                0, 0, 0, # source, style, strength
                freq,
                0, # random choice entry
                sample,
                freq_scale,
                vol_scale)

    def change_sample(self, selection, pcm_details):
        _, cur = selection.get_selected_rows()
        if not cur:
            return
        cur = cur[0][0]
        if cur == pcm_details.sample_info['cur']:
            return
        pcm_details.sample_info['cur'] = cur
        if cur not in pcm_details.sample_info['samples']:
            pcm_details.sample_info['path'].set_text('')
            pcm_details.sample_info['freq'].handler_block(
                    pcm_details.sample_info['freq'].shid)
            pcm_details.sample_info['freq'].set_value(44100)
            pcm_details.sample_info['freq'].handler_unblock(
                    pcm_details.sample_info['freq'].shid)
            return
        pcm_details.sample_info['path'].set_text(
                pcm_details.sample_info['samples'][cur][0])
        pcm_details.sample_info['freq'].handler_block(
                pcm_details.sample_info['freq'].shid)
        pcm_details.sample_info['freq'].set_value(
                pcm_details.sample_info['samples'][cur][1])
        pcm_details.sample_info['freq'].handler_unblock(
                pcm_details.sample_info['freq'].shid)

    def change_mid_freq(self, spin, ins_num, pcm_details):
        sample_num = pcm_details.sample_info['cur']
        if sample_num < 0:
            return
        spin.user_set = True
        liblo.send(self.engine, '/kunquat/ins_pcm_sample_set_mid_freq',
                self.song_id,
                ins_num,
                sample_num,
                spin.get_value())

    def load_sample(self, file_sel, id, ins_num, sample_num):
        if id == gtk.RESPONSE_CANCEL:
            file_sel.destroy()
        elif id == gtk.RESPONSE_OK:
            path = file_sel.get_filename()
            file_sel.destroy()
            liblo.send(self.engine, '/kunquat/ins_pcm_load_sample',
                    self.song_id,
                    ins_num,
                    sample_num,
                    path)

    def browse_sample(self, button, ins_num, pcm_details):
        sample_num = pcm_details.sample_info['cur']
        if sample_num < 0:
            return
        filter = gtk.FileFilter()
        filter.set_name('WavPack files')
        filter.add_pattern('*.wv')
        file_sel = gtk.FileChooserDialog(buttons=(gtk.STOCK_CANCEL,
                gtk.RESPONSE_CANCEL, gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        file_sel.connect('file-activated', self.load_sample,
                gtk.RESPONSE_OK, ins_num, sample_num)
        file_sel.connect('response', self.load_sample,
                ins_num, sample_num)
        file_sel.add_filter(filter)
        file_sel.show()

    def remove_sample(self, button, ins_num, pcm_details):
        sample_num = pcm_details.sample_info['cur']
        if sample_num < 0:
            return
        liblo.send(self.engine, '/kunquat/ins_pcm_remove_sample',
                self.song_id,
                ins_num,
                sample_num)

    def name_changed(self, cell, path, new_text):
        liblo.send(self.engine, '/kunquat/ins_set_name',
                self.song_id,
                int(path) + 1,
                new_text)

    def change_type(self, combobox):
        old_name = None
        if self.ins_params[self.cur_index - 1]:
            old_name = self.ins_params[self.cur_index - 1][1]
        new_type = combobox.get_active()
        if new_type > 0:
            new_type += 1
        liblo.send(self.engine, '/kunquat/new_ins',
                self.song_id,
                self.cur_index,
                new_type)
        if old_name:
            liblo.send(self.engine, '/kunquat/ins_set_name',
                    self.song_id,
                    self.cur_index,
                    old_name)

    def select_ins(self, selection):
        _, cur = selection.get_selected_rows()
        if cur == []:
            return
        self.cur_index = cur[0][0] + 1
        self.types.handler_block(self.htypes)
        if self.ins_params[self.cur_index - 1]:
            type_index = self.ins_params[self.cur_index - 1][0]
            if type_index > 0:
                type_index -= 1
            self.types.set_active(type_index)
        else:
            self.types.set_active(0)
        self.types.handler_unblock(self.htypes)
        chs = self.ins_details.get_children()
        if len(chs) == 2:
            if chs[1] == self.ui_params[self.cur_index - 1]:
                return
            self.ins_details.remove(chs[1])
            chs[1].hide()
        if self.ui_params[self.cur_index - 1]:
            self.ins_details.pack_start(self.ui_params[self.cur_index - 1])
            self.ui_params[self.cur_index - 1].show_all()

    def __init__(self, engine, server, song_id):
        self.engine = engine
        self.server = server
        self.song_id = song_id

        self.cur_index = 1
        self.ins_params = [None for _ in range(255)]
        self.ui_params = [None for _ in range(255)]
        
        self.types = gtk.combo_box_new_text()
        self.types.append_text('None')
        self.types.append_text('Sine')
        self.types.append_text('Triangle')
        self.types.append_text('Square')
        self.types.append_text('303 Square')
        self.types.append_text('Sawtooth')
        self.types.append_text('PCM')
        self.types.set_active(0)
        self.htypes = self.types.connect('changed', self.change_type)

        self.ins_details = gtk.VBox()
        self.ins_details.pack_start(self.types, False, False)
        self.types.show()

        gtk.HBox.__init__(self)

        self.ins_table = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_STRING)
        self.it_view = gtk.TreeView(self.ins_table)
        self.it_view.set_enable_search(False)
        selection = self.it_view.get_selection()
        selection.connect('changed', self.select_ins)

        for i in range(1, 256):
            iter = self.ins_table.append()
            self.ins_table.set(iter, 0, '%02X' % i)

        selection.select_path(0)

        cell = gtk.CellRendererText()
        column = gtk.TreeViewColumn('#', cell, text=0)
        self.it_view.append_column(column)
        cell = gtk.CellRendererText()
        cell.set_property('editable', True)
        cell.connect('edited', self.name_changed)
        column = gtk.TreeViewColumn('Name', cell, text=1)
        self.it_view.append_column(column)

        it_scroll = gtk.ScrolledWindow()
        it_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        it_scroll.add(self.it_view)
        self.it_view.show()

        self.pack_start(it_scroll)
        it_scroll.show()

        self.pack_start(self.ins_details)
        self.ins_details.show()


