/*
-> SYNC AA 0D 00 00 00 00 (bis zu 60 mal, 5ms+1ms*i interval)
<- ACK AA 0E 01 xx 00 00
-> initial RAW, VGA   AA 01 00 06 zz
<- ACK
-> Snapshot (uncompressed)  AA 05 01 00 00 00
<- ACK
-> GET PICTURE AA 04 01 00 00 00
<- ACK
<- DATA AA 0A 01 .. .. .. (image size)
-> ACK AA 0E 00 00 00 00 (package ID 0000h)
<- Image Data Package 512 bytes ID 0001h
<- ACK AA 0E 00 00 01 00 (package ID 0001h)
.
.
<- last data package
-> ACK AA 0E 00 00 F0 F0 (package ID F0F0h)
*/

static char * SYNC = "AA0D00000000";
static char * ACK = "AA0E01xx0000";

/**
 *
 */
int webcam_init(){
	int i = 0;
	int max_tries = 60;
	for (i = 0; i<max_tries; i++){
		send(SYNC);
		sleepMs(5 + i);
		receive();
		if(answer == ACK){

		}
	}
}

/**
 *
 */
int webcam_get(){


}

/**
 *
 */
int webcam_uninit(){



}
