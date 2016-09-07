#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "comms.h"

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif

// callback storage
typedef struct __comms_callback__ {
	char word[32];				// first word in the mesg to trigger this callback
	COMMSCB_f func; 			// function to trigger
	void * arg;					// argument to passback
} COMMSCB_t, * COMMSCB_p;
#define MAX_CALLBACK 16

// communications structure to store relevant parameters
typedef struct __comms__ {
	uint32_t flags;				// IPC flags
	int sd;						// UDP socket
	struct sockaddr_in client;	// Address of Remote Side
	char sbuf[__BUF_SIZE__];	// send buffer
	char rbuf[__BUF_SIZE__];	// receive buffer
	COMMSCB_t cb[MAX_CALLBACK];	// callbacks array
} COMMS_t, * COMMS_p;

static COMMS_p comms = NULL;

/* Initialize the Comms module.
 * <port> specifies the port to listen to. 0 = default __UDP_PORT__
 * returns 0 on success, -1 on error
 */
int COMMS_init (uint16_t port)
{
	struct sockaddr_in myaddr;
	int optval = 1;
	int i;

	if (!comms) {
		comms = (COMMS_p)calloc(1, sizeof(COMMS_t));
		if (port == 0)
			port = __UDP_PORT__;
	
		// listening socket
		comms->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (comms->sd < 0) {
			perror("creating socket");
			free(comms);
			comms = NULL;
			return (-1);
		}
		if (setsockopt(comms->sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
			perror("setsockopt reuse");
			free(comms);
			comms = NULL;
			return (-2);			
		}
		memset(&myaddr, 0, sizeof(myaddr));
		myaddr.sin_family = AF_INET;
		myaddr.sin_port = htons(port);
		myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(comms->sd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
			perror("binding socket");
			free(comms);
			comms = NULL;
			return (-3);
		}
		
		// misc initialization
		memset(&(comms->client), 0, sizeof(comms->client));
		memset(comms->sbuf, 0, __BUF_SIZE__);
		memset(comms->rbuf, 0, __BUF_SIZE__);
		comms->flags = 0x00;
		for (i=0; i<MAX_CALLBACK; i++)
			comms->cb[i].func = NULL;
		
		debug("COMMS: initialized to use port %d\n", port);
	}
	return (0);
}
	
/* clean up */
void COMMS_close (void)
{
	if (comms) {
		close(comms->sd);
		free (comms);
		comms = NULL;
	}
}

/* Install a callback.
 * When receive a message with the starting word matching <word>, 
 * the function <func> will be triggered, with the following format:
 * 		(*<func>)(<arg>, <word>, <msg>)
 * where <arg> is the specified <arg> to this function
 *       <word> is the matching first word in the received message
 *       <msg> is the remaining string the message sans the first word
 */
int COMMS_set_callback (const char * word, COMMSCB_f func, void * arg)
{
	COMMSCB_p cb;
	
	if (!comms)
		return (-1);
	// find the empty callback slot
	for (cb=comms->cb; cb->func; cb++);
	cb->func = func;
	cb->arg = arg;
	strncpy(cb->word, word, 31);

	debug("COMMS: added callabck for %s\n", word);

	return (0);
}

#define __SEND_MSG__(STRLEN) \
	if (sendto (comms->sd, comms->sbuf, STRLEN, 0, (struct sockaddr *)&(comms->client), sizeof(comms->client)) < 0) { \
		perror ("sendto"); \
		return (-2); \
	}

/* Send a message back to the client.
 * First word of the message is given in <word>
 * remaining part of the message is given in <msg>
 * The entire message (<word> + ' ' + <msg> + '\0') should not exceed __BUF_SIZE__
 * If there is no client, function returns -1
 * Else, the function returns number of bytes sent
 */
int COMMS_send_msg ( const char * word, const char * msg )
{
	int n;
	
	if (!comms || !(comms->flags & COMMS_FLAG_VALID_CLIENT))
		return (-1);
	n = sprintf(comms->sbuf, "%s %s", word, msg);
	__SEND_MSG__(n+1);

	debug("COMMS: message sent: %s\n", comms->sbuf);

	return (n+1);
}	

/* Send the captured sample <sample> back to the client.
 * This function will automatically break the samples into multiple messages
 * If there is no client, function returns -1
 * Else, the function returns number of messages sent
 */
int COMMS_send_sample ( SAMPLE_p sample )
{
	uint32_t tmp;
	int i, n, m;
	
	if (!comms || !(comms->flags & COMMS_FLAG_VALID_CLIENT))
		return (-1);
	
	// send the header first
	n = sprintf(comms->sbuf, "SAMPLE SETUP=%d NUM=%d START=%u", sample->setup, sample->num, sample->start);
	__SEND_MSG__(n+1);
	
	// now start sending the sample points in pair
	for (i=m=0; i<sample->num; m++) {
		for (n=sprintf(comms->sbuf, "DATA-%02d ", m); n<__BUF_SIZE__-5; ) {
			comms->sbuf[n++] = (char) ((sample->t_ofs[i] & 0xff00) >> 8);
			comms->sbuf[n++] = (char) (sample->t_ofs[i] & 0x00ff);
			comms->sbuf[n++] = (char) ((sample->val[i] & 0xff00) >> 8);
			comms->sbuf[n++] = (char) (sample->val[i] & 0x00ff);
			if (++i >= sample->num)
				break;
		}
		__SEND_MSG__(n);
		debug("COMMS: sending DATA-%02d (len=%d)...\n", m, n);
	}
	return (m+1);
}

/* handle received message */
static int COMMS_receive_msg (void)
{
	COMMSCB_p cb;
	char * sp, * ep, * mp;
	int rlen, alen;
#ifdef __DEBUG__
	char addr[64];
#endif
	
	// receive the UDP message
	alen = sizeof(comms->client);
	rlen = recvfrom(comms->sd, comms->rbuf, __BUF_SIZE__-1, 0, (struct sockaddr *)&(comms->client), &alen);
	if (rlen < 0) {
		perror("recvfrom");
		return (-1);
	}
	comms->flags |= COMMS_FLAG_VALID_CLIENT;

#ifdef __DEBUG__
	inet_ntop(AF_INET, &(comms->client.sin_addr), addr, 64);
	debug("COMMS: received message from %s:%d -> [%d] %s\n", addr, ntohs(comms->client.sin_port), rlen, comms->rbuf);
#endif
	
	// process the UDP message
	for (sp=comms->rbuf; (sp-comms->rbuf) < rlen; ) {
		// get to first word
		if ((*sp == ' ') || (*sp == '\n') || (*sp == '\0')) {
			sp ++;
			continue;
		}
		// find end of msg
		for (ep=sp+1; (*ep != '\n') && (*ep != '\0'); ep++);
		// check for callback to trigger
		*ep = 0;
		for (cb=comms->cb; cb->func; cb++) {
			if (strncmp(sp, cb->word, strlen(cb->word)) == 0) {
				// find separating ' '
				for (mp=sp+1; (*mp != ' ') && (mp < ep); mp++); 
				*mp = '\0';
				// Trigger callback
				(cb->func)(cb->arg, sp, mp+1);
				break;
			}
		}
		if (cb->func == NULL)
			// unknown command:
			fprintf(stderr, "Unhandled UDP MSG: %s\n", sp);
		sp = ep+1;
	}
	return (0);
}

/* Check comms msg
 *    <tout> is the amount of usec to wait for message before timeout 
 * Returns 0 if no message has been received, +ve if at least one message 
 * has been received and processed, -ve on error
 */
int COMMS_check_msg (uint32_t tout)
{
	struct timeval tv;
	fd_set rfds;
	tv.tv_sec = tout / 1000000;
	tv.tv_usec = tout % 1000000;
	FD_ZERO(&rfds);
	FD_SET(comms->sd, &rfds);
	if (select(comms->sd + 1, &rfds, NULL, NULL, &tv) < 0) {
		perror("select");
		return (-1);
	}
	if (FD_ISSET(comms->sd, &rfds))
		// receive command
		return (COMMS_receive_msg() < 0) ? -2 : 1;
	return (0);
}


/* main comms loop 
 * <periodic> is a callback function to trigger whenever the comms loop timeout
 * The <periodic> callback will be triggered with
 * 		(*<periodic>)(<arg>, NULL, NULL)
 * where <arg> is the second argument passed in.
 */
int COMMS_main_loop (COMMSCB_f periodic, void * arg)
{
	// main loop: listen for command
	while (!(comms->flags & COMMS_FLAG_QUIT)) {
		if (COMMS_check_msg ((comms->flags & COMMS_FLAG_SHORT_TIMEOUT) ? 1 : 50000) < 0)
			return (-1);
		if (periodic)
			(periodic)(arg, NULL, NULL);
	}
	debug("COMMS: exiting main loop ....\n");
	return (0);
}

// Set a flag
void COMMS_set_flag (uint32_t f)
{
	if (comms)
		comms->flags |= f;
}

// Clear a flag
void COMMS_clr_flag (uint32_t f)
{
	if (comms)
		comms->flags &= ~f;
}
