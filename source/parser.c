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

#include "import.h"
#include "ui.h"
#include "callbacks.h"

static int last_frame = -1;
static int bitflags = 0;
static qboolean reliable = qfalse;
static int servercount = 0;

qboolean connection_reliable() {
    return reliable;
}

int spawn_count() {
    return servercount;
}

static void parse_frame(msg_t *msg) {
    int length = read_short(msg); // length
    int pos = msg->readcount;
    read_long(msg); // serverTime
    int frame = read_long(msg);
    read_long(msg); // delta frame number
    read_long(msg); // ucmd executed
    int flags = read_byte(msg);
    read_byte(msg); // suppresscount

    read_byte(msg); // svc_gamecommands
    int framediff;
    while ((framediff = read_short(msg)) != -1) {
        char *cmd = read_string(msg);
        int numtargets = 0;
        static qbyte targets[MAX_CLIENTS / 8];
        if (flags & FRAMESNAP_FLAG_MULTIPOV) {
            numtargets = read_byte(msg);
            read_data(msg, targets, numtargets);
        }
        if (frame > last_frame + framediff)
            command(cmd, targets, numtargets);
    }
    skip_data(msg, length - (msg->readcount - pos));
    last_frame = frame;
}

void parse_message(msg_t *msg) {
    int cmd;
    while (1) {
        cmd = read_byte(msg);
        switch (cmd) {
            case svc_demoinfo:
                read_long(msg); // length
                read_long(msg); // meta data offset
                read_long(msg); // basetime
                size_t meta_data_realsize = read_long(msg);
                size_t meta_data_maxsize = read_long(msg);
                size_t end = msg->readcount + meta_data_realsize;
                while (msg->readcount < end) {
                    demoinfo_key(read_string(msg));
                    demoinfo_value(read_string(msg));
                }
                skip_data(msg, meta_data_maxsize - meta_data_realsize);
                break;
            case svc_clcack:
                read_long(msg); // reliable acknownledge
                read_long(msg); // ucmd acknowledged
                break;
            case svc_servercmd:
                if (!reliable)
                    read_long(msg); // command number
            case svc_servercs:
                command(read_string(msg), NULL, 0);
                break;
            case svc_serverdata:
                read_long(msg); // protocol version
                servercount = read_long(msg);
                read_short(msg); // snap frametime
                read_string(msg); // base game
                read_string(msg); // game
                read_short(msg); // player entity number
                read_string(msg); // level name
                bitflags = read_byte(msg); // server bitflags
                reliable = (bitflags & SV_BITFLAGS_RELIABLE) != 0;
                int pure = read_short(msg);
                while (pure > 0) {
                    read_string(msg); // pure pk3 name
                    read_long(msg); // checksum
                    pure--;
                }
                break;
            case svc_spawnbaseline:
                read_delta_entity(msg, read_entity_bits(msg));
                break;
            case svc_frame:
                parse_frame(msg);
                break;
            case -1:
                return;
            default:
                ui_output("unknown command: %d\n", cmd);
                int i;
                for (i = 0; i < msg->cursize; i++) {
                    if (msg->data[i])
                        ui_output("%i -> %i (%i): %c\n", msg->readcount, i, msg->cursize - msg->readcount, msg->data[i]);
                }
                return;
        }
    }
}

static qboolean read_demo_message(FILE *fp) {
    int length;
    if (!fread(&length, 4, 1, fp))
        return qfalse;
    length = LittleLong(length);
    if (length < 0)
        return qfalse;
    msg_t msg;
    if (!fread(msg.data, length, 1, fp))
        return qfalse;
    msg.readcount = 0;
    msg.cursize = length;
    msg.maxsize = sizeof(msg.data);
    msg.compressed = qfalse;
    parse_message(&msg);
    return qtrue;
}

void parse_demo(FILE *fp) {
    while (read_demo_message(fp))
        ;
}
