#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "MQTTClient.h"
#include <bcm2835.h>
#include <tm1638.h>
#include <pthread.h>


#define ADDRESS     "127.0.0.1:1883"
#define CLIENTID    "TM1638ClientSub"
#define QOS         0
#define TOPIC_LEDS       "8leds"
#define TOPIC_TEXT       "7segs"
#define TOPIC_BTTN       "buttons"
#define TIMEOUT     10000L

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *buttonsHandler();

// global pointer to the TM1638
static tm1638_p t;

volatile MQTTClient_deliveryToken deliveredtoken;
static MQTTClient client;

void delivered(void * context, MQTTClient_deliveryToken dt) {
   printf("Message with token value %d delivery confirmed\n", dt);
   deliveredtoken = dt;
}

void format_dots(char* m, char** s, int* d){
  *s=malloc( strlen(m) * sizeof(char)  );

  char c;
  int dots=0;
  int j=0;
  char last_c=' ';

  for (int i=0; m[i]!=0 && j<9;i++){
    c=m[i];
    if(c=='.' || c==',') {
      if(j==0 || last_c == '.' ) { // first char is a dot or we repeat a dot
        c=' ';
        (*s)[j++]=c;
      }
      dots |= 1<<(j-1) ;
      last_c='.';
    } else {
        (*s)[j++]=c;
        last_c=c;
    }
  }
  (*s)[j<8?j:8]=0;

  dots &= 255; // limit to 8 bits dots

  *d=dots;
}

int msgarrvd(void * context, char * topicName, int topicLen, MQTTClient_message * message) {
   uint8_t i;
   char * payloadptr;
   char *msg;
   char *s;
   int dots;

   msg=malloc( (message->payloadlen +1) * sizeof(char)  );

   printf("topic: %s, payload:", topicName);

   payloadptr = message->payload;

   for (i = 0; i < message->payloadlen; i++) {
      msg[i] = * payloadptr++; // don't forget ++
   }
   msg[i] = '\0'; // we finish the msg with \0
   printf("%s\n", msg);

   if (strcmp(topicName, TOPIC_LEDS) == 0) {
      int leds = atoi(msg);

      pthread_mutex_lock( &mutex );
      tm1638_set_8leds(t, leds, 0);
      pthread_mutex_unlock( &mutex );
   } else if (strcmp(topicName, TOPIC_TEXT) == 0) {
      format_dots( msg, &s, &dots);

      pthread_mutex_lock( &mutex );
      tm1638_set_7seg_text(t, s, dots); //dots=0x00
      pthread_mutex_unlock( &mutex );
   }

   MQTTClient_freeMessage( & message);
   MQTTClient_free(topicName);
   return 1;
}

void connlost(void * context, char * cause) {
   printf("\nConnection lost\n");
   printf("     cause: %s\n", cause);
}

int main(int argc, char * argv[]) {

   if (!bcm2835_init()) {
      printf("Unable to initialize BCM library\n");
      return -1;
   }

   t = tm1638_alloc(17, 27, 22);
   if (!t) {
      printf("Unable to allocate TM1638\n");
      return -2;
   }

   //MQTTClient client;
   MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
   int rc;

   MQTTClient_create( & client, ADDRESS, CLIENTID,
      MQTTCLIENT_PERSISTENCE_NONE, NULL);
   conn_opts.keepAliveInterval = 20;
   conn_opts.cleansession = 1;

   MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

   if ((rc = MQTTClient_connect(client, & conn_opts)) != MQTTCLIENT_SUCCESS) {
      printf("Failed to connect, return code %d\n", rc);
      exit(-1);
   }

   //printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
   MQTTClient_subscribe(client, TOPIC_LEDS, QOS);
   MQTTClient_subscribe(client, TOPIC_TEXT, QOS);
   
   // THREAD for buttons handling
   int rc1;
   pthread_t thread1;
   if( (rc1=pthread_create( &thread1, NULL, &buttonsHandler, NULL)) ) {
      printf("Thread creation failed: %d\n", rc1); 
   }

   while(1) {
     delay(10);
   }

   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy( & client);
   return rc;

   pthread_join( thread1, NULL);
}

void *buttonsHandler()
{
   //MQTTClient_deliveryToken token;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   pubmsg.qos = QOS;
   pubmsg.retained = 0;

   uint8_t old_buttons = 0;
   uint8_t new_buttons, pressed, released;

   char payload[2];
   payload[0]=0;
   payload[1]=0;

   while(1)
   {
      pthread_mutex_lock( &mutex );
      new_buttons = tm1638_read_8buttons(t);
      pthread_mutex_unlock( &mutex );

      pressed = ( old_buttons ^ new_buttons ) & ~old_buttons ;
      released = ( old_buttons ^ new_buttons ) & old_buttons ;

      for (unsigned char i=0;i<8;i++){
        if(pressed & 1) {
          printf("button %d pressed\n", i );

          payload[0] = '0' + i;
          payload[1] = 0;

          pubmsg.payload = &payload;
          pubmsg.payloadlen = strlen(pubmsg.payload);

          MQTTClient_publishMessage(client, TOPIC_BTTN, &pubmsg, NULL);
        }
        pressed >>= 1;

        if(released & 1) printf("button %d released\n", i );
        released >>= 1;
      }

      old_buttons=new_buttons;
      delay(10);
   }
}
