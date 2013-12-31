#include "projInfo.h"	//Don't include in main.h 'cause that's included in other .c's?
#include "main.h"


//Need to save memory space for the Tiny45...
// (Actually, now it fits.
//  FADER just uses the three remaining pins to cyclicly fade three LEDs
//  You can just ignore/disable it.)
#define FADER_ENABLED TRUE //FALSE

#if (defined(FADER_ENABLED) && FADER_ENABLED)
#include _SINETABLE_HEADER_
#endif


//For hacking when connected in single-pix mode
// (e.g. every-other pixel will either be black or white)
// Just comment-them-out when the single-to-dual converter is ready...
//#define H_RES	700L
// Unused:
//#define V_RES	1050

//A/O v48:
// I tend to use SwitchResX to test values, so set those values here.
// These are the values output by the GPU (single-pixel)
// They'll be converted to dual-pixel values as appropriate...
// (And apparently some of the math divides it in half then later
//  multiplies it by 2... so best not to have odd values here)
#define PIX_CLK_GPU	(100000000UL)

#define H_ACTIVE_GPU	(1400)		//Active pixels
#define H_FP_GPU		(32)			//Front Porch
#define H_WIDTH_GPU	(32)			//Sync Width
#define H_BP_GPU		(160)			//Back Porch
//a/o v48:
//Oddly, 16,16,64 worked with SwitchResX, but *here* they caused a
// horizontal shift... 120 seems to have fixed it.
// It could be that the display was already properly-synced from a
// different timing, then switching to the new timing allowed it to remain
// synced. Or maybe I've got some integer-rounding in here...? I dunno.
//a/o v51:
// 16,16,120 seems to work, but every once in a while I see a glitch in
// terminal (sharp contrast, I guess)

#define V_ACTIVE_GPU	(1050)		//Active Rows
#define V_FP_GPU		(0)			//Front Porch
#define V_WIDTH_GPU	(2)			//Sync Rows
#define V_BP_GPU		(6)			//Back Porch



//If you want to have alternate-timings available, set this TRUE and enter
//the alternate-values below.
#define ALT_TIMING	TRUE



#if (defined(ALT_TIMING) && ALT_TIMING)
// According to spwg, there's room for at least one "alternate timing"
// This could be useful here, for saving a couple values to try-out...
// e.g. 57Hz refresh worked perfectly, as far as I could tell, but I wanted
// to see if I could bump that up... 60Hz looked great, until I was working
// in Terminal (lots of contrast), and I noticed a few lines would skip...
// Hopefully, if this works, having an alternate-timing of the
// previously-working values will make switching back easy...
// Haven't yet figured out how I'm going to implement this...
// Alternate-timing kinda makes sense as the reliable fall-back
// OTOH, if the main timing is set to something non-bootable, then what?
// (If the experimental timing is put here in ALT_TIMING, then it could be
// tested *after* booting with reliable values... OTOH, I've *also* found
// that (maybe) the display itself has a better time syncing if it'd
// already synced since power-up... so switching to a new (questionable)
// value may work after the system has booted, but may not work upon a
// fresh boot... 

// Anyways, these are the same macro-names as above, just with _ALT.

#define PIX_CLK_GPU_ALT	(100000000UL)

//These values are for a 57Hz refresh
#define H_ACTIVE_GPU_ALT	(1400)		//Active pixels
#define H_FP_GPU_ALT			(32)			//Front Porch
#define H_WIDTH_GPU_ALT		(32)			//Sync Width
#define H_BP_GPU_ALT			(180)			//Back Porch

#define V_ACTIVE_GPU_ALT	(1050)		//Active Rows
#define V_FP_GPU_ALT			(2)			//Front Porch
#define V_WIDTH_GPU_ALT		(2)			//Sync Rows
#define V_BP_GPU_ALT 		(6)			//Back Porch

//a/o v51:
//These are values used in the actual EDID array...
// This avoids all single-to-dual translation, etc. that makes 
// it complicated to figure out where the NON-ALT values come from...
// (that stuff is, again, remnants of an older time, not relevent to most
// purposes)
#define H_BLANKING_ALT (H_FP_GPU_ALT + H_WIDTH_GPU_ALT + H_BP_GPU_ALT)
#define V_BLANKING_ALT (V_FP_GPU_ALT + V_WIDTH_GPU_ALT + V_BP_GPU_ALT)

#endif


//Currently, neither of these values can be greater than 255
// it should be a simple change.
#define H_IMAGE_SIZE_MM_LTD   245L
#define V_IMAGE_SIZE_MM_LTD	184L






//These are dual-pixel frequencies...
// e.g. the single-pixel transmitter (built into the mac)
// runs at twice this speed...
//15MHz looks nasty on boot, but macOS handles it fine
//20MHz Boots fine
//85MHz (single) looks great, only slight flicker (42.5MHz dual) = 47Hz
//SwitchResX Test at 53Hz = color inversion like the plasma...
// bit-shifts due to unmatched cable lengths?
// bounce due to termination placement?
//#define PIX_CLK_OVERRIDE (42500000L)
// As Of v47: 100MHz works, 104 causes jitter... (tested in SwitchResX)


//a/o v51: There've been some unusual testing in here...
//         e.g. one of the first experiments was to use a dual-pixel
//         display *directly* with the single-pixel GPU. Thus, the
//         horizontal resolution, according to the computer, was 1/2 the 
//			  physical resolution of the screen...
//         Later, this display was connected via a single-to-dual pixel
//         converter. At this point, the EDID information shouldn't rely in
//         any way on the fact that the display is a dual-pixel display
//         since it looks like a single-pixel display to the computer.
//         But the weird math remains here.
//         As a result of these strange tests, there's some weird stuff in
//         here regarding H_RES and PIX_CLK_OVERRIDE, etc.
//         I haven't yet removed it.
// e.g. As it's currently configured...
//      (note that _LTD is referring to my LTD121... LCD. NOT "limited")
//      PIX_CLK = PIX_CLK_TYP_LTD*2
//                PIX_CLK_TYP_LTD = PIX_CLK_OVERRIDE
//                                  PIX_CLK_OVERRIDE = PIX_CLK_GPU/2
// Thus: PIX_CLK = PIX_CLK_GPU (minus rounding error)
//
//      H_ACTIVE = H_PIXEL_CLOCKS_LTD*2
//                 H_PIXEL_CLOCKS_LTD = H_ACTIVE_GPU/2
// Thus H_ACTIVE = H_ACTIVE_GPU
//
// H_BLANKING = H_BLANKING_TYP_LTD*2
//              H_BLANKING_TYP_LTD = (H_FP_GPU+H_WIDTH_GPU+H_BP_GPU)/2 
// H_BLANKING = H_FP_GPU + H_WIDTH_GPU + H_BP_GPU
//
// V_ACTIVE = V_ACTIVE_GPU
//
// V_BLANKING = V_BLANKING_TYP
//					 V_BLANKING_TYP = V_FP_GPU + V_WIDTH_GPU + V_BP_GPU
//
// THFP = THFP_LTD*2
//        THFP_LTD = H_FP_GPU/2
// THFP = H_FP_GPU
//
// THW = THW_LTD*2
//       THW_LTD = H_WIDTH_GPU/2
// THW = H_WIDTH_GPU
//
// TVFP = V_FP_GPU
// TVW = V_WIDTH_GPU 


#define PIX_CLK_OVERRIDE (PIX_CLK_GPU/2) //(50000000L)


