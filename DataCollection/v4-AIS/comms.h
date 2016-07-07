#ifndef __COMMS_H__
#define __COMMS_H__

// need the SAMPLE_t structure
#include "sample.h"

#define __UDP_PORT__	4270
#define __BUF_SIZE__	1024

// callbacks for handling different commands
typedef void (*COMMSCB_f)(void *, const char *, char *);

// flags for comms
#define COMMS_FLAG_VALID_CLIENT		0x0001
#define COMMS_FLAG_SHORT_TIMEOUT	0x0010
#define COMMS_FLAG_QUIT				0x8000 

/* Initialize the Comms module.
 * <port> specifies the port to listen to. 0 = default __UDP_PORT__
 * returns 0 on success, -1 on error
 */
extern int COMMS_init (uint16_t port);

/* clean up */
extern void COMMS_close (void);

/* Install a callback.
 * When receive a message with the starting word matching <word>, 
 * the function <func> will be triggered, with the following format:
 * 		(*<func>)(<arg>, <word>, <msg>)
 * where <arg> is the specified <arg> to this function
 *       <word> is the matching first word in the received message
 *       <msg> is the remaining string the message sans the first word
 */
extern int COMMS_set_callback (const char * word, COMMSCB_f func, void * arg);

/* Send a message back to the client.
 * First word of the message is given in <word>
 * remaining part of the message is given in <msg>
 * The entire message (<word> + ' ' + <msg> + '\n\0') should not exceed __BUF_SIZE__
 * If there is no client, function returns -1
 * Else, the function returns number of bytes sent
 */
extern int COMMS_send_msg ( const char * word, const char * msg );

/* Send the captured sample <sample> back to the client.
 * This function will automatically break the samples into multiple messages
 * If there is no client, function returns -1
 * Else, the function returns number of messages sent
 */
extern int COMMS_send_sample ( SAMPLE_p sample );

/* Check comms msg
 *    <tout> is the amount of usec to wait for message before timeout 
 * Returns 0 if no message has been received, +ve if at least one message 
 * has been received and processed, -ve on error
 */
extern int COMMS_check_msg (uint32_t tout);

/* main comms loop 
 * <periodic> is a callback function to trigger whenever the comms loop timeout
 * The <periodic> callback will be triggered with
 * 		(*<periodic>)(<arg>, NULL, NULL)
 * where <arg> is the second argument passed in.
 */
extern int COMMS_main_loop (COMMSCB_f periodic, void * arg);

// Set a flag
extern void COMMS_set_flag (uint32_t f);

// Clear a flag
extern void COMMS_clr_flag (uint32_t f);

#endif // __COMMS_H__
