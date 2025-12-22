/*
    server.h
    TCP server interface for remote GUI communication

    Copyright (c) 2024
    This file is a part of aulon.
*/

#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

// Start the TCP server on the specified port
// Returns 0 on success, -1 on error
int server_start(uint16_t port);

// Main server loop - call this after server_start
void server_loop(void);

// Stop the server
void server_stop(void);

#endif /* SERVER_H */