//WTF: 4196 Bytes in Text region, not complaining?!
//  Apparently somewhere in avr-gcc/libc...?
//  (avrdude gives error when trying to write...)
//int test[400] = {1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8};
/*int test(void)
{
	int i;
	int j,k;

	j=0;

	for(i=0; i<85; i++)
	{
		setbit(i%8, j);
		for(k=0; k<254; k++)
			setbit((k+i)%7, j);
	}

	return j;

}
*/


//This is the display's pixel-clock...
#ifndef PIX_CLK_OVERRIDE
#error "This error is just a message to me, in case my variables aren't set as I'd expected"
#define PIX_CLK_TYP_LTD (54000000L)
#else
#define PIX_CLK_TYP_LTD PIX_CLK_OVERRIDE
#endif

//This is the display's number of active pixel-clocks
#define H_PIXEL_CLOCKS_LTD   (H_ACTIVE_GPU/2)	//700
// and inactive...
#define H_BLANKING_TYP_LTD 	((H_FP_GPU+H_WIDTH_GPU+H_BP_GPU)/2)	//144

//Horiz Sync Offset (THFP)
//#define THFP_MIN_LTD (H_FP_GPU/2)	//8
#define THFP_LTD (H_FP_GPU/2) //(THFP_MIN_LTD*2)

//Horiz Sync Pulse-Width
//#define THW_MIN_LTD 	(H_WIDTH_GPU/2) // 8
#define THW_LTD  (H_WIDTH_GPU/2)//(THW_MIN_LTD*2)

#ifdef H_RES
#error "This error is just a message to me, in case I have H_RES still defined"
// #define H_IMAGE_SIZE_MM ((uint8_t)((H_IMAGE_SIZE_MM_LTD*H_RES)/1400L))
 #define H_IMAGE_SIZE_MM \
	((uint8_t)((H_IMAGE_SIZE_MM_LTD*H_RES)/(H_ACTIVE_GPU)))
 #define V_IMAGE_SIZE_MM	V_IMAGE_SIZE_MM_LTD

 #define PIX_CLK  (PIX_CLK_TYP_LTD)
 #define H_ACTIVE    (H_PIXEL_CLOCKS_LTD)
 #define H_BLANKING  (H_BLANKING_TYP_LTD)
 #define THFP (THFP_LTD)
 #define THW (THW_LTD)
#else
 #define H_IMAGE_SIZE_MM	H_IMAGE_SIZE_MM_LTD
 #define V_IMAGE_SIZE_MM	V_IMAGE_SIZE_MM_LTD

 #define PIX_CLK  (PIX_CLK_TYP_LTD*2)
 #define H_ACTIVE (H_PIXEL_CLOCKS_LTD*2)
 #define H_BLANKING  (H_BLANKING_TYP_LTD*2)
 #define THFP (THFP_LTD*2)
 #define THW (THW_LTD*2)
#endif










#if !defined(__AVR_ARCH__)
 #error "__AVR_ARCH__ not defined!"
#endif

//#include <string.h> //for strcmp

// WTF I left this as 15 bytes but wrote 256 via usbTinyI2c 
// and had no errors (in version 20)?!
//uint8_t edidArray[15]; 
// = {0x00,0xe1,0xd2,0xc3,0xb4,0xa5,0x96,0x87,8,9};





//R,G,B
volatile uint8_t ledState[3] = {0,0,0};
uint8_t ledIndex = 0;


#define EDIDARRAYLENGTH	128 //256


