/*
 * token.c
 *
 *  Created on: May 30, 2015
 *      Author: sudipta
 */


#include "settings.h"
/*---------------------------------------------------------------------------*/
#if NODE_TYPE==NODE_TYPE_CONTROLLER || NODE_TYPE_VEHICLE_RECORDER

const char *token_next(const char *str, int *pos) {
	const char *ret;

	while (str[*pos] == ' ') {
		(*pos)++;
	}

	ret = str+*pos;

	/* Advance to the next space or end of string. */
	while (str[*pos] != ' ' && str[*pos] != '\0') {
		(*pos)++;
	}

	return ret;
}

void handle_serial_input(const char *line) {
	int pos = 0;
	const char *token;

	/* Consume the first token. */
	token = token_next(line, &pos);

	switch (token[0]) {
	case 'c':
		chlm.current_channel = atoi(token_next(line, &pos));
		printf("Setting current channel to %u.\n",chlm.current_channel);
		//radio_set_txpower(config.txpower);
		break;
	case 'p':
		chlm.current_power = atoi(token_next(line, &pos));
		printf("Setting current power to %u.\n",chlm.current_power);
		break;
	case 'd':
		chlm.current_data_packet_size = atoi(token_next(line, &pos));
		printf("Setting current data packet size to %u.\n",chlm.current_data_packet_size);
		break;

	default:
		printf("?\n");
		break;
	}
}

void print_observer_input(const char *line) {
	int pos = 0;
	const char *token;

	/* Consume the first token. */
	token = token_next(line, &pos);

	switch (token[0]) {
	case '1':printf("1\n");	// sender side human
		break;
	case '2':printf("2\n");	// sender side small vehicle
		break;
	case '3':printf("3\n");	// sender side medium vehicle
		break;
	case '4':printf("4\n"); // sender side large vehicle
		break;
	case '5':printf("5\n");	// receiver side human
		break;
	case '6':printf("6\n"); // receiver side small vehicle
		break;
	case '7':printf("7\n");	// receiver side medium vehicle
		break;
	case '8':printf("8\n");	// receiver side large vehicle
		break;

	case '15':printf("15\n");	// combination
		break;
	case '16':printf("16\n");	// combination
		break;
	case '17':printf("17\n");	// combination
		break;
	case '18':printf("18\n");	// combination
		break;

	case '25':printf("15\n");	// combination
		break;
	case '26':printf("16\n");	// combination
		break;
	case '27':printf("17\n");	// combination
		break;
	case '28':printf("18\n");	// combination
		break;

	case '35':printf("15\n");	// combination
		break;
	case '36':printf("16\n");	// combination
		break;
	case '37':printf("17\n");	// combination
		break;
	case '38':printf("18\n");	// combination
		break;

	case '45':printf("15\n");	// combination
		break;
	case '46':printf("16\n");	// combination
		break;
	case '47':printf("17\n");	// combination
		break;
	case '48':printf("18\n");	// combination
		break;

	default:
		printf("0\n");
		break;
	}
}

#endif
