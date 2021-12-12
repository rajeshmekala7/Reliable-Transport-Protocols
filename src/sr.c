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

#define BUFFER_SIZE 10000
#define A 0
#define B 1
#define RTT 25.0

// send_base is the sequence number of the oldest unacknowledged packet
// next_seqnum is the smallest unused sequence number
// N is the size of the window
int send_base, next_seqnum, N;

//  for accessing the buffer
int idx;

struct pkt buffer_packets[BUFFER_SIZE];
struct pkt buffer_received_packets[BUFFER_SIZE];

/*declaring an array for ack's received so that we can send
send all the unack packets again*/
int a_ack[BUFFER_SIZE];
int time[BUFFER_SIZE];

int rcv_base;
int b_ack[BUFFER_SIZE];

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

void send_message(){
	while( next_seqnum >= send_base && next_seqnum < send_base + N){
		tolayer3(A, buffer_packets[next_seqnum]);
		time[next_seqnum]=get_sim_time();
	}
	if(send_base==next_seqnum){
			//check when to start the timer
			starttimer(A, 20.0);
	}   
	next_seqnum++;	
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
	int i;
	if(packet.acknum < send_base){
		/*duplicate ACK && -1 for corrupted packets, timer interrupted will be called in this case 
		and packet will be resent*/ 
		return;
	}  
	else if(packet.acknum >= send_base && packet.acknum < send_base + N){
		//stop the timer
		stoptimer(A);
		
		//storing the acknowledgement information
		a_ack[packet.acknum]=1;
		
		//
		int i;
		for(i=send_base;i<next_seqnum;i++) {
		  if(a_ack[i] == 0 && get_sim_time() - time[i] < RTT)
	      {
	          int time_left = RTT - (get_sim_time() - time[i]);
	          starttimer(0, time_left);
	          break;
	      }
		}
		
		for(i = send_base; i < next_seqnum; i++)
	    {
	     if(a_ack[i] == 0  && get_sim_time() - time[i] > RTT)
	      {
	        tolayer3(0, buffer_packets[i]);
	        time[i] = get_sim_time();
	      }
	
	    }
		
		//changing the send_base value
		for(i=send_base;i<next_seqnum;i++){
			if(a_ack[i]==1){
				send_base=i;
				if(send_base + N < next_seqnum && a_ack[send_base + N] == 0)
		          {
		            tolayer3(0, buffer_packets[send_base + N]);
		            time[send_base + N] = get_sim_time();
		          }	
			}
		}
		
		
		
		//send remaining messages in buffer
	//	send_message();	
	}

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	int i;
	for(i=send_base;i<next_seqnum;i++){
		if(a_ack[i]==0){
			tolayer3(A, buffer_packets[i]);
			starttimer(A, 20.0);
			time[i]=get_sim_time();
			break;
		}
	}

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  	send_base=0;
	next_seqnum=0;
	N=getwinsize();
	idx=0;
	int i;
	for(i=0;i<BUFFER_SIZE;i++){
		a_ack[i]=0;
		time[i]=-1;
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
	if(packet.seqnum >= rcv_base && packet.seqnum < rcv_base+N-1){	
		
		//sending the ack
		ack_packet.acknum=packet.seqnum;
		tolayer3(B,ack_packet);
		
		//storing the acknowledgement information
		b_ack[packet.seqnum]=1;
		
		//storing the packet
		buffer_received_packets[packet.seqnum]=packet;
	
		//sending the message to layer5 if they are received in order
		for(i=rcv_base;i<rcv_base+N-1;i++){
			if(b_ack[i]==0){
				break;
			}
			tolayer5(B,buffer_received_packets[i].payload);
			rcv_base=i;
		}
	}

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	rcv_base=0;
	int i;
	for(i=0;i<BUFFER_SIZE;i++){
		b_ack[i]=0;
	}	
}