//Initial values stolen (and modified) from HV121P01-101
// Notes from SPWG spec 3.8
uint8_t edidArray[EDIDARRAYLENGTH] =
{
	//Addr	Data	//Values		Notes
//EDID Header
	[0x00]=	0,		//				EDID Header
	[0x01]=	255,
	[0x02]=	255,
	[0x03]=	255,
	[0x04]=	255,
	[0x05]=	255,
	[0x06]=	255,
	[0x07]=	0,

//Vendor/ Product ID Stuff...
	//ID Manufacturer Name
	// SPWG: "EISA manufacture code = 3 character ID (compressed ASCII)
	// edidv3: "2-byte representation of the monitor's manufacturer.
	// 	This is the same as the EISA ID. Based on compressed ASCII,
	//    0001=A...11010=Z."
	//    Bit:    7   6   5   4   3   2   1   0
	//
	//    Byte 1: 0 | 4   3   2   1   0 | 4   3
	//            * |      char 1       | char 2
	//
	//    Byte 2: 2   1   0 | 4   3   2   1   0
	//            char 2    |      char 3
	//    "EISA manufacturer ID's are issued by Microsoft" YAY!

//Must be uppercase char1-3
#define TOCOMPRESSEDASCII(char1,char2,char3) \
	      ((((char1)-'@')<<10) | (((char2)-'@')<<5) | ((char3)-'@'))
#define TOCOMPRESSEDASCIIBYTE1(char1,char2,char3) \
	      ((TOCOMPRESSEDASCII(char1,char2,char3))>>8)
#define TOCOMPRESSEDASCIIBYTE2(char1,char2,char3) \
	      ((TOCOMPRESSEDASCII(char1,char2,char3))&0xff)
	//[0x08]=	0x30,	//LEN			ID=LEN
	//[0x09]=	0xAE,	
	[0x08]=	TOCOMPRESSEDASCIIBYTE1('M','E','H'),
	[0x09]=	TOCOMPRESSEDASCIIBYTE2('M','E','H'),

	//ID Product Code
	// SPWG: "Pannel Supplier Reserved - Product Code"
	// edidv3: "vendor-assigned product code"
	//[0x0A]=	0x05,	//SXGA+		SXGA+ FFS
	//[0x0B]=	0x40,
	// My first product...
	//  should report <0x01><PROJ_VER>
	//  e.g. 0x01 0x30, which is kinda like version 1.30
	//  yeah-no. PROJ_VER=30 is not 0x30 it's 30
	//
#if ( (PROJ_VER > 99) || (MY_EDID_PRODUCT_NUM > 99) )
 #error "TOBCD(val) doesn't work with values > 99..."
 #error "...either Project Ver or Product Num: congrats on making it this far!"
#endif

#warning "Using PROJ_VER as the display's product-code causes new versions"
#warning " to no longer be compatible with SwitchResX's timings for the"
#warning " old version... Something to contemplate..."
	[0x0A]=	TOBCD(PROJ_VER),
	[0x0B]=	TOBCD(MY_EDID_PRODUCT_NUM), //0x01,

	//32-bit serial No.
	// (optional; 0 if unused)
	[0x0C]=	0x00,	//				(blank)
	[0x0D]=	0x00,
	[0x0E]=	0x00,
	[0x0F]=	0x00,

	//Week of manufacture
	[0x10]=	0x00,	//				(blank)
	//Year of manufacture (offset from 1990)
//	[0x11]=	16,	//2006		Manufactured in 2006
	[0x11]=  (COMPILE_YEAR - 1990),

//EDID Version Info "Panel ID structure version/revision"
	//EDID Structure Ver.
	[0x12]=	0x01,	//				EDID Ver 1.0
	//EDID Revision #
	[0x13]=	0x03,	//				EDID Rev 0.3 -- Same for E-EDID

//Display Parameters
	//Video input definition:
	// SPWG: "Vidio I/P definition = Digital I/P (0x80)"
	// 0x50 deduced from 0x50 = 80, assumed reversed in HV121 LCD specs...
	// But, apparently 0x80 was correct, according to SPWG
	// According to edidv3 "Digital input requires use of the EDID structure
	//  version 2" which doesn't appear to be the case (edidv3 is old?)
	// According to E-EDID Standard (newer?):
	// "Compatibility with monitors and systems that require EDID 
	//  structure 2 is achieved by allowing EDID structure 2 to 
	//  be included in E-EDID as two extensions residing at fixed locations"
	// Looking into E-EDID for now on...
	// Bit 7 = 1 : Digital
	//   Bits 6-1: Reserved = 0
  	//   Bit 0:    "DFP 1.x" "If set=1, Interface is signal compatible
	//                        with VESA DFP 1.x: TMDS CRGB, 1pix/clk,
	//								  up to 8bits/color MSB aligned, DE active high"
	//             NOT SET...
	//             So maybe edidv3 was right....?

	[0x14]=	0x80, //0x50,	//	"HEX=80 DEC=50" WTF?!			-------(????)

#define H_IMAGE_SIZE_CM ((H_IMAGE_SIZE_MM+5L)/10L)
#if (H_IMAGE_SIZE_CM > 255)
 #error "H_IMAGE_SIZE_CM is too large!"
#endif
	//Max H image size (cm rounded)
	[0x15]=	((uint8_t)H_IMAGE_SIZE_CM), //25,	//				25cm

	//Max V image size (rounded)
	[0x16]=	18,	//				18cm

	//Display Gamma
	// = (gamma * 100)-100
	// e.g. (2.2*100)-100=120 (SPWG)
	[0x17]=	120,	//0x78=2.2	Gamma curve = 2.2

	//Feature Support
	// SPWG: "no DPMS, Active Off, RGB, timing BLK 1 == 0x0A"
	// ????
	// Bits 7-5: standby, suspend, active-off
	// Bit 4-3: Display Type (b4=0, b3=1: RGB)
	// Bit 2: colorspace
	// Bit 1: Preferred Timing Mode
	//        "If this bit is set to 1, the display's preferred timing mode
	//         is indicated in the first detailed timing block.
	//         NOTE: Use of the preferred timing mode is required by
	//         EDID Structure version 1 Revision 3 and higher."
	// Bit 0: 0=GTF timings not supported
	[0x18]=	0x0A, //234,	//0xEA		RGB display, Preferred Timing Mode
	


//Panel Color Coordinates
	//Red/Green low bits (RxRy/GxGy)
	[0x19]=	175,	//0xAF		--- Red/Green Low Bits
	//Blue/White low bits (BxBy/WxWy)
	[0x1A]=	64,	//0x40		--- Blue/White low bits

	// Below:
	//  R/G/B/Wx = 0.xxx
	//  R/G/B/Wy = 0.xxx
	[0x1B]=	0x95,	// Red x high bits 95 149 0.584 
						// Red (x) = 10010101 (0.584) 
	[0x1C]=	0x56,	// Red y high bits 56 86 0.338 
						// Red (y) = 01010110 (0.338) 
	[0x1D]=	0x4A,	// Green x high bits 4A 74 0.292 
						// Green (x) = 01001010 (0.292) 
	[0x1E]=	0x8F,	// Green y high bits 8F 143 0.562 
						// Green (y) = 10001111 (0.562) 
	[0x1F]=	0x25,	// Blue x high bits 25 37 0.146 
						// Blue (x) = 00100101 (0.146) 
	[0x20]=	0x20,	// BLue y high bits 20 32 0.125 
						// Blue (y) = 00100000 (0.125) 
	[0x21]=	0x50,	// White x high bits 50 80 0.313 
						// White (x) = 01010000 (0.313) 
	[0x22]=	0x54,	// White y high bits 54 84 0.329 
						// White (y) = 01010100 (0.329) 


//Established Timings
	// SPWG suggests 0x00 for all three: "not used"
	// "Also, if any one-bit flag is not set in the Established Timing 
	//  block, this data can not be used to determine if that timing is
  	//  within the supported scanning frequency of the display - 
	//  only that it is not a Factory Supported Mode."
	// "Factory Supported Modes are defined as modes that are properly 
	//  sized and centered as the monitor is delivered from factory."
	// IOW: 0's is fine.
	//Established Timings I
	[0x23]=	0x00, //0x21,	// Established timing 1 21 33 ----
						// Bit 5: 640x480 @ 60Hz
						// Bit 0: 800x600 @ 60Hz
	//Established Timings II
	[0x24]=	0x00, //0x08,	// Established timing 2 08 8 ----
						// Bit 3: 1024x768 @ 60Hz
	//Established Timings III / Manufacturer's Timings
	// "Bits 6 → 0 (inclusive) of byte 3 are used to define manufacturer’s
  	//  proprietary timings, and may be used if a manufacturer wants to 
	//  identify such timings through the use of one-bit flags."
	// NOT required, and not recommended.
	[0x25]=	0x00,	// Established timing 3 00 0 ----

//Standard Timing ID (SPWG suggests all = 0x01: "not used"
	//Standard Timing #1
	// These values don't make any sense! (from HV121 spec)
	// iBook LTN display lists none
	//[0x26]=	0x81,	// 81 129    HV121:"Not Used"
	//[0x27]=	0x80,	// 80 128 
	[0x26 ... 0x27]= 0x01, //Not Used

	//Standard Timings #2-8 "Not Used"
	// (Each is a pair of bytes, as Timing #1 above)
	[0x28 ... 0x35]=	0x01,	// 01 1 

//Detailed Timing/Monitor Descriptor #1 (60Hz)
//"The first descriptor block shall be used to indicate the display's 
// preferred timing mode"
	// SPWG: Pixel Clock / 10,000
	//[0x36]=	0x20,	// LSB 84.80 MHz Main Clock
	//[0x37]=	0x21,	// MSB
	//LTD121: 51-57MHz 54typ
	// WORKING WITH IT DIRECTLY HERE...
	// (dual-pixel mode, with second pixel blank)

	//This is per EDID standard:
  #define CLK_SCALE   (10000L)

	[0x36]=	((PIX_CLK/CLK_SCALE)&0xff),
	[0x37]=	((PIX_CLK/CLK_SCALE)>>8),
	// Horizontal
	//0x578 = 1400, 0x0C8 = 200
	// SPWG Note 2:
	// "HA, byte 38h, is true active pixels"
	// "HA_pixClks, bytes 55/56, is HA for XGA 
	//  and HA/2 for WSXGA+ and above resolutions" this seems to be in error
	//[0x38]=	0x78, //120, //0x78	Hor Active = 1400 lower 8bits
	//[0x39]=	0xC8, //200, //0xC8	Hor Blanking = 200 lower 8bits (Thbp)
	//[0x3A]=	((0x05<<4) | (0x00)),	//0x50 80

	//4bits Hor Active + 4bits Hor Blanking upper4:4bits
	// In pixel-clocks
	[0x38]=	((H_ACTIVE)&0xff),
	[0x39]=	((H_BLANKING)&0xff),
	[0x3A]=	(((H_ACTIVE)&0xf00)>>4) | (((H_BLANKING)&0xf00)>>8),


//#warning "LTD-DIRECT"
// In lines
#define V_ACTIVE		(V_ACTIVE_GPU) //1050
#define V_BLANKING_TYP	(V_FP_GPU+V_WIDTH_GPU+V_BP_GPU) //16
#define V_BLANKING (V_BLANKING_TYP)
	// Vertical
	//0x41A = 1050, 0x00A = 10
	//[0x3B]=	0x1A,	//26	Ver Active = 1050
	//[0x3C]=	0x0A, //10	Ver Blanking = 10 (Tvbp)
						// SPWG: "DE Blanking typ. For DE only panels" ???
	//[0x3D]=	((0x04<<4) | (0x00)),	//0x40 64
						//4bits Ver Active + 4bits Ver Blanking
	[0x3B]=  ((V_ACTIVE)&0xff),
	[0x3C]=  ((V_BLANKING)&0xff),
	[0x3D]=  (((V_ACTIVE)&0xf00)>>4) | (((V_BLANKING)&0xf00)>>8),


	//[0x3E]=	20,	//Hor Sync Offset = 20 pixels (Thfp)
						//"Pixels, from blanking starts, lower 8 bits"
	[0x3E]= ((THFP)&0xff),

	//[0x3F]=	116,	//Hor Sync Pulse Width = 116 pixels
	[0x3F]= ((THW)&0xff),

	//Vertical Sync Offset (TVFP)
//#define TVFP_MIN	(V_FP_GPU) //0
#define TVFP	(V_FP_GPU) //(TVFP_MIN + 2)
	//Vertical Sync Pulse Width
//#define TVW_MIN	(V_WIDTH_GPU) //2
#define TVW	(V_WIDTH_GPU) //(TVW_MIN*2)
	// HV121 spec seems to be misleading... should be (?)
	// [0x40]=((Voff&0x0f)<<4) | (Vpw&0x0f)
	// [0x41]=((Voff&0x30)<<6) | ((Vpw&0x30)<<4) 
	//        | (Hpw&0x300)>>6) | ((Hoff&0x300)>>8) --ish
//	[0x40]=	37,	//0x25	2	Vert Sync Offset = 2 lines  ??? (Tvfp)
						//SPWG: "Offset=xx lines    Sync Width=xx lines"
	[0x40]=	(((TVFP)&0x0f)<<4) | ((TVW)&0x0f),

//	[0x41]=	0,		//0x00	5	Vert Sync Pulse Width = 5 lines
						//SPWG: "Horiz Vert Sync Offset/Width upper 2 bits"

#if( ((THFP)&0xffffff00) | ((THW)&0xffffff00) \
		| ((TVFP)&0xfffffff0) | ((TVW)&0xfffffff0) )
	#error "Byte 0x41 (HSYNC/VSYNC Offset/Pulsewidth upper bits NYI"
