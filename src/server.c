/*
    server.c
    TCP server for remote GUI communication

    Copyright (c) 2024
    This file is a part of aulon.

    Provides a JSON-based TCP API for the iQue Tools GUI to communicate
    with aulon running in a Windows XP VM.
*/

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define closesocket close
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu_func.h"
#include "server.h"
#include "usb.h"

#define BUFFER_SIZE 4096
#define MAX_RESPONSE_SIZE 65536

static SOCKET server_socket = INVALID_SOCKET;
static SOCKET client_socket = INVALID_SOCKET;
static int running = 0;

// Simple JSON helpers (no external dependencies)
static const char *json_get_string(const char *json, const char *key,
                                   char *value, size_t max_len) {
  char search[64];
  snprintf(search, sizeof(search), "\"%s\"", key);
  const char *pos = strstr(json, search);
  if (!pos)
    return NULL;

  pos = strchr(pos + strlen(search), ':');
  if (!pos)
    return NULL;

  while (*pos == ':' || *pos == ' ' || *pos == '\t')
    pos++;

  if (*pos == '"') {
    pos++;
    const char *end = strchr(pos, '"');
    if (!end)
      return NULL;
    size_t len = end - pos;
    if (len >= max_len)
      len = max_len - 1;
    strncpy(value, pos, len);
    value[len] = '\0';
    return value;
  }
  return NULL;
}

// Build JSON response
static void json_response(char *buffer, size_t max_len, int success,
                          const char *message, const char *data) {
  if (data && strlen(data) > 0) {
    snprintf(buffer, max_len,
             "{\"success\":%s,\"message\":\"%s\",\"data\":%s}\n",
             success ? "true" : "false", message, data);
  } else {
    snprintf(buffer, max_len, "{\"success\":%s,\"message\":\"%s\"}\n",
             success ? "true" : "false", message);
  }
}

// Handle incoming command
static void handle_command(const char *request, char *response,
                           size_t max_response) {
  char cmd[64] = {0};
  char filename[256] = {0};

  if (!json_get_string(request, "cmd", cmd, sizeof(cmd))) {
    json_response(response, max_response, 0, "Missing 'cmd' field", NULL);
    return;
  }

  printf("[Server] Command: %s\n", cmd);

  if (strcmp(cmd, "ping") == 0) {
    json_response(response, max_response, 1, "pong", NULL);
  } else if (strcmp(cmd, "init") == 0) {
    int result = Init();
    if (result) {
      json_response(response, max_response, 1, "Connected to iQue Player",
                    NULL);
    } else {
      json_response(response, max_response, 0,
                    "Failed to connect to iQue Player", NULL);
    }
  } else if (strcmp(cmd, "close") == 0) {
    int result = Close();
    json_response(response, max_response, result ? 1 : 0,
                  result ? "Connection closed" : "Failed to close connection",
                  NULL);
  } else if (strcmp(cmd, "get_bbid") == 0) {
    int result = GetBBID();
    if (result) {
      // BBID is printed to stdout by GetBBID, we need to capture it
      // For now just indicate success
      json_response(response, max_response, 1, "BBID retrieved", NULL);
    } else {
      json_response(response, max_response, 0, "Failed to get BBID", NULL);
    }
  } else if (strcmp(cmd, "list_files") == 0) {
    int result = ListFiles();
    json_response(response, max_response, result ? 1 : 0,
                  result ? "Files listed" : "Failed to list files", NULL);
  } else if (strcmp(cmd, "dump_nand") == 0) {
    int result = DumpNand();
    json_response(response, max_response, result ? 1 : 0,
                  result ? "NAND dumped to nand.bin and spare.bin"
                         : "Failed to dump NAND",
                  NULL);
  } else if (strcmp(cmd, "dump_fs") == 0) {
    int result = DumpCurrentFS();
    json_response(response, max_response, result ? 1 : 0,
                  result ? "Filesystem dumped to current_fs.bin"
                         : "Failed to dump filesystem",
                  NULL);
  } else if (strcmp(cmd, "read_file") == 0) {
    if (json_get_string(request, "filename", filename, sizeof(filename))) {
      char input_line[280];
      snprintf(input_line, sizeof(input_line), "3 %s", filename);
      int result = AulonReadFile(input_line);
      json_response(response, max_response, result ? 1 : 0,
                    result ? "File read successfully" : "Failed to read file",
                    NULL);
    } else {
      json_response(response, max_response, 0, "Missing 'filename' field",
                    NULL);
    }
  } else if (strcmp(cmd, "set_led") == 0) {
    char value[16] = {0};
    if (json_get_string(request, "value", value, sizeof(value))) {
      char input_line[32];
      snprintf(input_line, sizeof(input_line), "H %s", value);
      int result = SetLED(input_line);
      json_response(response, max_response, result ? 1 : 0,
                    result ? "LED set" : "Failed to set LED", NULL);
    } else {
      json_response(response, max_response, 0, "Missing 'value' field", NULL);
    }
  } else if (strcmp(cmd, "status") == 0) {
    int connected = usb_handle_exists();
    char data[64];
    snprintf(data, sizeof(data), "{\"connected\":%s}",
             connected ? "true" : "false");
    json_response(response, max_response, 1, "Status retrieved", data);
  } else {
    json_response(response, max_response, 0, "Unknown command", NULL);
  }
}

