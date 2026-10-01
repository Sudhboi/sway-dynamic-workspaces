#define I3IPC_IMPLEMENTATION
#include "daemon.h"
#include "i3ipc.h"
#include "workspaces.h"

#include <alloca.h>
#include <assert.h>
#include <poll.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(
        stderr,
        "usage: %s {left|right|daemon}\n"
        "Dynamic Workspace Manager for Sway\n"
        "\n"
        "  daemon      listen on $DYNWORK_SOCK (default "
        "$XDG_RUNTIME_DIR/dynwork.sock)\n"
        "  left|right  send to the daemon if running, otherwise act directly\n",
        argv[0]);
    return 2;
  }

  if (strcmp(argv[1], "daemon") == 0)
    return run_daemon();

  int rc = send_to_daemon(argv[1]);
  if (rc >= 0)
    return rc;

  if (connect_sway())
    return 1;
  if (dispatch(argv[1]) != 0) {
    fprintf(stderr, "dynwork: unknown command '%s'\n", argv[1]);
    return 2;
  }
  return 0;
}
