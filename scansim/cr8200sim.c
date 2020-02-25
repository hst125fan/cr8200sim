
/* standard libraries */
#include <stdio.h>
#include <string.h>

/* 3rdparty deps */
#include "rs232.h"
#include "crc.h"

/* my own header */
#include "cr8200sim.h"

/* features / handlers */
#include "handlers/lock.h"
#include "handlers/img.h"
#include "handlers/barcode.h"
#include "handlers/config.h"
#include "handlers/ident.h"

#define HANDLERARRAYSIZE 6
#define LOGWIDTH 16

/* Same as comRead, but blocks until <length> bytes have been read */
void comReadDefinetly( int port, char* data, int length )
{
   int whereAmI = 0;
   while( whereAmI < length )
   {
      int read = comRead( port, data + whereAmI, length - whereAmI );
      whereAmI = whereAmI + read;
   }
}

void dbgPrintData( const char* data, int length )
{
   char backbuf[LOGWIDTH];
   memset( backbuf, 0, LOGWIDTH );
   for( int i = 0; i < length; i++ )
   {
      backbuf[i%LOGWIDTH] = *( data + i );
      printf( " %02x", *( data + i ) & 0xFF );
      if( i % LOGWIDTH == 15 || i == length - 1 )
      {
         for( int j = i % LOGWIDTH; j < 15; j++ )
         {
            printf( "   " );
         }
         printf( "    " );
         for( int j = 0; j <= i % LOGWIDTH; j++ )
         {
            if( backbuf[j] > ' ' && backbuf[j] < '~' )
            {
               printf( "%c", backbuf[j] );
            }
            else
            {
               printf( "." );
            }
         }
         printf( "\n" );
      }
   }
   printf( "============================================\n" );
}

void sendData( int port, const char* data, int length, int respondTo, int type, int destaddress )
{
   int s19 = ( type == 0x02 ) ? 19 : 17;
   int s25 = ( type == 0x02 ) ? 25 : 23;
   int s23 = ( type == 0x02 ) ? 23 : 21;
   int packlength = length + s19;
   char* sendData = (char*)malloc( length + s25 );
   memset( sendData, 0, length + s25 );
   sendData[0] = 1;
   sendData[1] = 'C';
   sendData[2] = 'T';
   sendData[3] = '1';

   sendData[4] = packlength >> 8;
   sendData[5] = packlength & 0xFF;

   sendData[6] = ( destaddress >> 24 ) & 0xFF;
   sendData[7] = ( destaddress >> 16 ) & 0xFF;
   sendData[8] = ( destaddress >> 8 ) & 0xFF;
   sendData[9] = destaddress & 0xFF;

   sendData[10] = 0x0B;
   sendData[11] = 0x1A;
   sendData[12] = 0x20;
   sendData[13] = 0x4C;

   sendData[14] = 0x01;
   sendData[15] = 0x00;
   sendData[16] = type;

   sendData[17] = 0x00;
   sendData[18] = 0x00;

   sendData[19] = 0x00;
   sendData[20] = 0x01;

   if( type == 0x02 )
   {
      sendData[21] = respondTo >> 8;
      sendData[22] = respondTo & 0xFF;
   }

   memcpy( sendData + s23, data, length );

   unsigned short crc16table[CRC_TABLESIZE * sizeof( unsigned short )];
   crc16_table( crc16table, CRC16_SEED );
   unsigned short crcsum = crc16( 0, crc16table, sendData + 6, packlength - 2 );
   sendData[length + s23] = crcsum >> 8;
   sendData[length + s23 + 1] = crcsum & 0xFF;

   comWrite( port, sendData, length + s25 );
   free( sendData );
}

int handleRdrga( const char* data, int length, int comport )
{
   if( length > 21 && 0 == memcmp( data + 17, "RDRDG", 5 ) )
   {
      const char* response = "<RD><FW MJ=\"1\" MN=\"16\" BU=\"0\" OP=\"\" VS=\"1.16.0\" TY=\"C010603\" DV=\"cd(19.1.13.6081 - 20191105:1419)\" />"
                             "<CP RV=\"2.0\" SN=\"0123456789ABCD\" />"
                             "<RR SN=\"1234567890\" ID=\"123456789\" HR=\"0x04\" MD=\"CR8200\" MT=\"2AD0\" IS=\"20191119:1824\" DD=\"\" />"
                             "<QD S0=\"0\" N1=\"1\" L1=\"1\" N0=\"0\" L0=\"0\" RS=\"1\" US=\"0\" TG=\"1\" LD=\"1\" IL=\"1\" SP=\"1\" TR=\"1\" B1=\"0\" B2=\"0\" BL=\"0\" AF=\"0\" WT=\"0\" BT=\"0\" DL=\"0\" WL=\"0\" F0=\"0x00\" P0=\"0x00\" F1=\"0x78\" P1=\"0x02\" />"
                             "<LC GL=\"\" /><FB VB=\"1\" SM=\"0\" />"
                             "<IL LO=\"1\" MB=\"100\" AC=\"0\" />"
                             "<OF LE=\"\" />"
                             "<TC MD=\"1\" T1=\"0\" T2=\"0\" T3=\"0\" /></RD>";
      int seq = data[16];
      seq = seq | ( data[15] << 8 );
      seq = seq & 0x7F;
      sendData( comport, response, strlen( response ), seq, 0x02, 0x40000000 );
      return 0;
   }
   return 1;
}

