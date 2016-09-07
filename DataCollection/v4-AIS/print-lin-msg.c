/*
	Convenient APIs for Data Collection Circuit
*/
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef __DEBUG__
#define debug(fmt...)	fprintf(stderr, ##fmt)
#else
#define debug(fmt...)
#endif


static char __uart[1024][16];
static int __uart_ptr = 0;

/* Initialize the LIN interface
 */
int LIN_init (void)
{
	int i;
	
	__uart_ptr = 0;
	for (i=0; i<1024; i++) 
		__uart[i][0] = 0;
	return (0);
}

/* Close the LIN interface
 */
void LIN_close (void)
{
	int i;
	for (i=0; (i<1024) && (__uart[i][0]); i++) 
		printf ("%s\n", __uart[i]);
}

#define __ADDMSG(m) \
	strcpy(__uart[__uart_ptr++], m)

#define __LIN_SET_BRK() \
	strcpy(__uart[__uart_ptr++], "--BRK--")

#define __LIN_SET_TX() \
	strcpy(__uart[__uart_ptr++], "--TX--")

#define __LIN_SET_RX() \
	strcpy(__uart[__uart_ptr++], "--TX--")
	
#define __LIN_TX_BYTE(B) \
	sprintf(__uart[__uart_ptr++], "0x%2x", (unsigned)(B))
	


// the PID look up table
static unsigned char __lin_pid_lut__[64] = {
	0x80, 0xC1, 0x42, 0x03, 0xC4, 0x85, 0x06, 0x47,
	0x08, 0x49, 0xCA, 0x8B, 0x4C, 0x0D, 0x8E, 0xCF,
	0x50, 0x11, 0x92, 0xD3, 0x14, 0x55, 0xD6, 0x97,
	0xD8, 0x99, 0x1A, 0x5B, 0x9C, 0xDD, 0x5E, 0x1F,
	0x20, 0x61, 0xE2, 0xA3, 0x64, 0x25, 0xA6, 0xE7,
	0xA8, 0xE9, 0x6A, 0x2B, 0xEC, 0xAD, 0x2E, 0x6F,
	0xF0, 0xB1, 0x32, 0x73, 0xB4, 0xF5, 0x76, 0x37,
	0x78, 0x39, 0xBA, 0xFB, 0x3C, 0x7D, 0xFE, 0xBF
};

// Transmit a frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes
//	<data> is the array of bytes to send
//	Checksum will be calculated by this function
void LIN_send_frame (unsigned char id, int len, unsigned char * data )
{
	unsigned char pid, sync=0x55;
	int byte, r;
	uint32_t chksum, t;
	
	// the LIN protocol specifies a minimum 13-bit long dorminant pulse as a break signal,
	// followed by the start byte 0x55 to precede all frames.  
	// To get 13-bit, we have two methods:
	// (A) generate it using a dedicated GPIO pin (we use GPIO18 here)
	// (B) set the baudrate to 9600 = this will generate a 16-bit long break pulse
	
	// break the line -- using Method A
	__LIN_SET_BRK();
	
	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	for (byte=0; byte<len; byte++) 
		chksum += data[byte];
	chksum = ~(chksum + (chksum >> 8)) & 0xFF;

	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	__LIN_SET_TX();
	
	// send sync field and the frame and then chksum
	__LIN_TX_BYTE(sync);
	__LIN_TX_BYTE(pid);
	for (byte=0; byte<len; byte++) {
		__LIN_TX_BYTE(data[byte]);
	}
	__LIN_TX_BYTE(chksum);
	__LIN_SET_RX();	
}


#define LIN_ERR_TIMEOUT		-1
#define LIN_ERR_CHKSUM		-2
#define LIN_ERR_SERIAL		-3

// Poll for a response frame
// 	<id> is the contains only the first 6 bits: the P0 and P1 protection will be calculated by this function
//	<len> is the number of data bytes expected
//	<data> is the array of bytes to received the data
// NOTE: <data> must be at least <len>+1 size.  The +1 is for receiving the checksum
// Function returns 0 on success, negative error codes on error
int LIN_poll_frame (unsigned char id, int len, unsigned char * data)
{
	unsigned char pid, sync=0x55;
	int byte, ret;
	uint32_t chksum, start;
	
	// break the line
	__LIN_SET_BRK();

	// do calculations here
	pid = __lin_pid_lut__[id&0x3F];
	chksum = (id < 0x3C) ? pid : 0;		// ID > 0x3C always use classic checksum
	
	// wait for break to finish
	// LIN requires 13 bit of 0 (Dorminant) to break the signal.
	// At 19.2kHz, that is 677.08usec of 0-bits
	__LIN_SET_TX();
	// one bit of recesive as break delimiter

	// send sync field and the frame and then chksum
	__LIN_TX_BYTE(sync);
	__LIN_TX_BYTE(pid);
	__LIN_SET_RX();	
	return (0);
}

// Specifc Frames
#define LIN_ELMOS_START_MEASUREMENT				0x00
#define LIN_ELMOS_MEASURE_CONFIG_DATA2_WRITE	0x30
#define LIN_ELMOS_MEASURE_CONFIG_DATA2_READ		0x31
#define LIN_ELMOS_CONFIG_DATA_WRITE_ENABLE		0x38
#define LIN_ELMOS_CONFIG_DATA_READ_REQ			0x39