#endif

	[0x41]=	0,

#if (H_IMAGE_SIZE_MM > 0xff)
	#error "LCDreIDer doesn't yet parse H_IMAGE_SIZE_MM > 255"
	#error "It should be a simple modification..."
#else
	#define H_IMAGE_SIZE_MM_LOWBYTE	H_IMAGE_SIZE_MM
#endif

#if (V_IMAGE_SIZE_MM > 0xff)
	#error "LCDreIDer doesn't yet parse V_IMAGE_SIZE_MM > 255"
	#error "It should be a simple modification..."
#else
	#define V_IMAGE_SIZE_MM_LOWBYTE	V_IMAGE_SIZE_MM
#endif


	[0x42]=	H_IMAGE_SIZE_MM_LOWBYTE,
	[0x43]=	V_IMAGE_SIZE_MM_LOWBYTE,
		//0xB8,	//Vertical Image Size = 184mm (low 8 bits)
	[0x44]=	0x00,	// 4 bits of Hor Image Size + 4 bits of Ver Image Size
	[0x45]=	0x00,	//Horizontal Border = 0 pixels
	[0x46]=	0x00,	//Vertical Border = 0 lines

	//Flags (bits 6,5,4,3 repurpose other bits! Don't change)
	// Bit7:		0=Non-Interlaced
	// Bit6,5:	00=non-stereo
	// Bit4,3:	11=Digital Separate (as opposed to composite)
	// Bit2:		Vertical Polarity: 1=Vsync is positive
	// Bit1:		Horizontal Polarity: 1=Hsync is positive
	// Bit0:		x (don't care, when non-stereo) E-EDID WTF See below, SPWG:
	//[0x47]=	0x19,	// ----- ????
						// SPWG: "Non-interlaced, Normal, no stereo, Separate
						//        sync, H/V pol Negatives" = 18
						//       "DE only note: LSB is set to '1' if panel is
						//        DE-timing only. H/V can be ignored" = 19
						// Indeed: HV121 specs seem to be DE-only
						//        "Features: Data enable signal mode"
						//        Also, timing diagrams do not show H/V
	[0x47]=	0x18,


//E-EDID:
//Notes regarding EDID Monitor Range Limits Descriptor:
//		Use of this descriptor is mandatory. 
//		Any timing outside these limits may cause the monitor to enter a 
//		self-protection mode. The host shall always verify that an intended 
//		timing falls within these limits before the timing is applied.
// My Notes:
//  This does not seem to be implemented in any of the displays 
//  I've checked.
//  Nor is it mentioned in SPWG
//But E-EDID mentions it twice!
// 1) All blocks shall be filled with valid data using the formats
//		described in sections 3.10.2 and 3.10.3.  Use of a data 
//		fill pattern is not permitted. 
// 2) Timing data must represent a supported mode of the display. 
//	3) Descriptor blocks shall be ordered such that all detailed timing
//		blocks precede other  types of descriptor blocks 
//	4) The first descriptor block shall be used to indicate the display's
//		preferred timing mode.  This is described in section 3.10.1 
//	5) A Monitor Range Limits Descriptor must be provided 
//	6) A Monitor Name Descriptor must be provided 
//Note:  Items 4, 5 and 6 above were permitted but not required prior to 
//		EDID structure version 1 revision 3.  Hosts may encounter displays 
//		using EDID version 1 revision 0-2 which do not meet all of these 
//		requirements.
// Yeah, but All the screens I've checked report to be EDID V1R3...



// Detailed timing/monitor descriptor #2 Alternative Panel Timing 
	// SPWG: "Timing Descriptor #2 may be used for timings other than 60Hz"
	// HV121: (dummy)
	//SPWG shows Timing Descriptor #2 as identically-formatted to #1
	// With the exception of the last byte...
	//  byte 0x59 "Module 'A' Revision = Example 00, 01, 02, 03, etc."
	// (isn't the info from Timing1's last byte necessary here too?!)
#if (!defined(ALT_TIMING) || !ALT_TIMING)
	[0x48 ... 0x4A] = 0,
	[0x4B]=	0x10,	// Dummy Descriptor
	[0x4C ... 0x59] = 0,
#else
	//These are identical to Timing Descriptor #1, bytes 0x36-0x47
	//with the exception of byte 0x59... (TBD)
	// AND timing macros are suffixed with _ALT,
	// e.g. PIX_CLK -> PIX_CLK_ALT

	[0x48]=	((PIX_CLK_GPU_ALT/CLK_SCALE)&0xff),
	[0x49]=	((PIX_CLK_GPU_ALT/CLK_SCALE)>>8),

	[0x4A]=	((H_ACTIVE_GPU_ALT)&0xff),
	[0x4B]=	((H_BLANKING_ALT)&0xff),
	[0x4C]=	(((H_ACTIVE_GPU_ALT)&0xf00)>>4) | \
				(((H_BLANKING_ALT)&0xf00)>>8),

	[0x4D]=  ((V_ACTIVE_GPU_ALT)&0xff),
	[0x4E]=  ((V_BLANKING_ALT)&0xff),
	[0x4F]=  (((V_ACTIVE_GPU_ALT)&0xf00)>>4) | \
				(((V_BLANKING_ALT)&0xf00)>>8),

	[0x50]= ((H_FP_GPU_ALT)&0xff),
	[0x51]= ((H_WIDTH_GPU_ALT)&0xff),
	[0x52]= (((V_FP_GPU_ALT)&0x0f)<<4) | ((V_WIDTH_GPU_ALT)&0x0f),

