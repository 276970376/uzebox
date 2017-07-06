/*
*  Based on Sd Simple Tutorial
*  by Cunning Fellow
*
*  Streaming Music Demo
*  by Lee Weber(D3thAdd3r)
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ctype.h>
#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include "data/tiles.inc"
#include "data/fonts6x8.inc"
#include "data/compressedsong.inc"
#include "data/patches.inc"
#include <sdBase.h>


const char  fileName[] PROGMEM = "SD_MUSICDAT";

void CustomWaitVsync(u8 frames);
void UpdateEqualizer();

u8 sdInUse;
uint32_t songOff = 0;
extern volatile u16 songPos;
extern volatile u16 loopStart;
extern volatile u16 loopEnd;
extern Track tracks[CHANNELS];
u16 loopEndFound;
u16 bufferLimiter;
u16 bpfLimiter;
long sectorStart;

u16 padState, oldPadState;
u8 equalizer[SCREEN_TILES_H];
u8 equalizerHigh[SCREEN_TILES_H];

int main(){
   InitMusicPlayer(patches);
   SetTileTable(tiles);         //Set the tileset to use (set this first)
   SetFontTilesIndex(TILES_SIZE);   //Set the tile number in the tilset that contains the first font
   ClearVram();               //Clear the screen (fills the vram with tile zero)

   long otherOffset = 10;

   sdCardInitNoBuffer();
   
   if((sectorStart = sdCardFindFileFirstSectorFlash(fileName)) == 0)
      Print(0,0,PSTR("FILE SD_MUSIC.DAT NOT FOUND ON SD CARD"));
   else{
      sdCardCueSectorAddress(sectorStart);
      Print(0,0,PSTR("FOUND SD_MUSIC.DAT"));
   }
   sdInUse = 0;

   Print(0,2,PSTR("SONGPOS  :"));
   Print(0,3,PSTR("LOOPSTART:"));
   Print(0,4,PSTR("LOOPEND  :"));

   Print(0,6,PSTR("BUFFERED :"));
   Print(0,7,PSTR("FILL RATE:"));
   Print(0,8,PSTR("BUF LIMIT:"));
   Print(0,9,PSTR("MAX SIZE :"));

   bufferLimiter = 64;//let the user simulate different buffer sizes in real time
   bpfLimiter = 16;//let the user simulate different fill speeds

   /*while(!SongBufFull() && (SongBufBytes() < bufferLimiter)){
      SongBufWrite(sdCardGetByte());songOff++;
      //SongBufWrite(pgm_read_byte(&CompressedSong[songOff++]));
   }*/
   CustomWaitVsync(60);

   StartSong();

   u16 counter = 0;
   sdInUse = 0;

   while(1){
      oldPadState = padState;
      padState = ReadJoypad(0);

      if(padState & BTN_UP && !(oldPadState & BTN_UP) && bufferLimiter < SONG_BUFFER_SIZE)
         bufferLimiter++;
      if(padState & BTN_DOWN && !(oldPadState & BTN_DOWN) && bufferLimiter > SONG_BUFFER_MIN)
         bufferLimiter--;

      if(padState & BTN_LEFT && !(oldPadState & BTN_LEFT) && bpfLimiter)
         bpfLimiter--;

      if(padState & BTN_RIGHT && !(oldPadState & BTN_RIGHT) && bpfLimiter < bufferLimiter)
         bpfLimiter++;


      PrintInt(14,2,songPos,1);
      PrintInt(14,3,loopStart,1);
      PrintInt(14,4,loopEndFound,1);

      PrintInt(14,6,SongBufBytes(),1);
      PrintInt(14,7,bpfLimiter,1);
      PrintInt(14,8,bufferLimiter,1);
      PrintInt(14,9,SONG_BUFFER_SIZE,1);

      UpdateEqualizer();
      CustomWaitVsync(1);
   /*   if(false && ++counter == 60*2){//time to do something else with the SD card

         counter = 0;
         while(!SongBufFull())//fill up the buffer to give us the longest possible time to read the other data
            SongBufWrite(sdCardGetByte());
         sdInUse = 1;//tell our CustomWaitVsync() not to touch the SD card, it is not at the offset it expects
         CustomWaitVsync(1);
         //we do not necessarily need to get things done in one frame, but it is best to get things done, restore
         //the song offset, and then call CustomWaitVsync() as often as possible throughout the code
         sdCardCueSectorAddress(sectorStart+otherOffset);//move to the start of the other data
         sdCardDirectReadSimple(&vram[100],16);//read "HELLO WORLD!"
         sdCardCueByteAddress((uint32_t)((sectorStart*512UL)+songOff));//restore the SD back to the song position
         sdInUse = 0;//tell the CustomWaitVsync() it can touch the SD card again
      }*/
   }
}

void CustomWaitVsync(u8 frames){//we do a best effort to keep up to the demand of the song player
   while(frames){
      if(loopEnd){//we read past the end of the song..luckily it is padded with bytes from the loop start
         loopEndFound = loopEnd;//just to display it once found(not needed for games)
         songOff = (songOff-loopEnd)+loopStart;
         loopEnd = 0;//since we immediately zero it so we don't keep doing it
	 Print(0,6,PSTR("G-GLITCH :"));
         sdCardStopTransmission();
   //WaitVsync(1);
         sdCardCueSectorAddress(sectorStart);//sdCardCueByteAddress((sectorStart*512UL)+songOff);
         sdCardSkipBytes(songOff);
      }

      u8 total_bytes = 0;
      while(!GetVsyncFlag()){//try to use cycles that we would normally waste
         if(!sdInUse){//we are clear to use the SD card, the other section has restored our offset
            if(!SongBufFull() && SongBufBytes() < bufferLimiter && total_bytes < bpfLimiter){
               SongBufWrite(sdCardGetByte());songOff++;
               //SongBufWrite(pgm_read_byte(&CompressedSong[songOff++]));
               total_bytes++;            
            }         
         }
      }
      ClearVsyncFlag();
      frames--;
   }
}

void UpdateEqualizer(){
return;
   for(uint8_t i=0;i<SCREEN_TILES_H;i++){//fade down equalizer and high point
      if(equalizer[i] < 8)
         equalizer[i] = 0;
      else
         equalizer[i] -= 8;

      if(equalizerHigh[i])
         equalizerHigh[i]--;
   }
      
   for(uint8_t j=0;j<CHANNELS;j++){//bump up equalizer based on active notes on tracks
      u8 center = (tracks[j].note)/(128/SCREEN_TILES_H);
      u8 mag = (tracks[j].noteVol)/1;
      if(center){
         if(equalizer[center-1] < 255-mag)
             equalizer[center-1] = mag;
      }

      if(center < SCREEN_TILES_H-1){
         if(equalizer[center+1] < 255-mag)
             equalizer[center+1] = mag;
      }
      if(equalizer[center] < 255-mag)
         equalizer[center] = mag;
   }

   for(uint8_t y=10;y<SCREEN_TILES_V;y++){
      for(uint8_t x=0;x<SCREEN_TILES_H;x++){      
         SetTile(x,y,0);

         if(equalizer[x]/(SCREEN_TILES_V-10) >= (SCREEN_TILES_V-y))
            SetTile(x,y,'=');
         if(equalizerHigh[x]/(SCREEN_TILES_V-10) >= (SCREEN_TILES_V-y))
            SetTile(x,y,'*');
      }
   }
}
