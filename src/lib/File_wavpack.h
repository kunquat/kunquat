

/*
 * Copyright 2008 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_FILE_WAVPACK_H
#define K_FILE_WAVPACK_H


#include <stdbool.h>
#include <stdio.h>

#include <Sample.h>
#include <archive.h>
#include <archive_entry.h>


bool File_wavpack_load_sample(Sample* sample, FILE* in,
                              struct archive* reader,
                              struct archive_entry* entry);


#endif // K_FILE_WAVPACK_H