#if(H_FP_GPU_ALT > 0xff)
#error "H_FP_GPU_ALT > 0xff is not yet implemented"
#endif
#if (H_WIDTH_GPU_ALT > 0xff)
#error "H_WIDTH_GPU_ALT > 0xff is not yet implemented"
#endif
#if (V_FP_GPU_ALT > 0xf)
#error "V_FP_GPU_ALT > 0xf is not yet implemented"
#endif
#if (V_WIDTH_GPU_ALT > 0xf)
#error "V_WIDTH_GPU_ALT > 0xf is not yet implemented"
#endif
	[0x53]=	0, //Upper bits are not implemented

	[0x54]=	H_IMAGE_SIZE_MM_LOWBYTE,
	[0x55]=	V_IMAGE_SIZE_MM_LOWBYTE,
		//0xB8,	//Vertical Image Size = 184mm (low 8 bits)
	[0x56]=	0x00,	// 4 bits of Hor Image Size + 4 bits of Ver Image Size
	[0x57]=	0x00,	//Horizontal Border = 0 pixels
	[0x58]=	0x00,	//Vertical Border = 0 lines


	//This byte is different from 0x47, according to spwg:
	// "Module “A” Revision =          Example: 00, 01, 02, 03, etc."
	// (Doesn't it need DE, polarity, etc?!)
	[0x59]=	0,
#endif
// Detailed timing/monitor descriptor #3
	//SPWG: Shows #3 as same-format as HV121's #4
	// SPWG notes placed there
	// SPWG apparently doesn't have a descriptor matching this one...
	//[0x5A ... 0x5C] = 0,	//SPWG: "Flag" (all 0)
	//[0x5D]=	0x0F,	//Manufacturer Specified
						//  SPWG: Dummy Descriptor=0xFE
	//[0x5E]=	0x00,	//
	//[0x5F]=	144,	//1440
	//[0x60]=	0x43,	//4:3
	//[0x61]=	50,	//50Hz
	//[0x62 ... 0x64] = 0,	//"Not Supported"
	//[0x65]=	19,	//190 (what?)
	//[0x66]=	2,		//FFS (what??)
	//[0x67]=	0,		//"Reserved"
	// BOE ???
	//[0x68]=	0x09,
	//[0x69]=	0xE5,	
	//[0x6A ... 0x6B] = 0,

	//Adding another Dummy Descriptor:
	[0x5A ... 0x5C] = 0,
	[0x5D] = 0x10,
	[0x5E ... 0x6B] = 0,

// Detailed timing/monitor descriptor #4
	// Product Name Tag (ASCII)
	[0x6C ... 0x6E] = 0x00,	//SPWG: "Flag"
	[0x6F]= 0xFC,	//Let's make this a monitor-name instead of ASCII Data	
			//0xFE,				//SPWG: "Dummy Descriptor" NOT: FE = ASCII Data
	[0x70]=	0x00,				//SPWG: "Flag"

	// Model Name: mehLTD121KM7K //HV121P01-101
	[0x71]=	'm', //'H',				//SPWG: PC Maker P/N 1st Character
	[0x72]=	'e', //'V',				//SPWG: PC Maker P/N 2nd Character
	[0x73]=	'h', //'1',				//SPWG: PC Maker P/N 3rd Character
	[0x74]=	'L', //'2',				//SPWG: PC Maker P/N 4th Character
	[0x75]=	'T', //'1',				//SPWG: PC Maker P/N 5th Character
	[0x76]=	'D', //'P',				//SPWG: LCD Supplier Revision #
	[0x77]=	'1', //'0',				//SPWG: Manufacturer P/N
	[0x78]=	'2', //'1',				//SPWG: Manufacturer P/N
	[0x79]=	'1', //'-',				//SPWG: Manufacturer P/N
	[0x7A]=	'K', //'1',				//SPWG: Manufacturer P/N
	[0x7B]=	'M', //'0',				//SPWG: Manufacturer P/N
	[0x7C]=	'7', //'1',				//SPWG: Manufacturer P/N
	[0x7D]=	'K', //0x0A,				//SPWG: Manufacturer P/N
									//SPWG: "(if <13 char, terminate with ASCII
									//       code 0Ah, set remaining char = 20h"
									// Apparently referring to ALL characters
									//   0x71-0x7D

	//SPWG Timing Descriptor #4:
	//0x6C-6E	0x00	Flag
	//0x6F		0xFE	Data Type Tag
	//0x70		0x00	Flag
	//0x71-75	XX		SMBUS Value = XX nits
	//0x76-77	XXX	SMBUS Value = XXX nits
	//0x78		XXX	SMBUS Value = max nits (Typically 00h, XXX nits)
	//0x79		1/2	Number of LVDS channels (1 or 2)
	//0x7A		0/1	Panel Self Test (00-not present, 01-present)
	//0x7B      0Ah	(less than 13 char, so terminate)
	//0x7C-7D	20h	(remaining characters, terminated above)

//The End.
	[0x7E]=	0,		//"Extension Flag"
						//SPWG: "Number of optional 128 panel ID extension blocks
						//      to follow, Typ=0"
	[0x7F]=	0x75	// CHECKSUM
						//SPWG: "The 1-byte sum of all 128 bytes in this panel
						//       ID block shall = 0"
};


uint8_t edidArrayIndex = 0;
//Called immediately by the i2c interrupts when a byte is received
static __inline__
void processReceivedByte(uint8_t receivedByte, uint8_t byteNum)
	__attribute__((__always_inline__));
//Called immediately by the i2c interrupts when a byte is to be loaded
// for transmission
static __inline__
uint8_t nextByteToTransmit(uint8_t masterACKed)
	__attribute__((__always_inline__));;




#define USICNT_MASK ((1<<USICNT3)|(1<<USICNT2)|(1<<USICNT1)|(1<<USICNT0))
// There are four basic states: 
//    Awaiting Start Condition
//    ACK Transmission
//		Address
//    Data (read/write)
// AWAITING values are set... others are read... (helps for readability)
#define USI_STATE_AWAITING_START		0
#define USI_STATE_AWAITING_START_SCL 1
#define USI_STATE_START_SCL_RECEIVED USI_STATE_AWAITING_START_SCL
#define USI_STATE_AWAITING_ADDRESS	2
#define USI_STATE_ADDRESS_RECEIVED 	USI_STATE_AWAITING_ADDRESS	
#define USI_STATE_AWAITING_ACK		3
#define USI_STATE_ACK_COMPLETE		USI_STATE_AWAITING_ACK
#define USI_STATE_AWAITING_BYTE	   4
#define USI_STATE_BYTE_COMPLETE		USI_STATE_AWAITING_BYTE

//uint8_t usi_i2c_deviceAddressReceived = FALSE;
uint8_t usi_i2c_byteToTransmit = 0xff; //0xA5;
uint8_t usi_i2c_state = USI_STATE_AWAITING_START;
uint8_t usi_i2c_readFromSlave = 0;
uint8_t usi_i2c_requestedAddres = 0;
uint8_t usi_i2c_receivedByte = 0;

