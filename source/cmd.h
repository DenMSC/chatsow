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

#ifndef WRLC_CMD_H
#define WRLC_CMD_H

#define MAX_CMDS 2048*4 // :D
#define MAX_ARGC 512
#define MAX_ARG_SIZE 2048
#define MAX_ARGS_SIZE (MAX_ARGC * MAX_ARG_SIZE)

#include "import.h"
#include "ui.h"

void cmd_execute(int c, char *cmd);
void cmd_execute_public(int c, int caller, char *cmd);
void cmd_execute_event(int c, char *cmd);
void cmd_execute_from_server(int c, char *cmd);
int cmd_suggest(int c, char *name, int cursor, char suggestions[][MAX_SUGGESTION_SIZE], qboolean public);

int cmd_client();
int cmd_caller();
int cmd_argc();
char *cmd_argv(int index);
char *cmd_args(int start);

int cmd_add(int client, char *name, void (*f)());
int cmd_add_persistent(int client, char *name, void (*f)());
int cmd_add_generic(char *name, void (*f)());
int cmd_add_public(int client, char *name, void (*f)());
int cmd_add_public_persistent(int client, char *name, void (*f)());
int cmd_add_public_generic(char *name, void (*f)());
int cmd_add_event(char *name, void (*f)());
int cmd_add_from_server(char *name, void (*f)());
int cmd_add_server(int client, char *name);
int cmd_add_global(char *name, void (*f)());
int cmd_add_find_free(char *name, void (*f)());
int cmd_add_broadcast(char *name, void (*f)());
int cmd_add_broadcast_all(char *name, void (*f)());
int cmd_add_cvar(char *name, void (*f)(), void *(*cvar_get)());
void cmd_complete(int index, int (*complete)(int arg, char suggestions[][MAX_SUGGESTION_SIZE]));
void cmd_remove(int index);

#endif
