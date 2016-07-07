#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "rpi-gpio.h"

#define __UDP_PORT__	4270
#define __BUF_SIZE__	1024

#define FLAG_RANGING	0x01		// indicate ranging in progress
#define FLAG_QUIT		0x8000		// indicate that user has asked us to quit		

typedef struct __main_ctrl__ {
	RPI_CB_p rpi;				// R-Pi Control block from rpi-gpio
	int sd;						// UDP socket
	struct sockaddr_in client;	// Address of Remote Side
	char sbuf[__BUF_SIZE__];	// send buffer
	char rbuf[__BUF_SIZE__];	// receive buffer
	unsigned flags;				// flags
} MYCB_t, * MYCB_p;


#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif


/* Initialize the Control Block */
static void init (MYCB_p cb)
{
	struct sockaddr_in myaddr;
	int optval = 1;
	
	// initialize R-Pi
	cb->rpi = rpigpio_init();
	cb->rpi->sensors[0].enabled = 1;		// default enable sensor #0

	// listening socket
	cb->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cb->sd < 0) {
		perror("creating socket");
		exit(-1);
	}
	if (setsockopt(cb->sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		perror("setsockopt reuse");
		exit(-1);
	}
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(__UDP_PORT__);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(cb->sd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
		perror("binding socket");
		exit(-1);
	}
	
	// misc initialization
	memset(&(cb->client), 0, sizeof(cb->client));
	memset(cb->sbuf, 0, __BUF_SIZE__);
	memset(cb->rbuf, 0, __BUF_SIZE__);
}
	
/* clean up */
static void fini (MYCB_p cb)
{
	close(cb->sd);
	rpigpio_fini();
}

/* send configuration back to client */
static void send_cfg (MYCB_p cb) 
{
	char *p;	// end-of-string pointer
	int len, i;
	
	// inidcate message as config
	p = strcpy (cb->sbuf, "CONFIG ");
	p += strlen(cb->sbuf);
	
	// sensors settings
	for (i=0; i<NUM_SENSORS; i++) {
		len = sprintf (p, "SEN-%d=%c ", i, cb->rpi->sensors[i].enabled ? '1' : '0');
		p += len;
	}
	
	// timing settings
	len = sprintf (p, "T-RANGE=%d T-SAMPLE=%d T-NEXT=%d T-CYCLE=%d T-ADC=%d ",
		cb->rpi->usec_ranging / 1000,
		cb->rpi->usec_sampling / 1000,
		cb->rpi->usec_next / 1000,
		cb->rpi->usec_cycle / 1000,
		cb->rpi->usec_sample);
	p[len] = 0;
		
	// send
	if (sendto (cb->sd, cb->sbuf, strlen(cb->sbuf)+1, 0, (struct sockaddr *)&(cb->client), sizeof(cb->client)) < 0) {
		perror ("sendto");
		exit(-1);
	}
}

#define __SET_TIMING__(_SNAME,_SLEN,_FIELD,_SCALE) \
	if (strncmp(sp, _SNAME, _SLEN) == 0) { \
		_FIELD = (unsigned)(val * _SCALE); \
		debug("handle_setcfg(): setting %s to %u\n", _SNAME, _FIELD); \
	}

