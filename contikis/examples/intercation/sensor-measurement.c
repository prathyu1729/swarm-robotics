/**
 * \file
 *			Multi-purpose multi-channel link measurement ................
 * \author
 *         Sudipta Saha < sudipta@comp.nus.edu.sg, sudipta.saha.22@gmail.com,>
 */

#include "contiki.h"
#include "settings.h"
#include "net/packetbuf.h"
#include "easysen-sensors.h"
#include "repeate.h"
#include "file.h"
#include "token.h"
#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------------------------------------------------------------
// Initializations
void init(void);

//--------------------------------------------------------
// We need the following frequency hopping code -
// only if we need to do frequency hopping

void init(void) {

	int i;

	chlm.local_iteration_count=0;
	chlm.state=STATE_STOPPED;
	chlm.data_id=1;
	chlm.received_message_flag=0;
	rtimer_init();

	cc2420_set_channel(CHANNEL_SYNC);
	cc2420_set_txpower(CC2420_POWER_SYNC);

	leds_off(LEDS_ALL);
	chlm.observer_input = '0';
}

/*---------------------------------------------------------------------------*/
PROCESS(measure_channel, "Channel measurement");
AUTOSTART_PROCESSES(&measure_channel);
/*---------------------------------------------------------------------------*/

static void
abc_recv(struct abc_conn *c)//, const rimeaddr_t *from)
{

	uint8_t *packet = packetbuf_dataptr();

	//PRINTF_NORMAL("Received type %u %u\n",packet[0], packet[1]);

	if(packet[0] == MCM_CNTRL  && chlm.state==STATE_STOPPED) {
		if(packetbuf_attr(PACKETBUF_ATTR_CRC_OK)==128){
			printf("Setting the channel to %u, power to level %u, and data size to %u\n",packet[1],packet[2],packet[3]);
			leds_on(LEDS_RED);
			chlm.current_channel = packet[1];
			chlm.current_power = packet[2];
			chlm.current_data_packet_size = packet[3];
			leds_off(LEDS_RED);
		}

	}

	if(packet[0]==MCM_SYNC && chlm.state==STATE_STOPPED){
		if(packetbuf_attr(PACKETBUF_ATTR_CRC_OK)==128){
			uint8_t round_count_lower = packet[1];
			uint8_t round_count_upper = packet[2];
			uint8_t packet_id = packet[3];
			chlm.round_count = (round_count_lower)+(100*round_count_upper);

			//		chlm.one_round.round_number = chlm.round_count;

#if NODE_TYPE == NODE_TYPE_SENDER
			//		if(is_sender()){
			sender_initiates_sync(packet_id);

#elif NODE_TYPE == NODE_TYPE_RECEIVER_PACKET || NODE_TYPE == NODE_TYPE_RECEIVER_RSSI || NODE_TYPE == NODE_TYPE_RECEIVER_SENSOR || NODE_TYPE == NODE_TYPE_VEHICLE_RECORDER
			//		}else if(is_receiver()){
			receiver_initiates_sync(packet_id);
			//		}
#endif
		}

	} else if(packet[0] == MCM_DATA) {

		//printf("packet id = %u", packet_id);
		if(packetbuf_attr(PACKETBUF_ATTR_CRC_OK)==128)
			chlm.record[packet[1]]=2;
		else if(packetbuf_attr(PACKETBUF_ATTR_CRC_OK)==0){
			//-----------------------------------------------------------------------
			// Since the packet is corrupted, we
			// Cannot rely on the id bit
			// So, we take the internal counter
			// which is being incremented at each packet sending interval.
			chlm.record[(chlm.num_packet_sent-1)]=1;
		}

		chlm.received_message_flag=1;

		//--------------------------------------------------------------------------------------
		// Use the following for scheme 1
		//--------------------------------------------------------------------------------------

		if(chlm.local_iteration_count == 0){
			//------------------------------------------------------------------------------
			// If this is the very first iteration of a certain round
			// We record the round number
			//			chlm.one_round.round_number = chlm.round_count;
			//			PRINTF_FILE_WRITE("Wrote : %4d\n",
			//					chlm.one_round.round_number);

		}

#if NODE_TYPE == NODE_TYPE_RECEIVER_PACKET
		// For every iteration in a round
		// We record the channel_rssi values for each channel
		//chlm.one_round.round_record[chlm.local_iteration_count][0] = 2;

		//		PRINTF_NORMAL("%4u %4u %4u\n", chlm.round_count, chlm.local_iteration_count,
		//				chlm.one_round.round_record[chlm.local_iteration_count][0]);

		//		PRINTF_FILE_WRITE("Iteration number %2d: wrote : %2d\n",
		//				chlm.local_iteration_count,
		//				chlm.one_round.round_record[0]);

		// Now after a round ends we can directly write the structure one_round with its size =
		//}
#endif

	}

}


