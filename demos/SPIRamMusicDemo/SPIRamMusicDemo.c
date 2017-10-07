/*
 *
 *  SPI Ram Streaming Music Demo
 *  by Lee Weber(D3thAdd3r) 2017
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
//#include "data/tiles.inc"
#include "data/font-8x8.inc"
//#include "data/compressedsong.inc"
#include "data/patches.inc"
#include <sdBase.h>
#include <spiram.h>


const char fileName[] PROGMEM = "SD_MUSICDAT";

void CustomWaitVsync(u8 frames);
void UpdateEqualizer();

uint32_t songOff = 0;
extern s8 songSpeed;
extern bool playSong;
extern volatile u16 songPos;
extern volatile u16 loopStart;
extern volatile u16 loopEnd;
extern Track tracks[CHANNELS];
extern u8 ram_tiles[];
u8 songNum;
extern u16 songStalls;
extern u8 songBufIn,songBufOut;
u16 loopEndFound;
u16 bufferLimiter;
u16 bpfLimiter;
u32 songBase;
u8 doSongBuffer;
long sectorStart;


u16 padState, oldPadState;
u8 visualizer[SCREEN_TILES_H];
u8 visualizerHigh[SCREEN_TILES_H];
uint32_t streamPcmPos;
uint16_t streamPcmSize;

int main(){
	InitMusicPlayer(patches);
	SetTileTable(font);
	ClearVram();

	sdCardInitNoBuffer();
	SpiRamInit();

	if((sectorStart = sdCardFindFileFirstSectorFlash(fileName)) == 0){
		Print(0,0,PSTR("FILE SD_MUSIC.DAT NOT FOUND ON SD CARD"));
		while(1);	
	}

	//load all music resources from the SD card into SPI ram
	SetRenderingParameters(FRAME_LINES-10,8);//decrease the number of drawn scanlines for more free cycles to load data
	for(uint16_t i=0;i<32*2;i++){//load 512 bytes at a time from SD into ram, then from ram into SPI ram, for a total of 32K or 64 sectors

		sdCardCueSectorAddress(sectorStart+i);
		sdCardDirectReadSimple(vram+256,512);
		sdCardStopTransmission();
 		SpiRamWriteFrom((i*512)>>16,(i*512)&0xFF,vram+256,512);
	}
	ClearVram();//clear the gibberish we wrote over vram
	SetRenderingParameters(FIRST_RENDER_LINE,FRAME_LINES);//increase the number of drawn scanlines so the whole screen shows


	Print(0,0,PSTR("SONGNUM  :"));
	Print(0,1,PSTR("SONGPOS  :"));
	Print(0,2,PSTR("SONGSPEED:"));
	Print(0,3,PSTR("LOOPSTART:"));
	Print(0,4,PSTR("LOOPEND  :"));
	Print(0,5,PSTR("SONGBASE :"));

	Print(0,6,PSTR("BUFFERED :"));
	Print(0,7,PSTR("FILL RATE:"));
	Print(0,8,PSTR("BUF LIMIT:"));
	Print(0,9,PSTR("STALLS   :"));

	streamPcmPos = 0xFFFFFF;
	bufferLimiter = 16;//let the user simulate different buffer sizes in real time
	bpfLimiter = 16;//let the user simulate different fill speeds(for SPI ram it is likely ok to have it always fill the buffer fully)
	songSpeed = 0;
	songBase = SpiRamReadU32(0,0);//read the first entry from the directory, this is the offset to the first song
PrintInt(20,10,songBase,1);
while(1);
	SpiRamSeqReadStart(songBase>>16,songBase&0xFF);//make our buffering code start at the first byte of the song
	WaitVsync(1);
	doSongBuffer = 1;//let the CustomWaitVsync() fill the buffer. We did not want it to do that before, since it would be reading the directory
	CustomWaitVsync(2);//let it buffer some data
	/////////songStalls = 0;//the first CustomWaitVsync() will detect a stall, but that doesn't count because we weren't going yet
	StartSong();//start the song now that we have some data

	for(uint8_t j=0;j<64;j++){//load up some graphics for coloring
		ram_tiles[(0*64)+j] = pgm_read_byte(&font[(29*64)+j])&0b00101000;
		ram_tiles[(1*64)+j] = pgm_read_byte(&font[(10*64)+j])&0b00101101;
		ram_tiles[(2*64)+j] = pgm_read_byte(&font[(3*64)+j])&0b00000111;
	}

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

		if(padState & BTN_SL && !(oldPadState & BTN_SL))
			songSpeed--;

		if(padState & BTN_SR && !(oldPadState & BTN_SR))
			songSpeed++;

		if(padState & BTN_SELECT && !(oldPadState & BTN_SELECT)){
			songBufIn = songBufOut = 0;
			songOff = songBase;
			SpiRamSeqReadEnd();
			SpiRamSeqReadStart(songBase>>16,songBase&0xFF);
			StartSong();
			/*if(++songNum > 2)
				songNum = 0;
			SpiRamSeqReadEnd();//end the sequential read we were using to fill the buffer
			songBase = SpiRamReadU16(0,songNum);//read the start of this song
			SpiRamSeqReadStart(0,songBase);//start the sequential read again so buffering can happen
			while(SongBufBytes())
				SongBufRead();//eat any buffered bytes from the last song
			StartSong();*/
		}
		
		if(padState & BTN_B && !(oldPadState & BTN_B)){
			streamPcmPos = 0;
			streamPcmSize = 8000;
		}

		PrintInt(14,0,songNum,1);
		PrintInt(14,1,songPos,1);
		PrintInt(14,2,songSpeed,1);
		PrintInt(14,3,loopStart,1);
		PrintInt(14,4,loopEndFound,1);
		PrintInt(14,5,songBase,1);

		PrintInt(14,6,SongBufBytes(),1);
		PrintInt(14,7,bpfLimiter,1);
		PrintInt(14,8,bufferLimiter,1);
		PrintInt(14,9,songStalls,1);

		UpdateEqualizer();
		CustomWaitVsync(1);

	}
}

