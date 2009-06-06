

/*
 * Copyright 2009 Tomi Jylhä-Ollila
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


#ifndef K_FILE_BASE_H
#define K_FILE_BASE_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Real.h>
#include <Reltime.h>


#define STATE_PATH_LENGTH (512)
#define ERROR_MESSAGE_LENGTH (256)

#define MAGIC_ID "kunquat_"


typedef struct Read_state
{
    bool error;
    char path[STATE_PATH_LENGTH];
    int row;
    char message[ERROR_MESSAGE_LENGTH];
} Read_state;


typedef struct Write_state
{
    bool error;
    char path[STATE_PATH_LENGTH];
    int row;
    char message[ERROR_MESSAGE_LENGTH];
    int indent;
} Write_state;


void Read_state_init(Read_state* state, char* path);


void Read_state_set_error(Read_state* state, char* message, ...);


void Read_state_clear_error(Read_state* state);


char* read_file(FILE* in, Read_state* state);


char* read_const_char(char* str, char result, Read_state* state);


char* read_const_string(char* str, char* result, Read_state* state);


char* read_bool(char* str, bool* result, Read_state* state);


char* read_string(char* str, char* result, int max_len, Read_state* state);


char* read_int(char* str, int64_t* result, Read_state* state);


char* read_double(char* str, double* result, Read_state* state);


char* read_tuning(char* str, Real* result, double* cents, Read_state* state);


char* read_reltime(char* str, Reltime* result, Read_state* state);


#endif // K_FILE_BASE_H