void sendAck( int port, char acker[2] )
{
   char sendData[21];
   sendData[0] = 1;
   sendData[1] = 'C';
   sendData[2] = 'T';
   sendData[3] = '1';

   sendData[4] = 0;
   sendData[5] = 15;

   sendData[6] = 0x40;
   sendData[7] = 0x00;
   sendData[8] = 0x00;
   sendData[9] = 0x00;

   sendData[10] = 0x0B;
   sendData[11] = 0x1A;
   sendData[12] = 0x20;
   sendData[13] = 0x4C;

   sendData[14] = 0x01;
   sendData[15] = 0x01;
   sendData[16] = 0x00;

   sendData[17] = acker[0];
   sendData[18] = acker[1];


   unsigned short crc16table[CRC_TABLESIZE];
   crc16_table( crc16table, CRC16_SEED );
   unsigned short crcsum = crc16( 0, crc16table, sendData + 6, 13 );
   sendData[19] = crcsum >> 8;
   sendData[20] = crcsum & 0xFF;

   comWrite( port, sendData, 21 );
}


int handleConnectionProtocol( int port, packetHandlerType* packetHandlers , int packetHandlersCount )
{
   int isOpen = comOpen( port, 115200 );
   if( isOpen == 0 )
   {
      return 1;
   }
   int handleResponse = 0;
   int state = 0;
   int readlen = 0;
   while( 1 )
   {
      if( state == 4 )
      {
         char length[2];
         comReadDefinetly( port, length, 2 );
         state = 5;
         readlen = (length[0] << 8) | length[1];
      }
      else if( state == 5 )
      {
         char* data = (char*)malloc( readlen );
         comReadDefinetly( port, data, readlen );
         if( data[10] != 0x00 )
         {
            sendAck( port, data + 13 );
         }
         for( int i = 0; i < packetHandlersCount; i++ )
         {
            packetHandlerType packetHandler = *(packetHandlers + i);
            handleResponse = packetHandler( data, readlen - 2, port );
            if( handleResponse == 0 )
            {
               break;
            }
         }
         
         free( data );
         state = 0;
      }
      else
      {
         char c;
         comReadDefinetly( port, &c, 1 );
         switch( c | state << 8 )
         {
            case 0x001: state = 1; break;
            case 0x143: state = 2; break;
            case 0x254: state = 3; break;
            case 0x331: state = 4; break;
            default: state = 0; break;
         }
      }
   }
   comClose( port );
   return 0;
}

int main( int argc, char** argv )
{
   if( argc != 2 )
   {
      printf( "Usage: %s <COM/RS232-Port>\n", argv[0] );
      return 1;
   }
   int ports = comEnumerate();
   if( ports < 1 )
   {
      printf( "No COM/RS232 Ports found!\n" );
      return 1;
   }
   int myPortEnumNo = comFindPort( argv[1] );
   if( myPortEnumNo < 0 )
   {
      printf( "COM/RS232 Port %s doesn't exist\n", argv[1] );
      for( int i = 0; i < ports; i++ )
      {
         printf( "\tTry COM/RS232 Port \"%s\"\n", comGetPortName( i ) );
      }
      comTerminate();
      return 1;
   }

   packetHandlerType handlers[HANDLERARRAYSIZE];

   /* Below all command handlers need to be defined */
   /* HANDLERARRAYSIZE needs to be set accordingly (=how many handlers)*/
   handlers[0] = handleRdrga;
   handlers[1] = handleLock;
   handlers[2] = handleImage;
   handlers[3] = handleBarcode;
   handlers[4] = handleConfig;
   handlers[5] = handleIdent;
   
   printf( "Now listening on COM-Port %s\n", argv[1] );
   printf( " -> Terminate the program with Ctrl+C\n" );
   printf( " -> Move a PNG file to working directory (as \"barcode.png\") to decode\n" );
   printf( " -> Move decoded data to working directory (as \"barcode.dat\") to send\n" );
   printf( "Processed files (barcode.png/barcode.dat) will be deleted after use.\n" );
   printf( "Barcode reading must be activated first (trigger event/continuous mode)\n" );
   

   handleConnectionProtocol( myPortEnumNo, handlers, HANDLERARRAYSIZE );
   comTerminate();
   return 0;
}