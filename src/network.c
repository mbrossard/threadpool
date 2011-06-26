/*
 * Copyright (c) 2011, Mathias Brossard <mathias@brossard.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "network.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

ssize_t nw_nwrite(int fd, const void *buf, size_t count)
{
    const char *ptr = buf; /* Removes warning on pointer operations */
    ssize_t t, left = count;
    while(left) {
        if((t = write(fd, ptr, left)) < 0) {
            return -1;
        }
        left -= t;
        ptr += t;
    }
    return count;
}

ssize_t nw_nread(int fd, void *buf, size_t count)
{
    char *ptr = buf; /* Removes warning on pointer operation */
    ssize_t t = 0, left = count;
    while(left) {
        if((t = read(fd, ptr, left)) < 0) {
            return -1;
        }
        left -= t;
        ptr += t;
    }
    return count;
}

int nw_unix_client(const char *path, struct sockaddr_un *unixaddr)
{
    int fd;

    if((path == NULL) || strlen(path) > UNIX_PATH_MAX - 1) {
        return -1;
    }

    if((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    memset(unixaddr, 0, sizeof(struct sockaddr_un));
    unixaddr->sun_family = AF_UNIX;
    strncpy(unixaddr->sun_path, path, UNIX_PATH_MAX - 1);
    if(connect(fd, (struct sockaddr *)unixaddr,
               sizeof(struct sockaddr_un)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int nw_unix_server(const char *path, struct sockaddr_un *unixaddr,
                   mode_t rights, uid_t uid, gid_t gid, int backlog)
{
    int fd, one = 1;

    if(strlen(path) > UNIX_PATH_MAX - 1) {
        return -1;
    }
    if((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    /* Should we check this ? Bind would fail anyway */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    memset(unixaddr, 0, sizeof(struct sockaddr_un));
    unixaddr->sun_family = AF_UNIX;
    strncpy(unixaddr->sun_path, path, UNIX_PATH_MAX - 1);
    if(bind(fd, (struct sockaddr *)unixaddr, 
            sizeof(struct sockaddr_un)) < 0) {
        return -1;
    }

    if((rights <= 0) && (chmod(path, rights) < 0)) {
        return -1;
    }

    if(((uid <= 0) || (gid <= 0)) && (chown(path, uid, gid) < 0)) {
        return -1;
    }

    if(listen(fd, backlog) < 0) {
        return -1;
    }

    return fd;
}

int nw_tcp_client(const char *host, int port, struct sockaddr_in *inetaddr)
{
    int fd;
    struct hostent *ent;

    if (((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) ||
        ((ent = gethostbyname(host)) == NULL)) {
        return -1;
    }

    inetaddr->sin_family = AF_INET;
    inetaddr->sin_port = htons(port);
    inetaddr->sin_addr.s_addr = ((struct in_addr *)ent->h_addr)->s_addr;
    if(connect(fd, (struct sockaddr *)inetaddr,
               sizeof(struct sockaddr_in)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int nw_tcp_server(int port, uint32_t address, int backlog)
{
    int fd, one = 1;
    struct sockaddr_in inetaddr;

    if((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    /* Should we check this ? Bind would fail anyway */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    inetaddr.sin_family = AF_INET;
    inetaddr.sin_port = htons(port);
    inetaddr.sin_addr.s_addr = htonl(address);

    if(bind(fd, (struct sockaddr *)&inetaddr, sizeof(inetaddr)) < 0) {
        return -1;
    }

    if(listen(fd, backlog) < 0) {
        return -1;
    }

    return fd;
}


static void daemon_sigterm_handler() {
    /* SIGTERM received from daemon: daemon init OK */
    exit(EXIT_SUCCESS);
}

int nw_daemonize(int stdfd, char *pidfile, uid_t uid, gid_t gid,
                 mode_t mask, char *rundir, pid_t *parent)
{
    struct stat s;
    FILE *f;
    int fd;
    int child_status, grandchild_status;
    pid_t first_child_pid, child_pid;

    /* check existing pid file */
    if(pidfile && (stat(pidfile, &s) != -1)) {
        return -1;
    }
  
    /* The double fork() forces the server to become a child of init,
     * which will always clean up zombie process state on termination.
     * This even works in strange corner cases where the server
     * terminates but this client does not. 
     *
     * TODO: check Stevens' "Advanced Programming in the UNIX
     * Environment" for details
     */

    /* 1st Fork */
    switch(fork()) {
        case -1:
            return -1;

        case 0:
            /* if daemon grandchild init ok, daemon will send SIGTERM to its
             * parent, i.e. this child */
            first_child_pid = getpid();
            /* ready to receive SIGTERM from grandchild */
            signal(SIGTERM, daemon_sigterm_handler);
            break;

        default:
            /* Wait for the (first) child process to finish before exiting */
            do {
                child_pid = wait(&child_status);
            } while(child_pid != -1 && child_pid != first_child_pid);

            /* use child exit status as main exit status */
            if(WIFEXITED(child_status) && WEXITSTATUS(child_status) != EXIT_SUCCESS) {
                fprintf(stderr, "Critical Daemon Init Error\n");
            }
            exit( WIFEXITED(child_status) ? WEXITSTATUS(child_status) : EXIT_SUCCESS );
    }

    /* decouple from parent environment */
    /* set newly created file permissions */
    umask(mask);
    /* change server running dir to a known directory */
    if(chdir(rundir) != 0) {
        fprintf(stderr, "Can't change process running dir to %s: %s\n",
                rundir, strerror(errno));
        return -1;
    }

    setsid();

    /* Change process owner user/group */

    /* Has to be done before grandchild is born so that grandchild
     * inherits same owner user/group and has permission to kill his
     * parent, i.e. this child */
    if((gid && setgid(gid)) || (uid && setuid(uid))) {
        return -1;
    }
    
    /* 2nd Fork */
    switch(fork()) {
        case -1:
            return -1;
            
        case 0:
            break;
            
        default:
            /* Wait for the daemon (grand)child process to finish if init goes wrong.
             * If daemon init OK, SIGTERM kill received from daemon
             * child. See above for SIGTERM handling.
             */
            wait(&grandchild_status);
            /* The grandchild is the actual daemon process.
             * Exiting with grandchild exit status helps propagate daemon
             * init errors to the main/parent process.
             */
            exit( WIFEXITED(grandchild_status) ?
                  WEXITSTATUS(grandchild_status) : EXIT_SUCCESS );
    }
    
    /* Create/Write pid file (as new user/group) */
    if(pidfile) {
        struct stat s;
        
        if(stat(pidfile, &s) != -1) {
            return -1;
        }
        
        if((f = fopen(pidfile, "w+")) == NULL) {
            fprintf(stderr, "Can't open pid file for writing '%s': %s\n",
                    pidfile, strerror(errno));
            return -1;
        }
        
        /* lock pid file to notify other server instances that one is
         * already running */
        if (flock(fileno(f), LOCK_EX) != 0) {
            fprintf(stderr, "Can't lock pid file '%s': %s\n",
                    pidfile, strerror(errno));
            return -1; 
        }
        fprintf(f, "%d\n", (int)getpid());
        if (fclose(f) != 0) {
            fprintf(stderr, "Can't close file '%s': %s\n",
                    pidfile, strerror(errno));
        }
    }
    
    if(stdfd) {
        if(((fd = open("/dev/null", O_RDWR) == -1)) ||
           ((dup2(fd, STDIN_FILENO) < 0)) ||
           ((dup2(fd, STDOUT_FILENO) < 0)) ||
           ((dup2(fd, STDERR_FILENO) < 0))) {
            return -1;
        }
        close(fd);
    }

    /* We may terminate the main process and the child/parent */
    if(parent) {
        *parent = first_child_pid;
    } else {
        fprintf(stderr, "Going to kill\n");
        if(kill(first_child_pid, SIGTERM) != 0) {
            fprintf(stderr, "Daemon could not kill parent init process: %s\n",
                    strerror(errno));
            return -1;
        }
    }
    return 0;
}
