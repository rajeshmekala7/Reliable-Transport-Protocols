#include "../include/simulator.h"

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

// send_base is the sequence number of the oldest unacknowledged packet
// next_seqnum is the smallest unused sequence number
// N is the size of the window
int send_base, next_seqnum, N;
int recv_base;

struct pkt buffer_packets[BUFFER_SIZE];
struct pkt buffer_received_packets[BUFFER_SIZE];
int sent_time[BUFFER_SIZE];

//  for accessing the buffer
int idx;

/*declaring an array for ack's received so that we can send
send all the unack packets again*/
int a_ack[BUFFER_SIZE];
int b_ack[BUFFER_SIZE];

int checksum(struct pkt packet){
	int checksum=0,i;
	for(i=0;i<20;i++) {
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
   buffer_packets[idx].seqnum = next_seqnum;
   //buffer_packets[idx].acknum = -1;
   for(i=0;i<20;i++){
		if(message.data[i]=='\0') {
	    	break;
		}
		buffer_packets[idx].payload[i]=message.data[i];
	}
	buffer_packets[idx].payload[i]='\0';
   buffer_packets[idx].checksum = checksum(buffer_packets[idx]);
   sent_time[idx] = 0;
   idx++;


	if(send_base == next_seqnum) {
		tolayer3(0, buffer_packets[next_seqnum]);
		starttimer(0, 25.0);
	} else if(next_seqnum > send_base && (next_seqnum < send_base + N)) {
		tolayer3(0, buffer_packets[next_seqnum]);
		sent_time[next_seqnum] = get_sim_time();
	}
   next_seqnum++;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{

	if(packet.acknum < send_base) {
		return;
	} else if(packet.acknum == send_base) {
		a_ack[send_base] = 1;
		stoptimer(0);
		int i;
		for(i=send_base;a_ack[i]==0;i++) {
			if(i + N < next_seqnum && a_ack[i + N] == 0) {
				tolayer3(0, buffer_packets[i + N]);
				sent_time[i + N] = get_sim_time();
			}
		}
		send_base = i;
	} else if(packet.acknum > send_base && (packet.acknum < send_base + N)) {
		a_ack[send_base] = 1;
	}

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	tolayer3(0, buffer_packets[send_base]);
	sent_time[send_base] = get_sim_time();
	starttimer(0, 25.0);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
   send_base=0;
 	next_seqnum=0;
 	N=getwinsize();
 	idx=0;
	for(int i=0;i<BUFFER_SIZE;i++){
		a_ack[i]=0;
	}
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	int i;
	struct pkt ack_packet;
	if(packet.checksum!=checksum(packet)){
		//corrupt packet, sending the previous ack value
		ack_packet.acknum=-1;
		tolayer3(B, ack_packet);
		return;
	}

	if(packet.seqnum >= recv_base && packet.seqnum < recv_base + N)
	{
		ack_packet.acknum = packet.seqnum;
		tolayer3(1, ack_packet);
		buffer_received_packets[packet.seqnum] = packet;
		b_ack[packet.seqnum]= 1;
		for(i=recv_base;i<recv_base+N;i++) {
			if(b_ack[i]==0) {
				break;
			}
			tolayer5(B,buffer_received_packets[i].payload);
		}
		recv_base=i;
	}


}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	int i;
	recv_base = 0;
	for(i=0;i<BUFFER_SIZE;i++){
		b_ack[i]=0;
	}
}
