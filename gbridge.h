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

#ifndef _GBRIDGE_H_
#define _GBRIDGE_H_

#include <stdint.h>
#include <linux/types.h>
#include <sys/queue.h>

#define __packed  __attribute__((__packed__))

#include <greybus.h>
#include <greybus_protocols.h>
#include <gb_netlink.h>

#define SVC_CPORT		0
#define OP_RESPONSE		0x80

struct operation {
	struct gb_operation_msg_hdr *req;
	struct gb_operation_msg_hdr *resp;
	uint16_t cport_id;
	 TAILQ_ENTRY(operation) cnode;
};

typedef int (*greybus_handler_t) (struct operation *);

#define operation_to_request(op)	\
	(void *)((op)->req + 1)

#define operation_to_response(op)	\
	(void *)((op)->resp + 1)

#define gb_operation_msg_size(hdr)	\
	le16toh(((struct gb_operation_msg_hdr *)(hdr))->size)

int svc_init(void);
int svc_handler(struct operation *op);
int svc_send_intf_hotplug_event(uint8_t intf_id,
				uint32_t vendor_id,
				uint32_t product_id, uint64_t serial_number);

int greybus_init(void);
struct operation *greybus_alloc_operation(uint8_t type,
					  void *payload, size_t len);
int greybus_alloc_response(struct operation *op, size_t size);
int greybus_send_request(uint16_t cport_id, struct operation *op);
int greybus_handler(uint16_t cport_id, struct gb_operation_msg_hdr *hdr);

#endif /* _GBRIDGE_H_ */