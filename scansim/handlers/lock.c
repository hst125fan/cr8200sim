#include "lock.h"
#include "../cr8200sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int handleLock( const char* data, int length, int comport )
{
   const char* action = "";
   if( length > 23 && 0 == memcmp( data + 17, "CFLKXLK", 7 ) )
   {
      action = "locked";
   }
   else if( length > 23 && 0 == memcmp( data + 17, "CFLKXUK", 7 ) )
   {
      action = "unlocked";
   }
   else
   {
      return 1;
   }

   int seq = data[16];
   seq = seq | ( data[15] << 8 );
   seq = seq & 0x7F;

   const char* response = "<Response Val=\"0\" Description=\"none\" />";
   sendData( comport, response, strlen( response ), seq, 0x02, 0x40000000 );

   char* crypto = (char*)malloc( length - 23 );
   memset( crypto, 0, length - 23 );
   memcpy( crypto, data + 24, length - 24 );
   printf( "Reader %s with crypto string %s\n", action, crypto );

   free( crypto );
   return 0;
}