void CustomWaitVsync(u8 frames){//we do a best effort to keep up to the demand of the song player.

	while(frames){
		if(loopEnd){//we read past the end of the song..luckily it is padded with bytes from the loop start
			SpiRamSeqReadEnd();
			loopEndFound = loopEnd;//just to display it once found(not needed for games)
			songOff = (songOff-loopEnd)+loopStart;
			loopEnd = 0;//since we immediately zero it so we don't keep doing it

			//WaitVsync(1);//asm volatile("lpm\n\tlpm\n\t");
			SpiRamSeqReadStart((songBase+songOff)>>16,(songBase+songOff)&0xFF);//read from the start of the song, plus the offset we already "read past"
		}

		u8 total_bytes = 0;
		while(!GetVsyncFlag()){//try to use cycles that we would normally waste

			if(false && streamPcmPos != 0xFFFFFF){//playing a PCM, this is simple code that is not necessarily safe in cycle intensive games
		
				SpiRamSeqReadEnd();//stop the music read
 				SpiRamReadInto((streamPcmPos>>16),(streamPcmPos&0xFF),(mix_buf+(mix_bank*262)),streamPcmSize<262?streamPcmSize:262);//read the PCM data into the sound buffer
				streamPcmPos += 262;//update our position into the PCM sound(only 15khz sounds, no frequency/note changes in this example)
				if(streamPcmSize < 262){//sound is done
					streamPcmSize = 0;
					streamPcmPos = 0xFFFFFF;
				}				
				SpiRamSeqReadStart(songOff>>16,songOff&0xFF);//restore the SPI Ram to the state we found it in, so the music player can continue
			}

			if(doSongBuffer && !SongBufFull() && SongBufBytes() < bufferLimiter && total_bytes < bpfLimiter){
				SongBufWrite(SpiRamSeqReadU8());
				songOff++;				
				total_bytes++;				
			}		
		}

		ClearVsyncFlag();
		frames--;
	}
}


void UpdateEqualizer(){
return;
	for(uint8_t i=1;i<SCREEN_TILES_H-1;i++){
		//avg = 0;
		for(uint8_t j=0;j<9;j++)
			visualizer[i] += mix_buf[(262*mix_bank)+(i*9)+j]/2;

		visualizer[i] /= 8;//9;
		if(visualizer[i] > 2)
			visualizer[i] -= 3;
		else
			visualizer[i] = 0;
		if(visualizer[i] > visualizerHigh[i])
			visualizerHigh[i] = visualizer[i];
		else if(mix_bank && visualizerHigh[i])
			visualizerHigh[i]--;

		
		for(uint8_t j=0;j<17;j++){
			if(visualizer[i] > j)
				vram[i+((SCREEN_TILES_V-1-j)*VRAM_TILES_H)] = 0;//RAM_TILES_COUNT+29;
			else if(false)//visualizerHigh[i] > j)
				vram[i+((SCREEN_TILES_V-1-j)*VRAM_TILES_H)] = 1;//RAM_TILES_COUNT+10;
			else if(visualizerHigh[i] == j)
				vram[i+((SCREEN_TILES_V-1-j)*VRAM_TILES_H)] = 2;//RAM_TILES_COUNT+3;
			else
				vram[i+((SCREEN_TILES_V-1-j)*VRAM_TILES_H)] = RAM_TILES_COUNT+0;
		}

	}
}





