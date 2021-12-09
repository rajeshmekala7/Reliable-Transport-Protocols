#include "../include/simulator.h"
#include<stdio.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int nop=0; //number of packets currently in window
int a_seqnum, b_seqnum;
struct pkt last_packet[50];


int checksum(struct pkt packet){
	int checksum=0,i;
	for( i=0;i<20;i++){
	    if(packet.payload[i]='\0') {
	    	break;
		}
		checksum+=packet.payload[i];
    }
	checksum+=packet.seqnum;
	checksum+=packet.acknum;
	return checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	/* if number of packets in the window is equal to 
	 	the window size buffer the packet*/

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	
}

/* called when A's timer goes off */
void A_timerinterrupt()
{

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	a_seqnum=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	b_seqnum=0;
}
