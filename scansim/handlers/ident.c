#include "config.h"
#include "../cr8200sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int handleIdent( const char* data, int length, int comport )
{
   const char* retstr = "";
   if( length > 23 && 0 == memcmp( data + 17, "RDFWGVS", 7 ) )
   {
      retstr = "<RD><FW VS=\"1.16.0\" /></RD>";
   }
   else if( length > 23 && 0 == memcmp( data + 17, "RDRRGMD", 7 ) )
   {
      retstr = "<RD><RR MD=\"CR8200\" /></RD>";
   }
   else if( length > 23 && 0 == memcmp( data + 17, "RDRRGHR", 7 ) )
   {
      retstr = "<RD><RR HR=\"0x04\" /></RD>";
   }
   else if( length > 23 && 0 == memcmp( data + 17, "RDCPGRV", 7 ) )
   {
      retstr = "<RD><CP RV=\"2.0\" /></RD>";
   }
   else
   {
      return 1;
   }
   
   int seq = data[16];
   seq = seq | ( data[15] << 8 );
   seq = seq & 0x7F;
   
   sendData( comport, retstr, strlen( retstr ), seq, 0x02, 0x40000000 );
   return 0;
}