#include "config.h"
#include "../cr8200sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int handleConfig( const char* data, int length, int comport )
{
   const char* action = "";
   if( ( length > 23 && ( data[21] == 'S' || data[21] == 'P' ) ) ||
       ( length > 25 && data[17] == 'S' && data[18] == 'Y' && ( data[23] == 'S' || data[23] == 'P' ) ) )
   {
      int seq = data[16];
      seq = seq | ( data[15] << 8 );
      seq = seq & 0x7F;
      
      const char* response = "<Response Val=\"0\" Description=\"none\" />";
      sendData( comport, response, strlen( response ), seq, 0x02, 0x40000000 );
      return 0;
   }
   return 1;
}