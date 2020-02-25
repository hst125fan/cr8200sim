#pragma once

/* This is the function type for defining command handlers */
/* First parameter = payload data (non-null-terminated) */
/* second parameter = length of payload data */
/* third parameter - port number for response */
typedef int( *packetHandlerType )( const char*, int, int );

void dbgPrintData( const char* data, int length );

/* This method should be used to send data to the connected client */
void sendData( int port, const char* data, int length, int respondTo, int type, int destination );