void LIN_send_StartMeasurement (int measurementSetup)
{
	unsigned char frame[3] = { 0xAA, 0xAA, 0x00 };
	frame[2] |= (measurementSetup & 0x03);
	__ADDMSG("StartMeas");
	LIN_send_frame (LIN_ELMOS_START_MEASUREMENT, 3, frame);
}

typedef struct __tag_lin_measurement_config_data2__ {
	unsigned char sID;		// slave ID
	unsigned char mlen[4];	// measurement length
	unsigned char blen[4];  // burst length
	unsigned char thresRising;	// Threshold slope rising
	unsigned char thresFalling;	// Threshold slope falling
} LIN_MEASURE_CFG_DATA2_t, * LIN_MEASURE_CFG_DATA2_p;

#define LIN_send_ConfigDataWriteEnable(SlaveID,Store,Wait) \
	frame[0] = SlaveID & 0x0F;\
	frame[1] = Store & 0x0F;\
	__ADDMSG("CfgWEn"); \
	LIN_send_frame(LIN_ELMOS_CONFIG_DATA_WRITE_ENABLE, 2, frame);

void LIN_send_MeasurementConfig2Write ( const LIN_MEASURE_CFG_DATA2_p mcd2, int toEEPROM )
{
	char frame[8];
	
	LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x00, 20000);	
	
	frame[0] = mcd2->sID & 0x0f;
	frame[1] = mcd2->mlen[0];
	frame[2] = mcd2->mlen[1];
	frame[3] = mcd2->mlen[2];
	frame[4] = mcd2->mlen[3];
	frame[5] = (mcd2->blen[0] & 0x0f) | ((mcd2->blen[1] << 4) & 0xf0);
	frame[6] = (mcd2->blen[2] & 0x0f) | ((mcd2->blen[3] << 4) & 0xf0);
	frame[7] = (mcd2->thresFalling & 0x0f) | ((mcd2->thresRising << 4) & 0xf0);
	__ADDMSG("CfgDataW");
	LIN_send_frame (LIN_ELMOS_MEASURE_CONFIG_DATA2_WRITE, 8, frame);
	
	LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x05, 20000);	
	if (toEEPROM) {
		LIN_send_ConfigDataWriteEnable(mcd2->sID, 0x50, 1600000);
	}
}

#define LIN_send_ConfigDataReadReq(SlaveID,Mem) \
	frame[0] = (Mem ? 0x10 : 0x00) | (SlaveID & 0x0F);\
	__ADDMSG("CfgDataReq");\
	LIN_send_frame (LIN_ELMOS_CONFIG_DATA_READ_REQ, 1, frame);

int LIN_send_MeasurementConfig2Read ( LIN_MEASURE_CFG_DATA2_p mcd2, int fromEEPROM )
{
	char frame[8];
	
	LIN_send_ConfigDataReadReq (mcd2->sID, fromEEPROM);
	__ADDMSG("MCfgData2R");
	if (LIN_poll_frame (LIN_ELMOS_MEASURE_CONFIG_DATA2_READ, 8, frame) == 0) {
		mcd2->mlen[0] = frame[1];
		mcd2->mlen[1] = frame[2];
		mcd2->mlen[2] = frame[3];
		mcd2->mlen[3] = frame[4];
		mcd2->blen[0] = frame[5] & 0x0F;
		mcd2->blen[1] = (frame[5] >> 4) & 0x0F;
		mcd2->blen[2] = frame[6] & 0x0F;
		mcd2->blen[3] = (frame[6] >> 4) & 0x0F;
		mcd2->thresFalling = frame[7] & 0x0F;
		mcd2->thresRising = (frame[7] >> 4) & 0x0F;
		return (0);
	}
	return (-1);
}


static void print_measurement_config (LIN_MEASURE_CFG_DATA2_p mcd2)
{
	printf ("Measurement Configuration for tslaveID = 0x%x\n", mcd2->sID);
	printf ("\tmLen[] = %3d, %3d, %3d, %3d\n", mcd2->mlen[0], mcd2->mlen[1], mcd2->mlen[2], mcd2->mlen[3]);
	printf ("\tbLen[] = %3d, %3d, %3d, %3d\n", mcd2->mlen[0], mcd2->mlen[1], mcd2->mlen[2], mcd2->mlen[3]);
	printf ("\tTheshold Falling = %d,  Theshold Rising = %d\n", mcd2->thresFalling, mcd2->thresRising);
}
		
int main (int argc, char * argv[])
{
	LIN_MEASURE_CFG_DATA2_t mcd2;
	LIN_init();
	
	printf("Sending StartMeasurement(0).\n");
	LIN_send_StartMeasurement(0);
	printf("Sending StartMeasurement(1).\n");
	LIN_send_StartMeasurement(1);
	printf("Sending MeasurementConfigRead for Slave 1.\n");
	mcd2.sID = 1;
	LIN_send_MeasurementConfig2Read(&mcd2, 0);
	printf("Sending MeasurementConfigRead for Slave 2.\n");
	mcd2.sID = 2;
	LIN_send_MeasurementConfig2Read(&mcd2, 0);
		
	LIN_close();
	
	return (0);
}

