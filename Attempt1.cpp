#include "frameio2.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

frameio net;             // gives us access to the raw network
message_queue ip_queue;  // message queue for the IP protocol stack
message_queue arp_queue; // message queue for the ARP protocol stack

struct ether_frame       // handy template for 802.3/DIX frames
{
   octet dst_mac[6];     // destination MAC address
   octet src_mac[6];     // source MAC address
   octet prot[2];        // protocol (or length)
   octet data[1500];     // payload
};

//
// This thread sits around and receives frames from the network.
// When it gets one, it dispatches it to the proper protocol stack.
//
void *protocol_loop(void *arg)
{
   ether_frame buf;
   while(1)
   {
      int n = net.recv_frame(&buf,sizeof(buf));
      if ( n < 42 ) continue; // bad frame!
      //
      //octet goal[] = {0xc8, 0xd9, 0xd2, 0x0c, 0xa4, 0x41};
      //if (buf.src_mac == goal){
      
	for(int i = 0;i<6;i++){
		  printf("%02x ", buf.dst_mac[i]);
	}
	for(int i = 0;i<6;i++){
		  printf("%02x ", buf.src_mac[i]);
	}
	for(int i = 0;i<2;i++){
		  printf("%02x ", buf.prot[i]);
	}
	for(int i = 0;i<8;i++){
		  printf("%02x ", buf.data[i]);
	}
	printf("\n");
	for(int i = 8; i<28; i++){
		  printf("%02x ", buf.data[i]);
	}
	printf("\n\n");
      //}
      /*
      printf("\n");
      for(int i = 22; i<42 ;i++){
		printf("%02x ", buf[i]);
      }
      printf("\n\n");
      //
      */
      /*
      switch ( buf.prot[0]<<8 | buf.prot[1] )
      {
          case 0x800:
             ip_queue.send(PACKET,buf.data,n);
             break;
          case 0x806:
             arp_queue.send(PACKET,buf.data,n);
             break;
      }
      */
   }
}

//
// Toy function to print something interesting when an IP frame arrives
//
void *ip_protocol_loop(void *arg)
{
   octet buf[1500];
   event_kind event;
   int timer_no = 1;

   // for fun, fire a timer each time we get a frame
   while ( 1 )
   {
      ip_queue.recv(&event, buf, sizeof(buf));
      if ( event != TIMER )
      {
         printf("got an IP frame from %d.%d.%d.%d, queued timer %d\n",
                  buf[12],buf[13],buf[14],buf[15],timer_no);
         ip_queue.timer(10,timer_no);
	 //First Test
	 //printf("%02x %02x %02x %02x %02x %02x %02x %02x \n\n",
		//buf[16],buf[17],buf[18],buf[19],buf[20],buf[21],
		//buf[22],buf[23]);
	 //
	 
	 //Second Test
	 /*
	 for(int i = 0;i<22;i++){
		printf("%02x ", buf[i]);
	 }
	 printf("\n");
	 for(int i = 22;i<42;i++){
		printf("%02x ", buf[i]);
	 }
	 printf("\n\n");
	 */
	 //
         timer_no++;
      }
      else
      {
         printf("timer %d fired\n",*(int *)buf);
      }
   }
}

//
// if you're going to have pthreads, you'll need some thread descriptors
//
pthread_t loop_thread, arp_thread, ip_thread;

//
// start all the threads then step back and watch (actually, the timer
// thread will be started later, but that is invisible to us.)
//
int main()
{
   net.open_net("c8:d9:d2:0c:a4:41");
   pthread_create(&loop_thread,NULL,protocol_loop,NULL);
   //pthread_create(&arp_thread,NULL,arp_protocol_loop,NULL);
   pthread_create(&ip_thread,NULL,ip_protocol_loop,NULL);
   for ( ; ; )
      sleep(1);
}
