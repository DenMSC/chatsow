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

#include "../plugins.h"
#include "../client.h"
#include "../cs.h"
#include "../utils.h"

#define LIVE_UPDATE 10000

plugin_interface_t *trap;
unsigned int last_update;

void init(plugin_interface_t *new_trap) {
    trap = new_trap;
    last_update = 0;
}

void frame() {
    if (millis() >= last_update + LIVE_UPDATE) {
        FILE *fp = fopen(trap->path("live.txt"), "w");
        int i;
        for (i = 0; i < CLIENTS; i++) {
            if (trap->client_active(i)) {
                cs_t *cs = trap->client_cs(i);
                char *server = cs_get(cs, 0);
                if (*server) {
                    fprintf(fp, "%s\n", server);
                    fprintf(fp, "%s:%d\n", trap->get_host(i), trap->get_port(i));
                    fprintf(fp, "%s\n", trap->get_level(i));
                    int j;
                    for (j = 1; j <= MAX_CLIENTS; j++) {
                        if (trap->get_playernum(i) != j) {
                            char *name = player_name(cs, j);
                            if (name && *name) {
                                int team = trap->get_stat(i, j, STAT_TEAM);
                                fprintf(fp, "%s\n%d\n%d\n", name, team == TEAM_SPECTATOR ? -9999 : trap->get_stat(i, j, STAT_SCORE), team);
                            }
                        }
                    }
                    fprintf(fp, "\n");
                }
            }
        }
        fclose(fp);
        last_update = millis();
    }
}

void shutdown() {
    remove(trap->path("live.txt"));
}
