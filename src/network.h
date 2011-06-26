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

#ifndef _NETWORK_H_
#define _NETWORK_H_

/**
 * @file network.h
 * @brief Network Header File
 */

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>

/**
 * @brief Write on network socket
 * @param fd
 * @param buf
 * @param count
 */
ssize_t nw_nwrite(int fd, const void *buf, size_t count);

/**
 * @brief Read from network socket
 * @param fd
 * @param buf
 * @param count
 */
ssize_t nw_nread(int fd, void *buf, size_t count);

/**
 * @brief Connect client UNIX domain socket
 * @param path
 * @param unixaddr
 * @return filedes on socket on success, -1 on error and errno is set appropriately
 */
int nw_unix_client(const char *path, struct sockaddr_un *unixaddr);

/**
 * @brief Bind server UNIX domain socket
 * @param path
 * @param rights
 * @param uid
 * @param gid
 * @param backlog
 * @return filedes on socket on success, -1 on error and errno is set appropriately
 */
int nw_unix_server(const char *path, struct sockaddr_un *unixaddr,
                   mode_t rights, uid_t uid, gid_t gid, int backlog);

/**
 * @brief Connect client in TCP domain.
 * @param host
 * @param port
 * @param inetaddr
 * @return filedes on socket on success, -1 on error and errno is set appropriately
 */
int nw_tcp_client(const char *host, int port, struct sockaddr_in *inetaddr);

/**
 * @brief Bind socket in TCP domain.
 * @param port
 * @param address
 * @param backlog
 * @return filedes on socket on success, -1 on error and errno is set appropriately
 */
int nw_tcp_server(int port, uint32_t address, int backlog);

/**
 * Make a daemon
 *
 * @param stdfd   if non-zero redirect stdin, stdout, stderr to /dev/null
 * @param pidfile path of the pid file to write/check or NULL
 * @param user    new process owner (username)
 * @param group	  new process owner group or NULL to keep existing
 * @param pidfile path of the pid file to write/check or NULL
 * @param mask	  umask for file creation permissions
 * @param rundir  new running directory
 * @param parent  pointer to daemon's parent pid.
 *  If parent is NULL, parent is killed when daemonize successfully
 *  returns
 *  If not, daemonize sets it to parent pid so that you can kill the
 *  parent later if needed
 * @returns 0 if successful, -1 otherwise.
 *
 * @note This function forks two times but the calling process isn't
 * terminated until the function has finished in the final process.
 * That way, the pidfile is created *before* the calling process exits.
 */
int nw_daemonize(int closestdfd, char *pidfile, uid_t uid, gid_t gid,
                 mode_t mask, char *rundir, pid_t *parent);

#endif
