/*
Copyright (C) 2013 hettoo (Gerco van Heerdt)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef WRLC_UTILS_H
#define WRLC_UTILS_H

#include "import.h"

typedef struct parse_state_s {
    qboolean set_color;
    void (*f_char)(char c);
    void (*f_color)(int color);
    char separator;
} parse_state_t;

int die(char *format, ...);
int min(int a, int b);
int max(int a, int b);
unsigned int millis();
int timestring(char *string);
void parse(char *string, void (*f_char)(char c), void (*f_color)(int color));
void parse_init(parse_state_t *state, void (*f_char)(char c), void (*f_color)(int color), char separator);
char *parse_interleaved(char *string, parse_state_t *state);
int parse_peek_count(char *string, parse_state_t *state);
void parse_finish(parse_state_t *state);
char *uncolor(char *string);
qboolean partial_match(char *a, char *b);
int insensitive_cmp(const void *a_raw, const void *b_raw);

#endif
