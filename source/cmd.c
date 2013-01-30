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

#include "import.h"
#include "global.h"
#include "cs.h"
#include "client.h"
#include "cmd.h"

#define CMD_STACK_SIZE 16

typedef enum cmd_type_e {
    CT_NORMAL,
    CT_PERSISTENT,
    CT_SPECIAL,
    CT_SPECIAL_PERSISTENT,
    CT_FROM_SERVER,
    CT_SERVER,
    CT_GLOBAL,
    CT_FIND_FREE,
    CT_BROADCAST,
    CT_BROADCAST_ALL,
    CT_TOTAL
} cmd_type_t;

typedef struct cmd_s {
    char *name;
    void (*f)();
    cmd_type_t type;
    qboolean clients[CLIENTS];
    int index;
} cmd_t;

typedef struct cmd_stack_s {
    int client;
    int argc;
    char argv[MAX_ARGC][MAX_ARG_SIZE];
    char args[MAX_ARGS_SIZE];
    int args_index[MAX_ARGC];
} cmd_stack_t;

cmd_t cmds[MAX_CMDS];
int cmd_count = 0;

static cmd_stack_t cmd_stack[CMD_STACK_SIZE];
static int cmd_stack_count = 0;

static cmd_stack_t *s = NULL;

static void cmd_stack_push() {
    if (cmd_stack_count == CMD_STACK_SIZE)
        die("Command stack overflow");
    s = cmd_stack + cmd_stack_count++;
    s->client = -1;
    s->argc = 0;
    return;
}

static void cmd_stack_pop() {
    cmd_stack_count--;
    if (cmd_stack_count == 0)
        s = NULL;
    else
        s = cmd_stack + cmd_stack_count - 1;
}

void parse_cmd(char *cmd) {
    s->argc = 0;
    strcpy(s->args, cmd);
    int i;
    qboolean escaped = qfalse;
    char quote = '\0';
    int len = strlen(s->args);
    int o = 0;
    int start = 0;
    for (i = 0; i < len; i++) {
        qboolean normal = qfalse;
        qboolean skip = qfalse;
        char add = '\0';
        if (escaped) {
            if (quote != s->args[i] && s->args[i] != '\\')
                add = '\\';
            escaped = qfalse;
            normal = qtrue;
        } else {
            switch (s->args[i]) {
                case '\\':
                    escaped = qtrue;
                    break;
                case ' ':
                case '\t':
                case '\n':
                    if (quote != '\0')
                        normal = qtrue;
                    else if (o > 0)
                        skip = qtrue;
                    else
                        start = i + 1;
                    break;
                case '\'':
                case '"':
                    if (quote == s->args[i])
                        skip = qtrue;
                    else if (quote == '\0')
                        quote = s->args[i];
                    else
                        normal = qtrue;
                    break;
                default:
                    normal = qtrue;
                    break;
            }
        }
        if (skip) {
            s->args_index[s->argc] = start;
            s->argv[s->argc][o] = '\0';
            s->argc++;
            o = 0;
            start = i + 1;
        }
        if (add != '\0' && o < MAX_ARG_SIZE - 1)
            s->argv[s->argc][o++] = add;
        if (normal && o < MAX_ARG_SIZE - 1)
            s->argv[s->argc][o++] = s->args[i];
    }
    if (o > 0 || (i >= 1 && (s->args[i - 1] == ' ' || s->args[i - 1] == '\t' || s->args[i - 1] == '\n'))) {
        s->args_index[s->argc] = start;
        s->argv[s->argc][o] = '\0';
        s->argc++;
    }
}

static qboolean cmd_type_extends(int type, int parent) {
    if (type == parent)
        return qtrue;

    if (parent == CT_NORMAL && (type == CT_PERSISTENT || type == CT_SERVER))
        return qtrue;

    if (parent == CT_SPECIAL && type == CT_SPECIAL_PERSISTENT)
        return qtrue;

    if (parent == CT_GLOBAL && (type == CT_FIND_FREE || type == CT_BROADCAST || type == CT_BROADCAST_ALL))
        return qtrue;

    return qfalse;
}

static qboolean cmd_type_compatible(int type, int parent) {
    if (cmd_type_extends(type, parent))
        return qtrue;

    if (parent == CT_NORMAL && cmd_type_extends(type, CT_GLOBAL))
        return qtrue;

    return qfalse;
}

static qboolean cmd_valid(cmd_t *cmd, int c, qboolean partial) {
    int len = strlen(cmd_argv(0));
    return (partial ? !strncmp(cmd->name, cmd_argv(0), len)
            : !strcmp(cmd->name, cmd_argv(0)))
        && (c < 0 || cmd->clients[c])
        && (cmd->type != CT_NORMAL || cmd->type != CT_SPECIAL || client_active(c));
}

static cmd_t *cmd_find(cmd_t *cmd, int c, int type, qboolean partial) {
    if (!partial && cmd_argc() == 0)
        return NULL;

    int i;
    for (i = cmd ? (cmd - cmds) + 1 : 0; i < cmd_count; i++) {
        if (cmd_type_compatible(cmds[i].type, type) && cmd_valid(cmds + i, c, partial))
            return cmds + i;
    }

    return NULL;
}

static int normal_type(int c) {
    return c >= 0 ? CT_NORMAL : CT_GLOBAL;
}

