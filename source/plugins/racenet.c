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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "../plugins.h"
#include "../import.h"
#include "../cs.h"

plugin_interface_t *trap;

static FILE *fp = NULL;
static int fn;
static char msg[MAX_STRING_CHARS];
static int msg_len;
static int cmd_client;

static int cmd_index;

static void cmd_racenet() {
    if (fp != NULL) {
        trap->client_say(cmd_client, "A query is already running");
        return;
    }

    fp = popen(trap->path("plugins/racenet.pl %s", cs_get(trap->client_cs(trap->ui_client()), CS_MAPNAME)), "r");
    fn = fileno(fp);
    fcntl(fn, F_SETFL, O_NONBLOCK);
    strcpy(msg, "\"");
    msg_len = strlen(msg);
    cmd_client = trap->cmd_client();
}

void init(plugin_interface_t *new_trap) {
    trap = new_trap;
    cmd_index = trap->cmd_add_special_generic("racenet", cmd_racenet);
}

void frame() {
    if (fp == NULL)
        return;

    char c;
    ssize_t r = read(fn, &c, 1);
    if (r == -1 && errno == EAGAIN) {
    } else if (r > 0) {
        msg[msg_len++] = (char)c;
    } else {
        if (msg[msg_len - 1] == '\n')
            msg_len--;
        msg[msg_len++] = '\0';
        trap->client_say(cmd_client, msg);
        pclose(fp);
        fp = NULL;
    }
}

void shutdown() {
    if (fp != NULL)
        pclose(fp);
    fp = NULL;
    trap->cmd_remove(cmd_index);
}
