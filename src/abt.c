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

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/

#define A 0
#define B 1
#define BUFFER_SIZE 1000

int a_seqnum, b_seqnum, last_seqnum, last_acknum;
int a_state;  // value would be 0 (waiting for ACK) or 1 (waiting for LAYER5)
struct pkt last_packet; //used to send last packet again if no ack is received
struct pkt buffer_packets[BUFFER_SIZE];
int idx;
int iterate;

int checksum(struct pkt packet){
	int checksum=0, i;
	for(i=0;i<20;i++){
	    if(packet.payload[i]=='\0') {
	    	break;
		}
		checksum+=packet.payload[i];
    }
	checksum+=packet.seqnum;
	checksum+=packet.acknum;
	return checksum;
}

void send_message(){
	if(a_state == 0){
		return;
	}
	else if(iterate<idx){
		last_packet = buffer_packets[iterate];
		a_state = 0;
		a_seqnum=buffer_packets[iterate].seqnum;
		starttimer(A, 20.0);
		tolayer3(A, buffer_packets[iterate] );
		iterate++;
	}

}
/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
	int i;

    for(i=0;i<20;i++) {
		if(message.data[i]=='\0') {
	    	break;
		}

    	buffer_packets[idx].payload[i] = message.data[i];
	}
	buffer_packets[idx].payload[i]='\0';
	if(idx==0){
		buffer_packets[idx].seqnum = 0;
	} else{
		buffer_packets[idx].seqnum = 1-buffer_packets[idx-1].seqnum;
	}

	buffer_packets[idx].checksum = checksum(buffer_packets[idx]);
	idx++;
	send_message();
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if(a_seqnum != packet.acknum && last_acknum != packet.acknum) {
		starttimer(A, 20.0);
		tolayer3(A, last_packet);
	} else if(last_acknum == packet.acknum) {
		// dupliate ack do nothing
	} else {
		printf("received %d ack from B\n", packet.acknum);
		stoptimer(A);
		last_acknum = packet.acknum;
		a_state=1;
		send_message();
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	printf(" timer interrupt called\n");
	starttimer(A, 25.0);
	tolayer3(A, last_packet);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	a_seqnum = 0;
	a_state = 1;
	idx=0;
	iterate=0;
	last_acknum=-1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	struct pkt ack_packet;
	if(packet.checksum != checksum(packet)) {
		//packet is corrupted
		printf("packet is corrupted\n");
		ack_packet.acknum = -1;
		tolayer3(B, ack_packet);
		return;
	} else if (packet.seqnum != b_seqnum && last_seqnum == packet.seqnum) {
		// ack lost
		ack_packet.acknum = 1-b_seqnum;
		tolayer3(B, ack_packet);
		return;
	}
	printf("packet %d received at B\n", b_seqnum);
	tolayer5(B, packet.payload);
	ack_packet.acknum = b_seqnum;
	last_seqnum = packet.seqnum;
	tolayer3(B, ack_packet);
	b_seqnum = 1-b_seqnum;
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	b_seqnum = 0;
	last_seqnum = -1;
}
