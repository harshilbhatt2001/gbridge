/*
 * GBridge (Greybus Bridge)
 * Copyright (c) 2016 Alexandre Bailon
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debug.h"
#include "gbridge.h"
#include <endian.h>

#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define MAGENTA "\x1b[35m"
#define RESET "\x1b[0m"

int log_level = LL_VERBOSE;

void set_log_level(int ll) { log_level = ll; }

struct operation_name {
  uint8_t type;
  const char *name;
};

static const struct operation_name control_types[] = {
    {GB_CONTROL_TYPE_VERSION, "VERSION"},
    {GB_CONTROL_TYPE_PROBE_AP, "PROBE_AP"},
    {GB_CONTROL_TYPE_GET_MANIFEST_SIZE, "GET_MANIFEST_SIZE"},
    {GB_CONTROL_TYPE_GET_MANIFEST, "GET_MANIFEST"},
    {GB_CONTROL_TYPE_CONNECTED, "CONNECTED"},
    {GB_CONTROL_TYPE_DISCONNECTED, "DISCONNECTED"},
    {GB_CONTROL_TYPE_TIMESYNC_ENABLE, "TIMESYNC_ENABLE"},
    {GB_CONTROL_TYPE_TIMESYNC_DISABLE, "TIMESYNC_DISABLE"},
    {GB_CONTROL_TYPE_TIMESYNC_AUTHORITATIVE, "TIMESYNC_AUTHORITATIVE"},
    {GB_CONTROL_TYPE_BUNDLE_VERSION, "BUNDLE_VERSION"},
    {GB_CONTROL_TYPE_DISCONNECTING, "DISCONNECTING"},
    {GB_CONTROL_TYPE_TIMESYNC_GET_LAST_EVENT, "TIMESYNC_GET_LAST_EVENT"},
    {GB_CONTROL_TYPE_MODE_SWITCH, "MODE_SWITCH"},
    {0, NULL}};

static const struct operation_name request_types[] = {
    {GB_REQUEST_TYPE_CPORT_SHUTDOWN, "CPORT_SHUTDOWN"},
    {GB_REQUEST_TYPE_INVALID, "INVALID"},
    {0, NULL}};

static const char *get_operation_name(uint8_t type) {
  const struct operation_name *table;
  uint8_t base_type = type & ~0x80;

  // Check for control protocol types (0x01-0x15)
  if (base_type >= GB_CONTROL_TYPE_VERSION &&
      base_type <= GB_CONTROL_TYPE_INTF_HIBERNATE_ABORT) {
    table = control_types;
  }
  // Check for request types (0x00-0x7f)
  else if (base_type <= GB_REQUEST_TYPE_INVALID) {
    table = request_types;
  } else {
    return "UNKNOWN";
  }

  while (table->name) {
    if (table->type == base_type)
      return table->name;
    table++;
  }
  return "UNKNOWN";
}

static void decode_greybus_header(struct gb_operation_msg_hdr *hdr) {
  printf(BLUE "[GREYBUS] " RESET);
  printf("op=" GREEN "%s" RESET "(" YELLOW "0x%02x" RESET ") id=" GREEN
         "%u" RESET " size=" GREEN "%u" RESET,
         get_operation_name(hdr->type & ~0x80), hdr->type & ~0x80,
         le16toh(hdr->operation_id), le16toh(hdr->size));

  if (hdr->type & 0x80) {
    printf(" result=" GREEN "%u" RESET, hdr->result);
  }
  printf("\n");
}

static void decode_payload(uint8_t type, void *payload, size_t payload_size) {
  if (!payload_size)
    return;

  switch (type & ~0x80) {
  case GB_CONTROL_TYPE_VERSION: {
    if (type & 0x80) {
      struct gb_control_version_response *resp = payload;
      printf(BLUE "[PAYLOAD] " RESET "version: " GREEN "%u.%u" RESET "\n",
             resp->major, resp->minor);
    } else {
      struct gb_control_version_request *req = payload;
      printf(BLUE "[PAYLOAD] " RESET "version: " GREEN "%u.%u" RESET "\n",
             req->major, req->minor);
    }
    break;
  }
  default:
    if (payload_size > 0) {
      printf(BLUE "[PAYLOAD] " RESET GREEN "%zu" RESET " bytes\n",
             payload_size);
    }
    break;
  }
}

void _pr_dump(const char *fn, uint8_t *data, size_t len) {
  if (log_level < LL_VERBOSE)
    return;

  printf("\n%s:\n", fn);

  if (len >= sizeof(struct gb_operation_msg_hdr)) {
    struct gb_operation_msg_hdr *hdr = (struct gb_operation_msg_hdr *)data;
    decode_greybus_header(hdr);

    if (len > sizeof(struct gb_operation_msg_hdr)) {
      decode_payload(hdr->type, data + sizeof(struct gb_operation_msg_hdr),
                     len - sizeof(struct gb_operation_msg_hdr));
    }
  }

  printf(BLUE "[HEX] " RESET);
  for (int i = 0; i < len; i++) {
    printf(MAGENTA "%02x " RESET, data[i]);
  }
  printf("\n\n");
}
