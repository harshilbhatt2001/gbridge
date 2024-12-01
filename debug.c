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

static const struct operation_name control_types[];
static const struct operation_name gpio_types[];
static const struct operation_name i2c_types[];
static const struct operation_name pwm_types[];
static const struct operation_name spi_types[];
static const struct operation_name uart_types[];
static const struct operation_name sdio_types[];

struct protocol_mapping {
  uint8_t protocol;
  const struct operation_name *types;
  const char *name;
};

static const struct protocol_mapping protocol_mappings[] = {
    {GREYBUS_PROTOCOL_CONTROL, control_types, "CONTROL"},
    {GREYBUS_PROTOCOL_GPIO, gpio_types, "GPIO"},
    {GREYBUS_PROTOCOL_I2C, i2c_types, "I2C"},
    {GREYBUS_PROTOCOL_UART, uart_types, "UART"},
    {GREYBUS_PROTOCOL_PWM, pwm_types, "PWM"},
    {GREYBUS_PROTOCOL_SPI, spi_types, "SPI"},
    {GREYBUS_PROTOCOL_SDIO, sdio_types, "SDIO"},
    {0, NULL}};

static const struct operation_name gpio_types[] = {
    {GB_GPIO_TYPE_LINE_COUNT, "LINE_COUNT"},
    {GB_GPIO_TYPE_ACTIVATE, "ACTIVATE"},
    {GB_GPIO_TYPE_DEACTIVATE, "DEACTIVATE"},
    {GB_GPIO_TYPE_GET_DIRECTION, "GET_DIRECTION"},
    {GB_GPIO_TYPE_DIRECTION_IN, "DIRECTION_IN"},
    {GB_GPIO_TYPE_DIRECTION_OUT, "DIRECTION_OUT"},
    {GB_GPIO_TYPE_GET_VALUE, "GET_VALUE"},
    {GB_GPIO_TYPE_SET_VALUE, "SET_VALUE"},
    {GB_GPIO_TYPE_SET_DEBOUNCE, "SET_DEBOUNCE"},
    {GB_GPIO_TYPE_IRQ_TYPE, "IRQ_TYPE"},
    {GB_GPIO_TYPE_IRQ_MASK, "IRQ_MASK"},
    {GB_GPIO_TYPE_IRQ_UNMASK, "IRQ_UNMASK"},
    {GB_GPIO_TYPE_IRQ_EVENT, "IRQ_EVENT"},
    {0, NULL}};

static const struct operation_name i2c_types[] = {
    {GB_I2C_TYPE_FUNCTIONALITY, "FUNCTIONALITY"},
    {GB_I2C_TYPE_TRANSFER, "TRANSFER"},
    {0, NULL}};

static const struct operation_name pwm_types[] = {
    {GB_PWM_TYPE_PWM_COUNT, "PWM_COUNT"},   {GB_PWM_TYPE_ACTIVATE, "ACTIVATE"},
    {GB_PWM_TYPE_DEACTIVATE, "DEACTIVATE"}, {GB_PWM_TYPE_CONFIG, "CONFIG"},
    {GB_PWM_TYPE_POLARITY, "POLARITY"},     {GB_PWM_TYPE_ENABLE, "ENABLE"},
    {GB_PWM_TYPE_DISABLE, "DISABLE"},       {0, NULL}};

static const struct operation_name spi_types[] = {
    {GB_SPI_TYPE_MASTER_CONFIG, "MASTER_CONFIG"},
    {GB_SPI_TYPE_DEVICE_CONFIG, "DEVICE_CONFIG"},
    {GB_SPI_TYPE_TRANSFER, "TRANSFER"},
    {0, NULL}};

static const struct operation_name uart_types[] = {
    {GB_UART_TYPE_SEND_DATA, "SEND_DATA"},
    {GB_UART_TYPE_RECEIVE_DATA, "RECEIVE_DATA"},
    {GB_UART_TYPE_SET_LINE_CODING, "SET_LINE_CODING"},
    {GB_UART_TYPE_SET_CONTROL_LINE_STATE, "SET_CONTROL_LINE_STATE"},
    {GB_UART_TYPE_SEND_BREAK, "SEND_BREAK"},
    {GB_UART_TYPE_SERIAL_STATE, "SERIAL_STATE"},
    {GB_UART_TYPE_RECEIVE_CREDITS, "RECEIVE_CREDITS"},
    {GB_UART_TYPE_FLUSH_FIFOS, "FLUSH_FIFOS"},
    {0, NULL}};