#define SDA_PIN	PB0
#define SCL_PIN	PB2
#define SDAPORT	PORTB
#define SCLPORT	PORTB

// This can be called in two cases:
//   upon INIT
//   and when a different slave has been addressed
//      from an overflow interrupt
void usi_i2c_awaitStart(void)
{
	usi_i2c_state = USI_STATE_AWAITING_START;

   //Clear the interrupt flags, etc. first...
	// NOTE This will release SCL hold (i.e. different slave addressed)
   USISR = (1<<USISIF) //Start Condition Interrupt Flag
                       //  (ONLY cleared when written 1)
         | (1<<USIOIF) //Counter Overflow Interrupt Flag
                       //  (ONLY cleared when written 1)
         | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
         | (0<<USIDC)  //Data Collision Flag (Read-Only, not an interrupt)
         | (USICNT_MASK & 0); //Clear the USI counter


   //Configure the USI to look for Start-Condition
   USICR = (1<<USISIE) //Enable the start-condition interrupt
         | (0<<USIOIE) //Disable the counter-overflow interrupt
         | (1<<USIWM1) //Enable two-wire mode 
							  // with SCL hold during start-condition
         | (0<<USIWM0) // without SCL hold during overflow
							  // will be set later
         | (1<<USICS1) //Select external clocking
         | (0<<USICS0) //  positive-edge
         | (0<<USICLK) //  4-bit counter counts on both external edges
         | (0<<USITC); // DON'T toggle the clock pin 
                       // (this should always be 0)

	setpinPORT(SDA_PIN, SDAPORT);
	setpinPORT(SCL_PIN, SCLPORT);

	setinPORT(SDA_PIN, SDAPORT);
	//Enable SCL-hold during the start-condition 
	setoutPORT(SCL_PIN, SCLPORT);
}



//Initialize the counter to count
//Clear the counter-overflow flag to
//Release SCL
// 16 clock edges will signal 8 bits received (count=0)
// 2 clock edges will signal ACK transmitted (count=14)
#define USI_I2C_OVERFLOW_RELEASE_SCL_AND_SET_COUNTER(count) \
   (USISR = ((1<<USIOIF) | (USICNT_MASK & (count))))

//This is called after a start-condition
void usi_i2c_awaitStartSCL(void)
{
	usi_i2c_state = USI_STATE_AWAITING_START_SCL;

	//Before enabling the counter-overflow interrupt
	// make sure the flag is clear
	// this will also clear the counter... watch out!
	USISR = (1<<USIOIF);

   //Configure the USI to look for counter-overflow (start SCL received)
   USICR = (1<<USISIE) //Enable the start-condition interrupt
         | (1<<USIOIE) //Enable the counter-overflow interrupt
                       // WE'RE READY!!!
         | (1<<USIWM1) //Enable two-wire mode
         | (1<<USIWM0) // with SCL hold during overflow
         | (1<<USICS1) //Select external clocking
         | (0<<USICS0) //  positive-edge
         | (0<<USICLK) //  4-bit counter counts on both external edges
         | (0<<USITC); // DON'T toggle the clock pin 
                       // (this should always be 0)

	//Don't clear the USISIF, SCL should be released AFTER it's received!
	// ONE SCL change (high to low) should be counted before we're
	//   before we're ready for 16 edges signalling address+r/w received
	//APPARENTLY the USI_START_vect ISR is called until USISIF is cleared
	// Unless the start-SCL occurs before the overflow interrupt is enabled
	//  this should have the same effect (the overflow interrupt stalls SCL)
	// However, if the start-SCL is too fast to detect, then we're screwed
	// all of, what, an interrupt-jump and a few instructions?
	//  Maybe revisit the zipped version (using a while loop instead of
	//   states)
	// also inline this function...
	USISR = (1<<USISIF) | (USICNT_MASK & 15);
	
//	USISR = (USICNT_MASK & 15);
//	static uint8_t callCount = 0;
//	if(callCount < 15)
//		callCount++;
//	set_heartBlink(callCount);
}



void usi_i2c_slaveInit(void)
{
	usi_i2c_awaitStart();
}

uint8_t heartBlinkInternal = 0;

ISR(USI_START_vect)
{
	// The start condition interrupt occurs when SDA is pulled low
	//   WHEN SCL is high...
	// Which means that the device will count an edge when SCL goes low...
	//   Thus, the number of clock edges will be 17 for the first 8 bits!!!
	//THIS IS A HACK AND SHOULD NOT BE IMPLEMENTED:
//	while(getpinPORT(SCL_PIN, SCLPORT))
//	{};
	// Should be fixed now, with the new AWAITING_START_SCL state

	heartBlinkInternal++;
	//set_heartBlink(heartBlinkInternal);

	//A Start-Detection has occurred
	// The slaves (including this device) have pulled SCL low
	// (from the manual):
	//  The start detector will hold the SCL line low after the master has
	//  forced a negative edge on this line. This allows the slave to wake
	//  up from sleep or complete other tasks before setting up the USI 
	//  Data Register to receive the address. This is done by clearing the 
	//  start condition flag and resetting the counter. 

	//NOT CERTAIN this is necessary
	// Shouldn't be, since the pin is set as an input...
	//   unless a START is received in the middle of a slave-write???
	// From the diagram showing the connections of the shift-registers
	// it seems we need this to prevent data collision
	//setinPORT(SDA_PIN, SDAPORT);
	//USIDR = 0xff;	

	//Set the pin directions...
	// Make sure, just in case a start-condition interrupts a slave-write...
	setinPORT(SDA_PIN, SDAPORT);
	// Leave the CLOCK pin active for SCL-hold at the first overflow
	// (once we've received address/direction)
	setoutPORT(SCL_PIN, SDAPORT);

	//Indicate that the address hasn't yet been received
	// (it should be the first byte transmitted)
	usi_i2c_awaitStartSCL();

	//heartClear();
}

//shifted right once to account for R/W bit...
#define USI_I2C_MYEDIDADDRESS 0x50
#define USI_I2C_MYLEDADDRESS	0x60

#define usi_i2c_isMyAddress(addr) \
	( ((addr) == USI_I2C_MYEDIDADDRESS)\
	| ((addr) == USI_I2C_MYLEDADDRESS) )


// In case this device handles multiple addresses... (NYI)
uint8_t usi_i2c_requestedAddress = 0;

uint8_t byteNum = 0;

