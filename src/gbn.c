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

#define BUFFER_SIZE 50000
#define A 0
#define B 1

// base is the sequence number of the oldest unacknowledged packet
// next_seqnum is the smallest unused sequence number
// N is the size of the window
int base, next_seqnum, N;
//  for accessing the buffer
int idx;
int b_seqnum, last_seqnum;
struct pkt buffer_packets[BUFFER_SIZE];

// send all eligible to be sent messages in the window.
void send_message(){
	int i;
	for(i=next_seqnum;i<idx && i<base+N; i++) {
		tolayer3(A, buffer_packets[next_seqnum]);
		if(base==next_seqnum){
			starttimer(A, 20.0);
		}
	}
	next_seqnum=i;
}


int checksum(struct pkt packet){
	int checksum=0,i;
	for( i=0;i<20;i++){
	    if(packet.payload[i]=='\0') {
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
	int i;
	if( idx == BUFFER_SIZE){
		//buffer is full
		return;
	}
	for(i=0;i<20;i++){
		if(message.data[i]=='\0') {
	    	break;
		}
		buffer_packets[idx].payload[i]=message.data[i];
	}
	buffer_packets[idx].payload[i]='\0';
	buffer_packets[idx].seqnum=idx;
	buffer_packets[idx].checksum = checksum(buffer_packets[idx]);
	idx++;
	send_message();
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	if (packet.acknum == base) {
		base++;
		if(base<next_seqnum) {
			stoptimer(A);
			starttimer(A, 20.0);
		} else {
			stoptimer(A);
			send_message();
		}
	} else if(packet.acknum > base) {
		//resend_messages();
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	//re-send all the messages in the window
	resend_messages();
}

void resend_messages() {
	for(int i=base;i<next_seqnum;i++){
		tolayer3(A, buffer_packets[i]);
		if(i==base)
		starttimer(A, 20.0);
	}
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	base=0;
	next_seqnum=0;
	N=getwinsize();
	idx=0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	struct pkt ack_packet;
	if(packet.checksum!=checksum(packet)){
		//corrupt packet, sending the previous ack value
		ack_packet.acknum=last_seqnum;
		tolayer3(B, ack_packet);
		return;
	} else if(packet.seqnum!=b_seqnum){
		//a packet is missed, sending the previous ack value
		ack_packet.acknum=last_seqnum;
		tolayer3(B, ack_packet);
		return;
	}
	last_seqnum=packet.seqnum;
	tolayer5(B, packet.payload);
	ack_packet.acknum=b_seqnum;
	tolayer3(B, ack_packet);
	b_seqnum++;
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	b_seqnum=0;
	last_seqnum=-1;
}
