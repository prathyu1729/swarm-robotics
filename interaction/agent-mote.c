/**
 * \file
 *			Multi-purpose multi-channel link measurement ................
 * \author
 *         Sudipta Saha < sudipta@comp.nus.edu.sg, sudipta.saha.22@gmail.com,>
 */

#include "contiki.h"
#include "../net/packetbuf.h"
#include "../net/packetbuf.h"
#include "token.h"
#include <string.h>
#include <stdio.h>


/*---------------------------------------------------------------------------*/
PROCESS(agent_mote, "Agent mote");
AUTOSTART_PROCESSES(&agent_mote);
/*---------------------------------------------------------------------------*/

static struct abc_conn abc_object;

static void
abc_recv(struct abc_conn *c)//, const rimeaddr_t *from)
{

	uint8_t *packet = packetbuf_dataptr();

	PRINTF_NORMAL("Received type %u %u\n",packet[0], packet[1]);

	// You have received a packet - 
	// So check what kind of packet it is
	// The first character will tell that

//	if(packet[0] == MCM_CNTRL  && chlm.state==STATE_STOPPED) {
//
//		if(packetbuf_attr(PACKETBUF_ATTR_CRC_OK)==128){
//
//			// If the packet is okay
//
//			leds_on(LEDS_RED);
//
//			// This means you have received a packet - now you need to
//			// check what kind of the packet it is -
//			// 0 type packets will be commands for conveying to your host
//			// 1 or any other tpe will be for internal purpose.
//			// in case of 0 you print it in serial line - so that host knows
//
//			leds_off(LEDS_RED);
//
//
//		}
//
//	}

}


static const struct abc_callbacks abc_call = {abc_recv,NULL};

PROCESS_THREAD(agent_mote, ev, data)
{

	PROCESS_EXITHANDLER();

	PROCESS_BEGIN();

//	cc2420_set_channel(CHANNEL_SYNC);
//	cc2420_set_txpower(CC2420_POWER_SYNC);
	SENSORS_ACTIVATE(button_sensor);



	abc_open(&abc_object, 130, &abc_call);

	PRINTF_NORMAL("Ready to start..........\n");

	//------------------------------------------------------------------------------------
	while(1){

		PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message ||
				((ev==sensors_event) && (data == &button_sensor)));

		if(ev == serial_line_event_message && data != NULL){

			leds_on(LEDS_RED);

			handle_serial_input((const char *) data);
			// If you see a serial like message communication from the host
			// This means you received a command from the host.
			// First see the command type - 
			// 1 type commands will be for communications
			// Any other type command will be for other purpose
			// The language has to be developed

			if ()
			packetbuf_copyfrom(chlm.packet_buf,4);

			if(abc_send(&chlm.abc)==0){

				PRINTF_NORMAL("\n-----------------------------------\n");
				PRINTF_NORMAL("Could not be sent ..................\n");

			}else {

				printf("Sent data : %u \n",1);

			}

			leds_off(LEDS_RED);

		}

		if((ev==sensors_event) && (data == &button_sensor)){

			// If button of the TB has been pressed.

			// Start and stop

		}

	}
	PROCESS_END();

}
