#include "barcode.h"
#include "../cr8200sim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ReadBarcode.h"
#include "TextUtfEncoding.h"

#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <thread>
#include <mutex>
#include <fstream>

using namespace ZXing;

std::thread* myThread = nullptr;
std::mutex myMutex;
bool threadrun = false;
bool onetime = false;
int lastComport = 0;

void threadStart()
{
   myMutex.lock();
   threadrun = true;
   if( !myThread )
   {
      myThread = new std::thread([&](){
         while( threadrun )
         {
            int width, height, channels;
            std::unique_ptr<stbi_uc, void(*)(void*)> buffer(stbi_load("barcode.png", &width, &height, &channels, 4), stbi_image_free);
            if (buffer == nullptr) 
            {
               std::ifstream override_file( "barcode.dat", std::ios::binary );
               std::string read( "11" );
               if( override_file.good() )
               {
                  while( override_file.good() )
                  {
                     read.append( 1, (char)override_file.get() );
                  }
                  dbgPrintData( read.c_str() + 2, read.length() - 3 );
                  sendData( lastComport, read.c_str(), (read.length() - 1), 0x00, 0x01, 0x40000000 );
                  override_file.close();
                  remove( "barcode.dat" );
               }
               else
               {
                  std::this_thread::sleep_for(std::chrono::seconds(2));
               }
               continue;
            }
            auto result = ReadBarcode(width, height, buffer.get(), width * 4, 4, 0, 1, 2, {}, true, true);
            remove( "barcode.png" );
            if( result.isValid() )
            {
               std::string read( "11" );
               for( int i = 0; i < result.text().length(); i++ )
               {
                  read.append( 1, (char)(result.text()[i]) );
               }
               dbgPrintData( read.c_str() + 2, read.length() - 2 );
               sendData( lastComport, read.c_str(), read.length(), 0x00, 0x01, 0x40000000 );
               if( onetime )
               {
                  myMutex.lock();
                  threadrun = false;
                  myMutex.unlock();
               }
            }
            else
            {
               printf("No barcode found inside barcode.png!\n");
            }
         }
      });
   }
   myMutex.unlock();
}

void threadStop()
{
   myMutex.lock();
   if( myThread )
   {
      threadrun = false;
      myThread->join();
      delete myThread;
      myThread = nullptr;
   }
   myMutex.unlock();
}

void decodePoll()
{
   threadStop();
   onetime = false;
   threadStart();
}

void decodeSingle()
{
   threadStop();
   onetime = true;
   threadStart();
}

void decodeStop()
{
   threadStop();
}

void answer_request( const char* data )
{
   int seq = data[16];
   seq = seq | ( data[15] << 8 );
   seq = seq & 0x7F;
      
   const char* response = "<Response Val=\"0\" Description=\"none\" />";
   sendData( lastComport, response, strlen( response ), seq, 0x02, 0x40000000 );
}

int handleBarcode( const char* data, int length, int comport )
{
   lastComport = comport;
   if( length > 30 && 0 == memcmp( data + 17, "RDCMXEV1,P11,P20", 16 ) )
   {
      decodeSingle();
      return 0;
   }
   else if( length > 26 && 0 == memcmp( data + 17, "RDCMXEV1,P10", 12 ) )
   {
      decodeStop();
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPSMD1", 8 ) )
   {
      decodePoll();
      answer_request( data );
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPSMD2", 8 ) )
   {
      decodePoll();
      answer_request( data );
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPPMD1", 8 ) )
   {
      decodePoll();
      answer_request( data );
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPPMD2", 8 ) )
   {
      decodePoll();
      answer_request( data );
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPPMD0", 8 ) )
   {
      decodeStop();
      answer_request( data );
      return 0;
   }
   else if( length > 22 && 0 == memcmp( data + 17, "CDOPPMD0", 8 ) )
   {
      decodeStop();
      answer_request( data );
      return 0;
   }
   return 1;
}