static const struct abc_callbacks abc_call = {abc_recv,NULL};

PROCESS_THREAD(measure_channel, ev, data)
{

	PROCESS_EXITHANDLER(
			abc_close(&chlm.abc);
	);

	PROCESS_BEGIN();

	cc2420_set_channel(CHANNEL_SYNC);
	cc2420_set_txpower(CC2420_POWER_SYNC);
	chlm.current_channel = CHANNEL_TEST;
	chlm.current_power = CC2420_POWER_TEST;
	chlm.current_data_packet_size = DATA_SIZE;
	SENSORS_ACTIVATE(button_sensor);


	//	/*--------------------------------------------------*/
	//	// Initialize the communication driver
	//	raw_comm_driver.init();
	//
	//	//---------------------------------------------------
	//	raw_comm_driver.receive = raw_receive;
	//	raw_comm_driver.packet_len = PACKET_SIZE;
	//	raw_comm_driver.num_rssi_sample = NUM_RSSI_SAMPLES;
	//	raw_comm_driver.rssi_sample_interval =
	//			(raw_comm_driver.packet_len+1)/raw_comm_driver.num_rssi_sample;
	//	raw_comm_driver.packet_buffer_send[0] =
	//			raw_comm_driver.packet_len;
	//	/*--------------------------------------------------*/
	//
	//	//CC2420_DISABLE_FIFOP_INT();
	//	/*--------------------------------------------------*/
	//

#if NODE_TYPE == NODE_TYPE_RECEIVER_SENSOR
	activate_sensor();
#endif

	init();
	chlm.round_count=0;
	int index=0;
	for(index=0;index<128;index++){
		chlm.packet_buf[index]=index;
	}

	abc_open(&chlm.abc, 130, &abc_call);
	//	packetbuf_copyfrom(chlm.packet_buf,DATA_SIZE);
	// 	Note:
	//-------------------------------------------------------------
	//	Please reserve if the size is grater than 278500 Bytes
	// 	Wired : sometimes reserve works fine - sometime not
	// 	When it does not work - it says that it has reserved 65500 pages -
	// 	Which is impossible.

	//MAX_FILE_SIZE;


	//--------------------------------------------------------------------------------
	//Both receiver and sender should wait here
	//	PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message ||
	//			((ev==sensors_event) && (data == &button_sensor)));

	//PRINTF_OUTPUT("\n\n\n\n\n\n\n");

#if NODE_TYPE == NODE_TYPE_CONTROLLER
	PRINTF_OUTPUT("CONTROLLER\n");
	PRINTF_NORMAL("This is the CONTROLLER, Node id is %d:%d. \n",
			rimeaddr_node_addr.u8[0],
			rimeaddr_node_addr.u8[1]);

#elif NODE_TYPE == NODE_TYPE_SENDER
	PRINTF_OUTPUT("SENDER\n");
	PRINTF_NORMAL("This is the SENDER, Node id is %d:%d \n",
			rimeaddr_node_addr.u8[0],
			rimeaddr_node_addr.u8[1]);
#elif NODE_TYPE == NODE_TYPE_RECEIVER_PACKET
	PRINTF_OUTPUT("RECEIVER (packet) : PACKET detector and classifier\n");
	PRINTF_OUTPUT("RECEIVER (packet) : each line following : <round number> <iteration number> <x>, where x may be 0, 1 or 2\n");
	PRINTF_OUTPUT("RECEIVER (packet) : 0 for lost packet, 1 for detected packet with corrupted CRC, 2 for correct packet\n");

	PRINTF_NORMAL("This is the RF packet receiver, Node id is %d:%d \n",
			rimeaddr_node_addr.u8[0],
			rimeaddr_node_addr.u8[1]);
#elif NODE_TYPE == NODE_TYPE_RECEIVER_RSSI
	PRINTF_OUTPUT("RECEIVER (rssi) : Records the RSSI values starting from a few milliseconds before to the end of the reception of an RF packet\n");
	PRINTF_OUTPUT("RECEIVER (rssi) : each line following : <round number> <iteration number> <x> .... <x>, 26 RSSI values\n");

	PRINTF_NORMAL("This is the RF RSSI recorder %d:%d \n",
			rimeaddr_node_addr.u8[0],
			rimeaddr_node_addr.u8[1]);
#elif NODE_TYPE == NODE_TYPE_RECEIVER_SENSOR

	PRINTF_OUTPUT("RECEIVER (sensor) : Records the sensor readings starting from a few milliseconds before to the end of the reception of an RF packet\n");
	PRINTF_OUTPUT("RECEIVER (sensor) : each line following : <round number> <iteration number> <x> .... <x>, 250 sensor readings\n");

	PRINTF_NORMAL("This is the sensor recorder %d:%d \n",
			rimeaddr_node_addr.u8[0],
			rimeaddr_node_addr.u8[1]);

#if SENSOR_TYPE == SBT80_VISUAL_LIGHT_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 Visual light readings\n");
	PRINTF_NORMAL("Records SBT80 visual light\n");

#elif SENSOR_TYPE == SBT80_SOUND_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 Sound readings\n");
	PRINTF_NORMAL("Records SBT80 sound \n");

#elif SENSOR_TYPE == SBT80_IR_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 IR readings\n");
	PRINTF_NORMAL("Records SBT80 IR\n");

#elif SENSOR_TYPE == SBT80_TEMP_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 Temperature readings\n");
	PRINTF_NORMAL("Records SBT80 temperature\n");
#elif SENSOR_TYPE == TELOSB_VISUAL_LIGHT_PAR_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : TelosB visual light 1 readings\n");
	PRINTF_NORMAL("Records telos B visual light PAR\n");
#elif SENSOR_TYPE == TELOSB_VISUAL_LIGHT_TAR_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : TelosB visual light 2 readings\n");
	PRINTF_NORMAL("Records telos B visual light TAR\n\n");
#elif SENSOR_TYPE == SBT80_ACC_SENSOR_X
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 Accelerometer (x-axis) readings\n");
	PRINTF_NORMAL("Records SBT80 accelerometer (only X)\n");
#elif SENSOR_TYPE == SBT80_MAG_SENSOR
	PRINTF_OUTPUT("RECEIVER (sensor) : SBT80 Magnetometer (x-axis) readings\n");
	PRINTF_NORMAL("Records SBT80 magnetometer (only X)\n");
#endif

#endif


	PRINTF_NORMAL("Ready to start..........\n");

	//------------------------------------------------------------------------------------
	while(1){

		//--------------------------------------------------------------------------------
		// The receiver starts waiting here

		PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message ||
				((ev==sensors_event) && (data == &button_sensor)));

#if NODE_TYPE == NODE_TYPE_CONTROLLER

		if(ev == serial_line_event_message && data != NULL){

			leds_on(LEDS_RED);

			handle_serial_input((const char *) data);

			// The current_channel and the current_power has been already set
			// Send a broadcast control message to everyone - to set things

			chlm.packet_buf[0] = MCM_CNTRL;
			chlm.packet_buf[1] = chlm.current_channel;
			chlm.packet_buf[2] = chlm.current_power;
			chlm.packet_buf[3] = chlm.current_data_packet_size;
			packetbuf_copyfrom(chlm.packet_buf,4);

			if(abc_send(&chlm.abc)==0){

				PRINTF_NORMAL("\n-----------------------------------\n");
				PRINTF_NORMAL("Could not be sent ..................\n");

			}else {

				printf("Channel, power, data size setting instructions are sent....\n");
				printf("Sent data : %u %u %u %u\n",chlm.packet_buf[0],chlm.packet_buf[1],chlm.packet_buf[2],chlm.packet_buf[3]);

			}

			leds_off(LEDS_RED);

		}

		if((ev==sensors_event) && (data == &button_sensor) && chlm.state == STATE_STOPPED){

			controller_initiate_sync();
			//printf("Controller - starting the loop in two seconds.....\n");

		}
#elif NODE_TYPE==NODE_TYPE_VEHICLE_RECORDER

		if(ev == serial_line_event_message && data != NULL){

			leds_on(LEDS_RED);

			chlm.observer_input = (const char *) data;

			leds_off(LEDS_RED);

		}
		//--------------------------------------------------------------------------------
#endif
	}
	PROCESS_END();

}