//Basically, everything after the start-condition
// ACK and bytes...
ISR(USI_OVF_vect)
{
	//The USI counter has overflowed... this occurs in a couple cases..
	// First: After the address and r/w bit have been received
	// Second: After the ACK bit has been transmitted (by the slave)
	// Third: After the first data-byte has been received
	// Fourth: After the ACK bit has been transmitted for it...
	// and so on...

	//The counter-overflow also pulls SCL Low (from the manual):
	// After eight bits containing slave address and data direction 
	// (read or write) have been transferred, the slave counter overflows 
	// and the SCL line is forced low. 

	// If the slave is not the one the 
	// master has addressed, it releases the SCL line and waits for a new 
	// start condition.


	//These cases are the same as AWAITING, except if we're here,
	// then we're no longer awaiting them...
	switch(usi_i2c_state)
	{
		// The start condition interrupt occurs immediately when SDA goes low
		//  while SCL is high. The SCL line needs to go low once before
		//  data can be transmitted (this was a BITCH to find)
		//  in other words, there are 17 clock edges between the start-
		//  condition and having fully received the r/w bit
		case USI_STATE_START_SCL_RECEIVED:
	//		usi_i2c_awaitAddress();
			usi_i2c_state = USI_STATE_AWAITING_ADDRESS;

		   //Clear the Start-Condition flag to release SCL
			// ALSO the Overflow flag for the same reason
			// ALSO: set the counter
			//   16 clock edges will signal 8 bits received (address+r/w)
			USISR = (1<<USISIF) //Start Condition Interrupt Flag
										//  (ONLY cleared when written 1)
					| (1<<USIOIF)	//Overflow flag 
		         | (USICNT_MASK & 0); //Clear the USI counter

			break;
		case USI_STATE_ADDRESS_RECEIVED:
			{};

			byteNum = 0;
			//heartClear();
			//Check to see if it's ours...
			uint8_t udrTemp = USIDR;
			//1 = master-read (slave-writes to SDA)
			//0 = master-write (slave-reads from SDA)
			usi_i2c_readFromSlave = udrTemp & 0x01;
			usi_i2c_requestedAddress = udrTemp >> 1;
			
			//From the manual:
			// When the slave is addressed, it holds the SDA line low during 
			// the acknowledgment cycle before holding the SCL line low again
			if( usi_i2c_isMyAddress(usi_i2c_requestedAddress) )
			{
				// Send ACK:
				// This is a hack:
				//  We get to this point on a falling-edge
				//  the USIDR bit 7 is loaded 0
				//  then the next rising edge it changes to bit 6
				//  the next falling edge triggers the AWAITING_ACK case below
				//  which releases bit 6 from the SDA pin
				//  The master pulls it low again for a STOP condition
				// It might be wiser to just set the PORT value...
				//USIDR = 0x3f; 
				USIDR = 0x00;
				setoutPORT(SDA_PIN, SDAPORT);

	         usi_i2c_state = USI_STATE_AWAITING_ACK;

	         //FTM:
	         // (The USI Counter Register is set to 14 before releasing SCL)
				USI_I2C_OVERFLOW_RELEASE_SCL_AND_SET_COUNTER(14);
			}
			else //Another slave was addressed
				usi_i2c_awaitStart();

			break;
		//ACK has been transmitted from/to this device...
		case USI_STATE_ACK_COMPLETE:
			if(!usi_i2c_readFromSlave)	//slaveRead
			{
				//Release SDA from the ACK (we'll be reading...)
				setinPORT(SDA_PIN, SDAPORT);
				//Shouldn't be necessary with above, but can't hurt...
				//UDR = 0xff; 
			}
			else //slaveWrite
			{
				uint8_t masterACK;
				// First ACK will be this device's response to address+r/w
				// Read the master's ACK?
				// I guess it'd be stupid to send a read request without
				// actually reading a byte, but I've done it for testing...
				// but if that's the case, then the byteToTransmit will be 
				// decremented anyhow... Not sure how to handle this yet...
				// OTOH: if a read is requested a byte MUST be transferred
				// otherwise a bit-7 = 0 loaded would prevent a master-stop
				masterACK = !(USIDR & 0x01);

				if(byteNum)// && !(USIDR & 0x01))
				{
					heartBlinkInternal+= 0x10;
					//set_heartBlink(heartBlinkInternal);
				}

				//Load the byte to write to the master...
				//USIDR = usi_i2c_byteToTransmit;

				//if we load the next byte and its bit7 is 0
				// it will hold SDA low when the master tries to 
				// pull it high for a stop-condition!
				if(masterACK)
					USIDR = nextByteToTransmit(masterACK);
				else
					USIDR = 0xff;

				//Decrement IF the master sent ACK...
				//if(masterACK)
				//	usi_i2c_byteToTransmit--;
				
				setoutPORT(SDA_PIN, SDAPORT);
			}

			usi_i2c_state = USI_STATE_AWAITING_BYTE;

			USI_I2C_OVERFLOW_RELEASE_SCL_AND_SET_COUNTER(0);
			//heartClear();
			break;
		case USI_STATE_BYTE_COMPLETE:  //BYTE transmitted/received
			byteNum++;
			// Get the byte (if receiving)
			if(!usi_i2c_readFromSlave)
			{
				processReceivedByte(USIDR, byteNum);
				//usi_i2c_receivedByte = USIDR;
				heartBlinkInternal = usi_i2c_receivedByte;
				//set_heartBlink(heartBlinkInternal);
//				heartClear();

				// Also, we need to send an ACK...
				USIDR = 0x3f;
				setoutPORT(SDA_PIN, SDAPORT);
			}
			else //slaveWrite
			{
				// The master sends the ACK in this case...
				setinPORT(SDA_PIN, SDAPORT);
				USIDR = 0xff;
//				heartClear();
			}

			//Reload the counter to interrupt after the ACK
			USI_I2C_OVERFLOW_RELEASE_SCL_AND_SET_COUNTER(14);

			usi_i2c_state = USI_STATE_AWAITING_ACK;
			//heartClear();
         break;
		//Shouldn't get here...
		default:
			usi_i2c_state = 0x77;
			//set_heartBlink(0x77);
			break;
	}


	//I want it to blink if it's not yet read the EDID...
	if((usi_i2c_readFromSlave)
		 && (usi_i2c_state == 4)
		 && (usi_i2c_requestedAddress == USI_I2C_MYEDIDADDRESS))
		set_heartBlink(0);
	//set_heartBlink((!(usi_i2c_readFromSlave)<<4) | usi_i2c_state);
}

uint8_t ledsControlled = FALSE;

#define incrementEDIDIndex()	\
{\
	edidArrayIndex++;\
	if(edidArrayIndex == EDIDARRAYLENGTH) \
		edidArrayIndex = 0; \
}

#define setEDIDIndex(index) \
{\
	edidArrayIndex = index;\
	edidArrayIndex%=EDIDARRAYLENGTH;\
}

#define incrementLEDIndex() \
{\
	ledIndex++;\
	if(ledIndex == 3)\
		ledIndex = 0;\
}


//Interesting I haven't run into this before...
//	if(blah)
//		setLEDIndex(blah);
//	else
//	{ blah }
// APPARENTLY the semicolon causes the if statement to be closed (?)
#define setLEDIndex(index) \
{\
	ledIndex = index;\
	ledIndex %= 3;\
}

void processReceivedByte(uint8_t receivedByte, uint8_t byteNum)
{
  if(usi_i2c_requestedAddress == USI_I2C_MYEDIDADDRESS)
  {
	if(byteNum == 1)
	{
		// Could be that the EDID request doesn't send a byte-index...?
		//ledsControlled = FALSE;
		setEDIDIndex(receivedByte);
//		edidArrayIndex = receivedByte;
	}
	else
	{
		edidArray[edidArrayIndex] = receivedByte;

		incrementEDIDIndex();
		//edidArrayIndex++;
		//edidArrayIndex &= 0x07;
	}
  }
  else if (usi_i2c_requestedAddress == USI_I2C_MYLEDADDRESS)
  {
	
	ledsControlled = TRUE;

	if(byteNum == 1)
	{
		setLEDIndex(receivedByte);
	}
	else
	{
		ledState[ledIndex] = receivedByte;
		incrementLEDIndex();
	}	
  }
}