int server_start(uint16_t port) {
#ifdef _WIN32
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
    fprintf(stderr, "[Server] WSAStartup failed\n");
    return -1;
  }
#endif

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == INVALID_SOCKET) {
    fprintf(stderr, "[Server] Socket creation failed\n");
    return -1;
  }

  // Allow socket reuse
  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
             sizeof(opt));

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == SOCKET_ERROR) {
    fprintf(stderr, "[Server] Bind failed\n");
    closesocket(server_socket);
    return -1;
  }

  if (listen(server_socket, 1) == SOCKET_ERROR) {
    fprintf(stderr, "[Server] Listen failed\n");
    closesocket(server_socket);
    return -1;
  }

  printf("[Server] Listening on port %d...\n", port);
  running = 1;
  return 0;
}

void server_loop(void) {
  char request[BUFFER_SIZE];
  char response[MAX_RESPONSE_SIZE];

  while (running) {
    printf("[Server] Waiting for client connection...\n");

    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);

    if (client_socket == INVALID_SOCKET) {
      if (running) {
        fprintf(stderr, "[Server] Accept failed\n");
      }
      continue;
    }

    printf("[Server] Client connected from %s\n",
           inet_ntoa(client_addr.sin_addr));

    // Handle client requests
    while (running) {
      memset(request, 0, sizeof(request));
      int bytes_received = recv(client_socket, request, sizeof(request) - 1, 0);

      if (bytes_received == 0) {
        printf("[Server] Client disconnected\n");
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
        break; // Exit inner loop to wait for new client
      } else if (bytes_received < 0) {
        fprintf(stderr, "[Server] Recv failed\n");
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
        break; // Exit inner loop to wait for new client
      }

      request[bytes_received] = '\0';
      printf("[Server] Received: %s\n", request);

      memset(response, 0, sizeof(response));
      handle_command(request, response, sizeof(response));

      if (send(client_socket, response, strlen(response), 0) == SOCKET_ERROR) {
        perror("[Server] Send failed");
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
        break; // Exit inner loop if send fails
      }
    }
  }
}

void server_stop(void) {
  running = 0;
  if (client_socket != INVALID_SOCKET) {
    closesocket(client_socket);
    client_socket = INVALID_SOCKET;
  }
  if (server_socket != INVALID_SOCKET) {
    closesocket(server_socket);
    server_socket = INVALID_SOCKET;
  }
#ifdef _WIN32
  WSACleanup();
#endif
  printf("[Server] Stopped\n");
}
