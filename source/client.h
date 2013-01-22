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
#include "ui.h"
#include "cs.h"

void client_register_commands();
void client_start(int id);
void client_frame(int id);
void client_activate(int id);
qboolean client_active(int id);
qboolean client_ready(int id);
void client_ack(int id, int num);
void client_command(int id, char *format, ...);
cs_t *client_cs(int id);
int player_suggest(int id, char *cmd, char suggestions[][MAX_SUGGESTION_SIZE]);
void disconnect(int id);

void demoinfo_key(int id, char *key);
void demoinfo_value(int id, char *value);
void execute(int id, char *cmd, qbyte *targets, int target_count);

void set_protocol(int id, int new_protocol);
void set_spawn_count(int id, int new_spawn_count);
int get_bitflags(int id);
void set_bitflags(int id, int new_bitflags);
void set_game(int id, char *new_game);
void set_playernum(int id, int new_playernum);
void set_level(int id, char *new_level);

#endif
