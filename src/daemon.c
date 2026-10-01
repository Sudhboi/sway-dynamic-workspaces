#define I3IPC_IMPLEMENTATION
#include "daemon.h"
#include "i3ipc.h"
#include "workspaces.h"

#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

int connect_sway(void) {
  /* i3ipc-simple falls back to `i3 --get-socketpath`, which doesn't exist under
   * sway */
  char *sock = getenv("SWAYSOCK");
  if (!sock)
    sock = getenv("I3SOCK");
  /* i3ipc_init_try() free()s a caller-supplied path, so hand it a heap copy */
  if (i3ipc_init_try(sock ? strdup(sock) : NULL)) {
    i3ipc_error_print("dynwork");
    return -1;
  }
  return 0;
}

static void socket_path(struct sockaddr_un *addr) {
  memset(addr, 0, sizeof(*addr));
  addr->sun_family = AF_UNIX;
  char const *env = getenv("DYNWORK_SOCK");
  if (env) {
    snprintf(addr->sun_path, sizeof(addr->sun_path), "%s", env);
    return;
  }
  char const *dir = getenv("XDG_RUNTIME_DIR");
  if (!dir)
    dir = "/tmp";
  snprintf(addr->sun_path, sizeof(addr->sun_path), "%s/dynwork.sock", dir);
}
/* Reads newline-separated commands from the client, runs each and answers
 * with "ok" or "error: ..." per line. */
static void handle_client(int fd) {
  char buf[256];
  size_t len = 0;
  for (;;) {
    ssize_t n = read(fd, buf + len, sizeof(buf) - 1 - len);
    if (n <= 0)
      break;
    len += n;
    buf[len] = '\0';

    char *line = buf;
    char *nl;
    while ((nl = strchr(line, '\n'))) {
      *nl = '\0';
      if (nl > line && nl[-1] == '\r')
        nl[-1] = '\0';
      if (*line) {
        if (dispatch(line) == 0)
          dprintf(fd, "ok\n");
        else
          dprintf(fd, "error: bruh unknown command '%s'\n", line);
      }
      line = nl + 1;
    }
    len = strlen(line);
    memmove(buf, line, len);
    if (len == sizeof(buf) - 1) {
      dprintf(fd, "error: line too long\n");
      break;
    }
  }
  /* Allow a final command without a trailing newline */
  if (len > 0) {
    buf[len] = '\0';
    if (dispatch(buf) == 0)
      dprintf(fd, "ok\n");
    else
      dprintf(fd, "error: nruh2 unknown command '%s'\n", buf);
  }
  close(fd);
}

int run_daemon(void) {
  if (connect_sway())
    return 1;

  struct sockaddr_un addr;
  socket_path(&addr);

  int srv = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (srv < 0) {
    perror("socket");
    return 1;
  }

  /* Remove a stale socket left by a previous instance, but refuse to start
   * if another daemon is still answering on it. */
  int probe = socket(AF_UNIX, SOCK_STREAM, 0);
  if (connect(probe, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
    fprintf(stderr, "dynwork: daemon already running on %s\n", addr.sun_path);
    return 1;
  }
  close(probe);
  unlink(addr.sun_path);

  if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }
  if (listen(srv, 8) < 0) {
    perror("listen");
    return 1;
  }

  signal(SIGPIPE, SIG_IGN);
  for (;;) {
    int fd = accept(srv, NULL, NULL);
    if (fd < 0) {
      if (errno == EINTR)
        continue;
      perror("accept");
      return 1;
    }
    handle_client(fd);
  }
}

/* Sends cmd to a running daemon and prints its reply. Returns the exit code,
 * or -1 if no daemon is listening. */
int send_to_daemon(char const *cmd) {
  struct sockaddr_un addr;
  socket_path(&addr);

  int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0)
    return -1;
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }

  dprintf(fd, "%s\n", cmd);
  shutdown(fd, SHUT_WR);

  char reply[256];
  ssize_t n = read(fd, reply, sizeof(reply) - 1);
  close(fd);
  if (n <= 0)
    return 1;
  reply[n] = '\0';
  if (strncmp(reply, "ok", 2) == 0)
    return 0;
  fputs(reply, stderr);
  return 1;
}