static const struct operation_name sdio_types[] = {
    {GB_SDIO_TYPE_GET_CAPABILITIES, "GET_CAPABILITIES"},
    {GB_SDIO_TYPE_SET_IOS, "SET_IOS"},
    {GB_SDIO_TYPE_COMMAND, "COMMAND"},
    {GB_SDIO_TYPE_TRANSFER, "TRANSFER"},
    {GB_SDIO_TYPE_EVENT, "EVENT"},
    {0, NULL}};

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
    {GB_CONTROL_TYPE_BUNDLE_SUSPEND, "BUNDLE_SUSPEND"},
    {GB_CONTROL_TYPE_BUNDLE_RESUME, "BUNDLE_RESUME"},
    {GB_CONTROL_TYPE_BUNDLE_DEACTIVATE, "BUNDLE_DEACTIVATE"},
    {GB_CONTROL_TYPE_BUNDLE_ACTIVATE, "BUNDLE_ACTIVATE"},
    {GB_CONTROL_TYPE_INTF_SUSPEND_PREPARE, "INTF_SUSPEND_PREPARE"},
    {GB_CONTROL_TYPE_INTF_DEACTIVATE_PREPARE, "INTF_DEACTIVATE_PREPARE"},
    {GB_CONTROL_TYPE_INTF_HIBERNATE_ABORT, "INTF_HIBERNATE_ABORT"},
    {0, NULL}};

// static const struct operation_name request_types[] = {
//    {GB_REQUEST_TYPE_CPORT_SHUTDOWN, "CPORT_SHUTDOWN"},
//    {GB_REQUEST_TYPE_INVALID, "INVALID"},
//    {0, NULL}
// };

static uint8_t infer_protocol(uint8_t type) {
  type &= ~0x80;

  if (type >= GB_CONTROL_TYPE_VERSION &&
      type <= GB_CONTROL_TYPE_INTF_HIBERNATE_ABORT)
    return GREYBUS_PROTOCOL_CONTROL;
  if (type >= GB_GPIO_TYPE_LINE_COUNT && type <= GB_GPIO_TYPE_IRQ_EVENT)
    return GREYBUS_PROTOCOL_GPIO;
  if (type >= GB_I2C_TYPE_FUNCTIONALITY && type <= GB_I2C_TYPE_TRANSFER)
    return GREYBUS_PROTOCOL_I2C;
  if (type >= GB_UART_TYPE_SEND_DATA && type <= GB_UART_TYPE_FLUSH_FIFOS)
    return GREYBUS_PROTOCOL_UART;
  if (type >= GB_PWM_TYPE_PWM_COUNT && type <= GB_PWM_TYPE_DISABLE)
    return GREYBUS_PROTOCOL_PWM;
  if (type >= GB_SPI_TYPE_MASTER_CONFIG && type <= GB_SPI_TYPE_TRANSFER)
    return GREYBUS_PROTOCOL_SPI;
  if (type >= GB_SDIO_TYPE_GET_CAPABILITIES && type <= GB_SDIO_TYPE_EVENT)
    return GREYBUS_PROTOCOL_SDIO;

  return GREYBUS_PROTOCOL_CONTROL;
}

static const struct operation_name *get_operation_table(uint8_t protocol) {
  for (int i = 0; protocol_mappings[i].types != NULL; i++) {
    if (protocol_mappings[i].protocol == protocol) {
      return protocol_mappings[i].types;
    }
  }
  return NULL;
}

static const char *get_protocol_name(uint8_t protocol) {
  for (int i = 0; protocol_mappings[i].types != NULL; i++) {
    if (protocol_mappings[i].protocol == protocol) {
      return protocol_mappings[i].name;
    }
  }
  return "UNKNOWN";
}

static const char *get_operation_name(uint8_t protocol, uint8_t type) {
  const struct operation_name *table = get_operation_table(protocol);
  if (!table)
    return "UNKNOWN";

  for (int i = 0; table[i].name != NULL; i++) {
    if (table[i].type == (type & ~0x80)) {
      return table[i].name;
    }
  }
  return "UNKNOWN";
}

static void decode_greybus_header(struct gb_operation_msg_hdr *hdr,
                                  uint8_t protocol) {
  printf(BLUE "[GREYBUS] " RESET);
  printf("proto=" GREEN "%s" RESET " op=" GREEN "%s" RESET "(" YELLOW
         "0x%02x" RESET ") id=" GREEN "%u" RESET " size=" GREEN "%u" RESET,
         get_protocol_name(protocol), get_operation_name(protocol, hdr->type),
         hdr->type & ~0x80, le16toh(hdr->operation_id), le16toh(hdr->size));

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
    uint8_t protocol = infer_protocol(hdr->type);
    decode_greybus_header(hdr, protocol);

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
