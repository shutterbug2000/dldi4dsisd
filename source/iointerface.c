/*
	iointerface.c template
	
 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. All derivative works must be clearly marked as such. Derivatives of this file 
	 must have the author of the derivative indicated within the source.  
  2. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// When compiling for NDS, make sure NDS is defined
#ifndef NDS
 #if defined ARM9 || defined ARM7
  #define NDS
 #endif
#endif

#ifdef NDS
 #include <nds/ndstypes.h>
#else
 #include "gba_types.h"
#endif

#define BYTES_PER_READ 512

#ifndef NULL
 #define NULL 0
#endif

#include <nds/disc_io.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <nds/system.h>

//---------------------------------------------------------------------------------
bool sdio_Startup() {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;

	fifoSendValue32(FIFO_SDMMC,SDMMC_HAVE_SD);
	

	while(!fifoCheckValue32(FIFO_SDMMC));

	int result = fifoGetValue32(FIFO_SDMMC);

	if(result==0) return false;

	fifoSendValue32(FIFO_SDMMC,SDMMC_SD_START);

	fifoWaitValue32(FIFO_SDMMC);

	result = fifoGetValue32(FIFO_SDMMC);
	
	return result == 0;
}

//---------------------------------------------------------------------------------
bool sdio_IsInserted() {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;

	fifoSendValue32(FIFO_SDMMC,SDMMC_SD_IS_INSERTED);

	fifoWaitValue32(FIFO_SDMMC);

	int result = fifoGetValue32(FIFO_SDMMC);

	return result == 1;
}

//---------------------------------------------------------------------------------
bool sdio_ReadSectors(sec_t sector, sec_t numSectors,void* buffer) {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;
	FifoMessage msg;

	DC_FlushRange(buffer,numSectors * 512);

	msg.type = SDMMC_SD_READ_SECTORS;
	msg.sdParams.startsector = sector;
	msg.sdParams.numsectors = numSectors;
	msg.sdParams.buffer = buffer;
	
	fifoSendDatamsg(FIFO_SDMMC, sizeof(msg), (u8*)&msg);

	fifoWaitValue32(FIFO_SDMMC);

	int result = fifoGetValue32(FIFO_SDMMC);
	
	return result == 0;
}

//---------------------------------------------------------------------------------
bool sdio_WriteSectors(sec_t sector, sec_t numSectors,const void* buffer) {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;
	FifoMessage msg;

	DC_FlushRange(buffer,numSectors * 512);

	msg.type = SDMMC_SD_WRITE_SECTORS;
	msg.sdParams.startsector = sector;
	msg.sdParams.numsectors = numSectors;
	msg.sdParams.buffer = (void*)buffer;
	
	fifoSendDatamsg(FIFO_SDMMC, sizeof(msg), (u8*)&msg);

	fifoWaitValue32(FIFO_SDMMC);

	int result = fifoGetValue32(FIFO_SDMMC);
	
	return result == 0;
}


//---------------------------------------------------------------------------------
bool sdio_ClearStatus() {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;
	return true;
}

//---------------------------------------------------------------------------------
bool sdio_Shutdown() {
//---------------------------------------------------------------------------------
	if (!REG_DSIMODE) return false;
	return true;
}

const DISC_INTERFACE __io_dsisd = {
	DEVICE_TYPE_DSI_SD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&sdio_Startup,
	(FN_MEDIUM_ISINSERTED)&sdio_IsInserted,
	(FN_MEDIUM_READSECTORS)&sdio_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&sdio_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&sdio_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&sdio_Shutdown
};


/*-----------------------------------------------------------------
startUp
Initialize the interface, geting it into an idle, ready state
returns true if successful, otherwise returns false
-----------------------------------------------------------------*/
bool startup(void) {	
	return sdio_Startup();
}

/*-----------------------------------------------------------------
isInserted
Is a card inserted?
return true if a card is inserted and usable
-----------------------------------------------------------------*/
bool isInserted (void) {
	return sdio_IsInserted();
}


/*-----------------------------------------------------------------
clearStatus
Reset the card, clearing any status errors
return true if the card is idle and ready
-----------------------------------------------------------------*/
bool clearStatus (void) {
	return sdio_ClearStatus();
}


/*-----------------------------------------------------------------
readSectors
Read "numSectors" 512-byte sized sectors from the card into "buffer", 
starting at "sector". 
The buffer may be unaligned, and the driver must deal with this correctly.
return true if it was successful, false if it failed for any reason
-----------------------------------------------------------------*/
bool readSectors (u32 sector, u32 numSectors, void* buffer) {
	return sdio_ReadSectors(sector,numSectors,buffer);
}



/*-----------------------------------------------------------------
writeSectors
Write "numSectors" 512-byte sized sectors from "buffer" to the card, 
starting at "sector".
The buffer may be unaligned, and the driver must deal with this correctly.
return true if it was successful, false if it failed for any reason
-----------------------------------------------------------------*/
bool writeSectors (u32 sector, u32 numSectors, void* buffer) {
	return sdio_WriteSectors(sector,numSectors,buffer);
}

/*-----------------------------------------------------------------
shutdown
shutdown the card, performing any needed cleanup operations
Don't expect this function to be called before power off, 
it is merely for disabling the card.
return true if the card is no longer active
-----------------------------------------------------------------*/
bool shutdown(void) {
	return sdio_Shutdown();
}
