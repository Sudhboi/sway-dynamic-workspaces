#define I3IPC_IMPLEMENTATION
#include "workspaces.h"
#include "i3ipc.h"

#include <alloca.h>
#include <assert.h>
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

/* Renames the workspaces to 1..n in order. Workspaces that already have the
 * right name are left alone, so running this again (e.g. in response to our own
 * rename events) sends no commands. */
void reorder(void) {
  I3ipc_reply_workspaces *ws = i3ipc_get_workspaces();
  char cmd[512], want[16];
  for (int i = 0; i < ws->workspaces_size; ++i) {
    int len = snprintf(want, sizeof(want), "%d", i + 1);
    if (ws->workspaces[i].name_size == len &&
        memcmp(ws->workspaces[i].name, want, len) == 0)
      continue;
    snprintf(cmd, sizeof(cmd), "rename workspace \"%.*s\" to \"%d\"",
             ws->workspaces[i].name_size, ws->workspaces[i].name, i + 1);
    i3ipc_run_command_simple(cmd);
  }
  free(ws);
}

/* Returns the focused workspace's name as an int (the workspaces are numbered
 * after reorder()), or -1 if none is focused. */
static int get_focused_workspace(void) {
  I3ipc_reply_workspaces *ws = i3ipc_get_workspaces();
  int result = -1;
  for (int i = 0; i < ws->workspaces_size; ++i) {
    if (ws->workspaces[i].focused) {
      char name[64];
      snprintf(name, sizeof(name), "%.*s", ws->workspaces[i].name_size,
               ws->workspaces[i].name);
      result = atoi(name);
      break;
    }
  }
  free(ws);
  return result;
}

static int workspace_count(void) {
  I3ipc_reply_workspaces *ws = i3ipc_get_workspaces();
  int n = ws->workspaces_size;
  free(ws);
  return n;
}

static void move_right(void) {
  int count = workspace_count();
  reorder();
  if (get_focused_workspace() == count) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "workspace %d", count + 1);
    i3ipc_run_command_simple(cmd);
  } else {
    i3ipc_run_command_simple("workspace next");
  }
  reorder();
}

static void move_left(void) {
  reorder();
  if (get_focused_workspace() > 1)
    i3ipc_run_command_simple("workspace prev");
  reorder();
}

static void move(char *num) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "workspace %s", num);
  i3ipc_run_command_simple(cmd);
  reorder();
}

/* Runs one command. Returns 0 on success, -1 if the command is unknown. */
int dispatch(char *cmd) {
  char *ptr;
  if (strcmp(cmd, "left") == 0)
    move_right();
  else if (strcmp(cmd, "right") == 0)
    move_left();
  else if (strcmp(cmd, "reorder") == 0)
    reorder();
  else if ((ptr = strstr(cmd, "move_")) != NULL)
    move(ptr + 5);
  else
    return -1;
  return 0;
}
