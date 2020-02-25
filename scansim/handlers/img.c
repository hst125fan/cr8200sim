#include "img.h"
#include "../cr8200sim.h"
#include <stdlib.h>
#include <string.h>
#include "testpicture.h"

int handleImage( const char* data, int length, int comport )
{
   int sendImage = 0;
   if( length > 23 && 0 == memcmp( data + 17, "CMCPPPM", 7 ) )
   {
      /* fallthrough */
   }
   else if( length > 23 && 0 == memcmp( data + 17, "CDIMPET", 7 ) )
   {
      /* fallthrough */
   }
   else if( length > 23 && 0 == memcmp( data + 17, "ENIMPJQ", 7 ) )
   {
      /* fallthrough */
   }
   else if( length > 23 && 0 == memcmp( data + 17, "CDTPXEV", 7 ) )
   {
      sendImage = 1;
      /* fallthrough */
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

   if( sendImage )
   {
      int size = testpicturesize - 1;
      sendData( comport, testpicture, testpicturesize - 1, seq, 0x03, 0x4FFFFFFF );
   }
   return 0;
}