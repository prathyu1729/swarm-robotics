/*
 * token.h
 *
 *  Created on: May 30, 2015
 *      Author: sudipta
 */

#ifndef TOKEN_H_
#define TOKEN_H_

/*---------------------------------------------------------------------------*/
#if NODE_TYPE==NODE_TYPE_CONTROLLER
const char *token_next(const char *str, int *pos);
void handle_serial_input(const char *line);
#endif


#endif /* TOKEN_H_ */
