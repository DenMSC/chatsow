/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "cs.h"

void cs_init(cs_t *cs) {
    int i;
    for (i = 0; i < MAX_CONFIGSTRINGS; i++)
        cs->css[i][0] = '\0';
}

void cs_set(cs_t *cs, int index, char *string) {
    strcpy(cs->css[index], string);
}

char *cs_get(cs_t *cs, int index) {
    return cs->css[index];
}

static char *playerinfo_token(char *playerinfo, char *name, qboolean clear_color) {
    int len = strlen(playerinfo);
    int i;
    static char token[MAX_CONFIGSTRING_CHARS + 2];
    int o = 0;
    qboolean value = qfalse;
    qboolean final = qfalse;
    for (i = 0; i < len; i++) {
        switch (playerinfo[i]) {
            case '\\':
                if (i != 0) {
                    token[o] = '\0';
                    if (!value && !strcmp(token, name))
                        final = qtrue;
                    else if (value && final)
                        return token;
                    value = !value;
                    o = 0;
                    if (final && clear_color) {
                        token[o++] = '^';
                        token[o++] = '7';
                    }
                }
                break;
            default:
                token[o++] = playerinfo[i];
                break;
        }
    }
    return "";
}

char *player_name(cs_t *cs, int num) {
    if (num == 0)
        return "console";
    return playerinfo_token(cs_get(cs, CS_PLAYERINFOS + num - 1), "name", qtrue);
}