/* handle configuration settings */
static char * handle_setcfg (MYCB_p cb, char * sp)
{
	int sen, val;
	char *p, *ep;
	
	while (sp) {
		// skip continuous spaces
		while (*sp == ' ')
			sp++;
		if (strncmp(sp, "SEN-", 4) == 0) {
			// sensor setting
			p = strchr(sp, '=');
			if (!p) // no '=', stop parsing
				return (sp);
			sen = (int)(sp[4] - '0');			
			if (sen < NUM_SENSORS) {
				cb->rpi->sensors[sen].enabled = (p[1] == '1');
				debug("handle_setcfg(): %s sensor %d\n", (p[1] == '1') ? "enabling" : "disabling", sen);
			}
			sp = strchr(p+1, ' ');
			continue;
		}
		else if (strncmp(sp, "T-", 2) == 0) {
			// timing setting
			p = strchr(sp, '=');
			if (!p) // no '=', stop parsing
				return (sp);
			// get the time value first
			ep = strchr(p+1, ' ');
			if (ep)
				*ep = 0;
			val = atoi(p+1);
			if (ep)
				*ep = ' ';
			// determine which timing to set
			__SET_TIMING__("T-RANGE", 7, cb->rpi->usec_ranging, 1000);
			__SET_TIMING__("T-SAMPLE", 8, cb->rpi->usec_sampling, 1000);
			__SET_TIMING__("T-NEXT", 6, cb->rpi->usec_next, 1000);
			__SET_TIMING__("T-CYCLE", 7, cb->rpi->usec_cycle, 1000);
			__SET_TIMING__("T-ADC", 5, cb->rpi->usec_sample, 1);
			sp = ep;
			continue;
		}
		else
			// unknown: stop parsing
			break;
	}
	return (sp);
}

/* handle commands */
static int handle_cmd (MYCB_p cb)
{
	char * sp, * ep;
	int rlen, alen;
	
	// receive the UDP message
	alen = sizeof(cb->client);
	rlen = recvfrom(cb->sd, cb->rbuf, __BUF_SIZE__-1, 0, (struct sockaddr *)&(cb->client), &alen);
	if (rlen < 0) {
		perror("recvfrom");
		return (-1);
	}
	
	// process the UDP message
	for (sp=cb->rbuf; *sp; ) {
		while (*sp == ' ')
			sp ++;
		if (strncmp(sp, "QUIT", 4) == 0) {
			// QUIT received
			printf ("handle-cmd(): received QUIT.\n");
			cb->flags |= FLAG_QUIT;
			return (-1);
		}
		if (strncmp(sp, "GETCFG", 6) == 0) {
			// GETCFG received
			debug ("handle-cmd(): received GETCFG");
			send_cfg (cb);
			sp += 6;
			continue;
		}
		if (strncmp(sp, "SETCFG", 6) == 0) {
			// SETCFG received
			debug ("handle-cmd(): received SETCFG");
			sp = handle_setcfg(cb, sp);
			send_cfg (cb);
			continue;
		}
		if (strncmp(sp, "START", 5) == 0) {
			// START received
			debug ("handle-cmd(): received START");
			cb->flags |= FLAG_RANGING;
			sp += 5;
			continue;
		}
		if (strncmp(sp, "STOP", 4) == 0) {
			// STOP received
			debug ("handle-cmd(): received STOP");
			cb->flags &= ~FLAG_RANGING;
			sp += 4;
			continue;
		}
		// unknown command
		sp = strchr(sp, ' ');
		if (sp)
			sp += 1;
		else break;
	}
}

/* ranging procedure */
void handle_ranging (MYCB_p cb)
{
	debug("handle_ranging() -- not implemented yet!!!\n");
#if 0
	
#endif
}

/* main routine */
int main (int argc, char * argv[])
{
	MYCB_t cb;
	int ret = 0;

	// initialize
	init(&cb);
	
	// main loop: listen for command
	while (!(cb.flags & FLAG_QUIT)) {
		struct timeval tv;
		fd_set rfds;
		
		tv.tv_sec = (cb.flags & FLAG_RANGING) ? 0 : 5;		// if ranging we don't spend too much time in select()
		tv.tv_usec = 1;
		FD_ZERO(&rfds);
		FD_SET(cb.sd, &rfds);
		if (select(cb.sd + 1, &rfds, NULL, NULL, &tv) < 0) {
			perror("select");
			ret = -1;
			break;
		}
		if (FD_ISSET(cb.sd, &rfds)) {
			// receive command
			if (handle_cmd(&cb) < 0)
				break;
		}
		if (cb.flags & FLAG_RANGING)
			handle_ranging(&cb);
	}
	
	// finalize
	fini(&cb);
	return (ret);
}