uint8_t nextByteToTransmit(uint8_t masterACKed)
{
//	static uint8_t temp = 0;

//	return temp++;

  if(usi_i2c_requestedAddress == USI_I2C_MYEDIDADDRESS)
  {
	uint8_t temp = edidArray[edidArrayIndex];

	ledsControlled = FALSE;
	if(masterACKed)
	{
		incrementEDIDIndex();
//		edidArrayIndex++;
	}

//	edidArrayIndex &= 0x07;

	return temp;
  }
  else if (usi_i2c_requestedAddress == USI_I2C_MYLEDADDRESS)
  {
	uint8_t temp = ledState[ledIndex];

	if(masterACKed)
	{
		incrementLEDIndex();
	}

	return temp;
  }

  //Should only get here if we're not address
  // in which case we shouldn't even get here.
  return 0xff;
}


void edid_checkSummer(void)
{
	uint8_t i;
	uint8_t sum = 0;

	// Don't include the garbage checksum value...
	// it will be overwritten here...
	for(i=0; i<0x7f; i++)
	{
		sum += edidArray[i];
	}

	//Load the calculated checksum
	edidArray[0x7f] = (uint8_t)((uint8_t)0 - (uint8_t)(sum));
}




#if FADER_ENABLED
//uint8_t ledsControlled = FALSE;

#define  LED_R PB4				// D4
#define  LED_G PB3				//	D5
#define	LED_B HEART_PINNUM   //	D2 after trace-change PB1 //NYI
#define LED_PORT PORTB


hfm_t ledHFM[3];


void updateLEDs(void);

void updateLEDFader(void)
{
	static uint8_t delay=0;
	static theta_t theta=2; //test = 2;

	delay++;
	if(delay == 0xff)
		theta++;

	if(theta == SINE_PI*3)
		theta = 0;



	if(theta < SINE_2PI)
		ledState[0] = \
			(uint8_t)((int16_t)sineRaw8(theta - SINE_PI_2) + (int16_t)127); 

	if((theta > SINE_PI) && (theta < SINE_PI*3))
	  	ledState[1] = \
		 	(uint8_t)((int16_t)sineRaw8(theta - SINE_PI_2 - SINE_PI) 
				+ (int16_t)127);	  

	if(theta > SINE_2PI)
		ledState[2] = \
			(uint8_t)((int16_t)sineRaw8(theta - SINE_PI_2 - SINE_2PI)
				+ (int16_t)127);
	else if(theta < SINE_PI)
		ledState[2] = \
			(uint8_t)((int16_t)sineRaw8(theta - SINE_PI_2 + SINE_PI) 
				+ (int16_t)127);
//	ledState[1] = 0x07;
	
//	ledState[2] = (uint8_t)((int16_t)sineRaw8(theta - SINE_PI_2) + (int16_t)127);

		//(uint32_t)(sineRaw(theta - SINE_PI_2))*127/SINE_MAX + 127;

	/*
	if(theta == 0xff)
	{
		theta=0;
		ledState[2]++; // = test; //2; //0x07; //theta;
	}
*/
//	ledState[0] = 0x07; //theta;

//	if(theta >= SINE_2PI) //*3)
//		theta -= SINE_2PI; //*3;

//	if(theta == SINE_2PI*8)
//	{
//		theta = 0;
//		togglebit(7, ledState[1]);
//	}

//	if(theta < SINE_2PI)
//		ledState[0] = (int32_t)theta*(int32_t)255/SINE_2PI;
//			(((int32_t)sineRaw(theta) * (int32_t)127) / 
//						 (int32_t)SINE_MAX) + 127;
		//((int8_t)(sineRaw(theta - SINE_PI_2)>>8) + (int8_t)127);

/*	if((theta > SINE_PI) && (theta < SINE_PI*3))
		ledState[1] = ((sineRaw(theta - SINE_PI_2 - SINE_PI)>>8) + 127);

	if(theta > SINE_2PI)
		ledState[2] = ((sineRaw(theta - SINE_PI_2 - SINE_2PI)>>8) + 127);
	else if(theta < SINE_PI)
		ledState[2] = ((sineRaw(theta - SINE_PI_2 + SINE_PI)>>8) + 127);
*/
	updateLEDs();
}

void updateLEDs(void)
{
	static uint8_t lastState[3] = {0,0,0};

	uint8_t i;
	for(i=0; i<3; i++)
	{
	//	if(ledState[i] != lastState[i])
		{
		//	ledsControlled = TRUE;
			lastState[i] = ledState[i];
			hfm_setPower(&(ledHFM[i]), ledState[i]);
		}
	}
	//hfm_setPower(&led_r, ledState[0]);
	//hfm_setPower(&led_g, ledState[1]);


	if(hfm_nextOutput(&(ledHFM[0])))
		clrpinPORT(LED_R, LED_PORT);
	else
		setpinPORT(LED_R, LED_PORT);

	if(hfm_nextOutput(&(ledHFM[1])))
		clrpinPORT(LED_G, LED_PORT);
	else
		setpinPORT(LED_G, LED_PORT);

	if(hfm_nextOutput(&(ledHFM[2])))
		clrpinPORT(LED_B, LED_PORT);
	else
		setpinPORT(LED_B, LED_PORT);

	//NYI:
	//if(ledState[2])

//	togglepinPORT(LED_R, LED_PORT);
//	togglepinPORT(LED_G, LED_PORT);
}
#endif

int main(void)
{
#if FADER_ENABLED
	uint8_t i;
	for(i=0; i<3; i++)
		hfm_setup(&(ledHFM[i]), 0, 255);
	//hfm_setup(&led_g, 0, 255);
#endif

//	uint16_t i;
//	for(i=0; i<256; i++)
//		edidArray[i] = i;

	//Found experimentally: assuming the free-running ADC is always 13
	// cycles per interrupt...
	// The default value was read to be 0x9f
	// This is of course device-specific
//	OSCCAL = 0x9a;

	//*** Initializations ***

	//!!! WDT could cause problems... this probably should be inited earlier and called everywhere...
	//INIT_HEARTBEAT(HEARTBEATPIN, HEARTBEAT, HEARTCONNECTION);


	init_heartBeat();

	setHeartRate(0);	

	//Blink until the EDID is read...
	//This is hokey...
	//set_heartBlink(1);

	edid_checkSummer();

	usi_i2c_slaveInit();

#if FADER_ENABLED
	setoutPORT(LED_R, LED_PORT);
	setoutPORT(LED_G, LED_PORT);
	setpinPORT(LED_R, LED_PORT);
	setpinPORT(LED_G, LED_PORT);
#endif

//	setoutPORT(LED_B, LED_PORT);

	//This was only necessary for debugging timer initialization bugs...
	// which have been resolved
//	set_heartBlink(retVal);

	while(1)
	{
#if FADER_ENABLED
		if(!ledsControlled)
		{
			extern uint8_t heartBlink; 

			if(heartBlink)
				heartUpdate();
			else
				updateLEDFader();
		}
		//This was not previously elsed... how did the heart work at all?!
		// at one point it didn't (and neither did updateLEDs()
		// but somehow it started up again
		// a/o v49: May have been a result of the hfm bug...
		// 			(how did it start up again? Something to do with an
		//           uninitialized value in HFM?)
		else
		{
			// The heartbeat may have been in input (off) mode when switched
			setoutPORT(LED_B, LED_PORT);

			updateLEDs();
		}
#else
		heartUpdate();
#endif
	}

}


