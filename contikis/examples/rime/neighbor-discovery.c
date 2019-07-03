
/**
 * \file
 *         Neighbor discovery
 * \author
 *         Sudipta Saha
 */

#include "contiki.h"
#include "node-id.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime/rime.h"

#include <stdio.h>

static uint16_t num_pkt_sent=1;

/* This is the structure of broadcast messages. */
struct broadcast_message {
	uint8_t seqno;
	uint8_t array[46];		// Padding to make the packet length 64
							// To make it 64 byte
};

/* This structure holds information about neighbors. */
struct neighbor {

	struct neighbor *next;			// next pointer

	linkaddr_t addr;				// address of the neighbor
	uint16_t node_id;				// node id of the node
	int last_rssi;					// last known RSSI
	uint16_t last_lqi;				// Last known LQI
	int avg_rssi;					// Average RSSI, Note that average RSSI calculation is not okay.
	uint16_t avg_lqi, avg_prr;		// Average LQI and Average PRR
	uint16_t num_pkt_received;		// number of packets received so far
	uint8_t last_seqno;				// last known received sequence number

};

#define MAX_NEIGHBORS 40
MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);
static struct broadcast_conn broadcast;

PROCESS(broadcast_process, "Broadcast process");
PROCESS(print_process, "Print process");
AUTOSTART_PROCESSES(&broadcast_process,&print_process);
static uint8_t last_pkt_len=0;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	struct neighbor *n;
	struct broadcast_message *m;

	last_pkt_len = get_last_pkt_len();

	m = packetbuf_dataptr();
	// Check if the neighbor was already found earlier
	for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {

		if(linkaddr_cmp(&n->addr, from)) {
			// yes, found the neighbor
			n->num_pkt_received++;
			n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
			n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
			n->avg_rssi = ((int)(n->num_pkt_received-1)*n->avg_rssi + n->last_rssi)/(int)(n->num_pkt_received);
			n->avg_lqi = ((n->num_pkt_received-1)*n->avg_lqi + n->last_lqi)/n->num_pkt_received;
			n->last_seqno = m->seqno;

			break;
		}
	}

	if(n == NULL) {

		// No, so new neighbor
		n = memb_alloc(&neighbors_memb);
		n->num_pkt_received = n->avg_rssi = n->avg_lqi = n->avg_prr = 0;
		n->num_pkt_received++;
		n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
		n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
		n->avg_rssi = n->last_rssi;
		n->avg_lqi = n->last_lqi;
		n->last_seqno = m->seqno;

		if(n == NULL) {
			return;
		}

		linkaddr_copy(&n->addr, from);
		list_add(neighbors_list, n);
	}
}



static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(print_process, ev, data)
{
	static struct etimer et_print;
	struct neighbor *n;
	static uint16_t counter=1;

	PROCESS_BEGIN();

	while(1) {

		/* Send a broadcast every 16 - 32 seconds */
		etimer_set(&et_print, CLOCK_SECOND * 30);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_print));

//		printf("%5s %5s %5s %5s %5s %5s %5s %5s\n",
//				"Tsnd", "Cntr","Id","Neigh","Pno","LSno","Avrs","Avlq" );
		for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {

			printf(
					"%5u %5u %5u %5u %5u %5u %5d %5u\n",
					node_id,								// The node id of the current node
					counter,								// This is the value of the printing iteration no
					num_pkt_sent,							// The number of packet already sent by this node
					last_pkt_len,							// Packet length (from radio)
					n->addr.u8[0],							// Neighbors address (Node id)
					n->num_pkt_received,					// How many packets received from the neighbor
					n->avg_rssi,							// average RSSI received so far
					n->avg_lqi								// average LQI value received
				);
		}

		counter++;
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
	static struct etimer et;
	struct broadcast_message msg;

	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
	PROCESS_BEGIN();
	broadcast_open(&broadcast, 129, &broadcast_call);

	while(1) {

		/* Send a broadcast every 16 - 32 seconds */
		etimer_set(&et, CLOCK_SECOND * 16 + random_rand() % (CLOCK_SECOND * 16));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		msg.seqno = num_pkt_sent;
		packetbuf_copyfrom(&msg, sizeof(struct broadcast_message));
		broadcast_send(&broadcast);
		num_pkt_sent++;
	}

	PROCESS_END();
}