int cmd_suggest(int c, char *name, char suggestions[][MAX_SUGGESTION_SIZE]) {
    int type = normal_type(c);
    int count = 0;
    cmd_stack_push();
    parse_cmd(name);
    if (cmd_argc() <= 1) {
        cmd_t *cmd = NULL;
        while ((cmd = cmd_find(cmd, c, type, qtrue)) != NULL) {
            if (cmd->name[0])
                strcpy(suggestions[count++], cmd->name);
        }
    }
    cmd_stack_pop();
    return count;
}

static void cmd_execute_real(int c, char *name, int type) {
    cmd_stack_push();
    parse_cmd(name);

    cmd_t *cmd = cmd_find(NULL, c, type, qfalse);
    if (!cmd) {
        if (!cmd_type_compatible(type, CT_SPECIAL))
            ui_output(c, "Unknown command: \"%s\"\n", cmd_argv(0));
        cmd_stack_pop();
        return;
    }

    int start = c;
    int end = c;
    qboolean switch_screen = qfalse;
    if (cmd_type_compatible(cmd->type, type))
        type = cmd->type;
    if (type == CT_BROADCAST && c < 0) {
        start = 0;
        end = CLIENTS - 1;
    } else if (type == CT_BROADCAST_ALL) {
        start = -1;
        end = CLIENTS - 1;
    } else if (type == CT_FIND_FREE && c < 0) {
        int i;
        qboolean found = qfalse;
        for (i = 0; i < CLIENTS && !found; i++) {
            if (!client_active(i)) {
                start = i;
                end = i;
                found = qtrue;
                switch_screen = qtrue;
                break;
            }
        }
        if (!found) {
            ui_output(c, "No free client found.\n");
            cmd_stack_pop();
            return;
        }
    }
    for (s->client = start; s->client <= end; s->client++) {
        if (type == CT_SERVER)
            client_command(s->client, "%s", cmd_args(0));
        else
            cmd->f();
        if (switch_screen)
            set_screen(s->client + 1);
    }
    cmd_stack_pop();
}

void cmd_execute(int c, char *cmd) {
    cmd_execute_real(c, cmd, normal_type(c));
}

void cmd_execute_special(int c, char *cmd) {
    cmd_execute_real(c, cmd, CT_SPECIAL);
}

void cmd_execute_from_server(int c, char *cmd) {
    cmd_execute_real(c, cmd, CT_FROM_SERVER);
}

int cmd_client() {
    return s->client;
}

int cmd_argc() {
    return s->argc;
}

char *cmd_argv(int index) {
    if (index >= s->argc)
        return "";
    return s->argv[index];
}

char *cmd_args(int start) {
    if (start >= s->argc)
        return "";
    return s->args + s->args_index[start];
}

static cmd_t *cmd_find_exact(char *name, void(*f)(), int type) {
    int i;
    for (i = 0; i < cmd_count; i++) {
        if (cmds[i].f == f && cmds[i].name == name && cmds[i].type == type)
            return cmds + i;
    }
    return NULL;
}

static cmd_t *cmd_reserve(char *name, void (*f)(), int type) {
    cmd_t *cmd = cmd_find_exact(name, f, type);
    if (cmd)
        return cmd;

    if (cmd_count == MAX_CMDS)
        die("Command count overflow.\n");
    cmd = cmds + cmd_count++;

    int i;
    for (i = 0; i < CLIENTS; i++)
        cmd->clients[i] = qfalse;
    cmd->name = name;
    cmd->f = f;
    cmd->type = type;

    return cmd;
}

static void cmd_allow_all(cmd_t *cmd) {
    int i;
    for (i = 0; i < CLIENTS; i++)
        cmd->clients[i] = qtrue;
}

static int cmd_index(cmd_t *cmd) {
    static int index = 0;
    cmd->index = index++;
    return cmd->index;
}

int cmd_add(int client, char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_NORMAL);
    cmd->clients[client] = qtrue;
    return cmd_index(cmd);
}

int cmd_add_persistent(int client, char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_PERSISTENT);
    cmd->clients[client] = qtrue;
    return cmd_index(cmd);
}

int cmd_add_generic(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_NORMAL);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_special(int client, char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_SPECIAL);
    cmd->clients[client] = qtrue;
    return cmd_index(cmd);
}

int cmd_add_special_persistent(int client, char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_SPECIAL_PERSISTENT);
    cmd->clients[client] = qtrue;
    return cmd_index(cmd);
}

int cmd_add_special_generic(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_SPECIAL);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_from_server(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_FROM_SERVER);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_server(int client, char *name) {
    cmd_t *cmd = cmd_reserve(name, NULL, CT_SERVER);
    cmd->clients[client] = qtrue;
    return cmd_index(cmd);
}

int cmd_add_global(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_GLOBAL);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_find_free(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_FIND_FREE);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_broadcast(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_BROADCAST);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

int cmd_add_broadcast_all(char *name, void (*f)()) {
    cmd_t *cmd = cmd_reserve(name, f, CT_BROADCAST_ALL);
    cmd_allow_all(cmd);
    return cmd_index(cmd);
}

void cmd_remove(int index) {
    int i;
    int skip = 0;
    for (i = 0; i + skip < cmd_count; i++) {
        if (cmds[i].index == index)
            skip++;
        if (skip > 0 && i + skip < cmd_count)
            cmds[i] = cmds[i + skip];
    }
}
