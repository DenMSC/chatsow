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

#ifndef WRLC_CLIENT_H
#define WRLC_CLIENT_H

#include "import.h"

void client_start(char *new_host, char *new_port, char *new_name);
void client_activate();
void client_ack(int num);
void client_command(char *format, ...);
void client_stop();

void demoinfo_key(char *key);
void demoinfo_value(char *value);
void execute(char *cmd, qbyte *targets, int target_count);

void set_protocol(int new_protocol);
void set_spawn_count(int new_spawn_count);
int get_bitflags();
void set_bitflags(int new_bitflags);
void set_game(char *new_game);
void set_playernum(int new_playernum);
void set_level(char *new_level);

#endif
