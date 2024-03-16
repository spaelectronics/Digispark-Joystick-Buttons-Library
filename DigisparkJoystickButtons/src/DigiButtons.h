/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 * Modified for Digispark by Digistump
 * And now modified by Sean Murphy (duckythescientist) from a keyboard device to a joystick device
 * And now modified by Bluebie to have better code style, not ruin system timers, and have delay() function
 * And now modified by drbogger from joystick to a joystick for buttons only
 * Most of the credit for the joystick code should go to Raphaël Assénat
 */
#ifndef __DigiButtons_h__
#define __DigiButtons_h__
 
#define GCN64_REPORT_SIZE 1

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "usbdrv.h"
//#include "devdesc.h"
#include "oddebug.h"
#include "usbconfig.h"
 
const static uchar *rt_usbHidReportDescriptor=NULL;
static uchar rt_usbHidReportDescriptorSize=0;
const static uchar *rt_usbDeviceDescriptor=NULL;
static uchar rt_usbDeviceDescriptorSize=0;

// TODO: Work around Arduino 12 issues better.
//#include <WConstants.h>
//#undef int()

//typedef uint8_t byte;

/* What was most recently read from the controller */
unsigned char last_built_report[GCN64_REPORT_SIZE];

/* What was most recently sent to the host */
unsigned char last_sent_report[GCN64_REPORT_SIZE];

uchar		 reportBuffer[8];

// report frequency set to default of 50hz
#define DIGIBUTTONS_DEFAULT_REPORT_INTERVAL 20
static unsigned char must_report = 0;
static unsigned char idle_rate = DIGIBUTTONS_DEFAULT_REPORT_INTERVAL / 4; // in units of 4ms
// new minimum report frequency system:
static unsigned long last_report_time = 0;


// You can verify your HID Descriptor at: https://eleccelerator.com/usbdescreqparser/
// Make sure to change #define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH in usbconfig.h to match.

const unsigned char gcn64_usbHidReportDescriptor[] PROGMEM = {
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x05,        // Usage (Game Pad)
	0xA1, 0x01,        // Collection (Application)
	0x05, 0x09,        //   Usage Page (Button)
	0x19, 0x01,        //   Usage Minimum (0x01)
	
	#if ButtonQty == 1
		0x29, 0x01,        //   Usage Maximum (0x01)
	#elif ButtonQty == 2
		0x29, 0x02,        //   Usage Maximum (0x02)
	#elif ButtonQty == 3
		0x29, 0x03,        //   Usage Maximum (0x03)
	#elif ButtonQty == 4
		0x29, 0x04,        //   Usage Maximum (0x04)
	#elif ButtonQty == 5
		0x29, 0x05,        //   Usage Maximum (0x05)
	#elif ButtonQty == 6
		0x29, 0x06,        //   Usage Maximum (0x06)
	#elif ButtonQty == 7
		0x29, 0x07,        //   Usage Maximum (0x07)
	#else ButtonQty == 8
		0x29, 0x08,        //   Usage Maximum (0x08)
	#endif

	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x01,        //   Logical Maximum (1)
	0x75, 0x01,        //   Report Size (1)
	0x95, 0x08,        //   Report Count (8)
	0x81, 0x22,        //   Input (Data,Var,Abs,No Wrap,Linear,No Preferred State,No Null Position)
	0xC0,              // End Collection
}; // 23 bytes


#define USBDESCR_DEVICE					1

const unsigned char usbDescrDevice[] PROGMEM = {		/* USB device descriptor */
		18,					/* sizeof(usbDescrDevice): length of descriptor in bytes */
		USBDESCR_DEVICE,		/* descriptor type */
		0x01, 0x01, /* USB version supported */
		USB_CFG_DEVICE_CLASS,
		USB_CFG_DEVICE_SUBCLASS,
		0,					/* protocol */
		8,					/* max packet size */
		USB_CFG_VENDOR_ID,	/* 2 bytes */
		USB_CFG_DEVICE_ID,	/* 2 bytes */
		USB_CFG_DEVICE_VERSION, /* 2 bytes */
#if USB_CFG_VENDOR_NAME_LEN
		1,					/* manufacturer string index */
#else
		0,					/* manufacturer string index */
#endif
#if USB_CFG_DEVICE_NAME_LEN
		2,					/* product string index */
#else
		0,					/* product string index */
#endif
#if USB_CFG_SERIAL_NUMBER_LENGTH
		3,					/* serial number string index */
#else
		0,					/* serial number string index */
#endif
		1,					/* number of configurations */
};



void buttonsBuildReport(unsigned char *reportBuf) {
	if (reportBuf != NULL) {
		memcpy(reportBuf, last_built_report, GCN64_REPORT_SIZE);
	}
	
	memcpy(last_sent_report, last_built_report, GCN64_REPORT_SIZE); 
}

 
class DigiButtonsDevice {
 public:
	DigiButtonsDevice () {
		wdt_enable(WDTO_2S);  // Enable Watchdog Timer
		wdt_reset();
		cli();
		usbDeviceDisconnect();
		_delay_ms(250);
		wdt_reset();
		usbDeviceConnect();
	
		rt_usbHidReportDescriptor = gcn64_usbHidReportDescriptor;
		rt_usbHidReportDescriptorSize = sizeof(gcn64_usbHidReportDescriptor);
		rt_usbDeviceDescriptor = usbDescrDevice;
		rt_usbDeviceDescriptorSize = sizeof(usbDescrDevice);
		
		usbInit();
		
		sei();
		
		last_report_time = millis();
		
		
	}
	
	void update() {
		wdt_reset();
		usbPoll();
		
		// instead of above code, use millis arduino system to enforce minimum reporting frequency
		unsigned long time_since_last_report = millis() - last_report_time;
		if (time_since_last_report >= (idle_rate * 4 /* in units of 4ms - usb spec stuff */)) {
			last_report_time += idle_rate * 4;
			must_report = 1;
		}
		
		// if the report has changed, try force an update anyway
		if (memcmp(last_built_report, last_sent_report, GCN64_REPORT_SIZE)) {
			must_report = 1;
		}
	
		// if we want to send a report, signal the host computer to ask us for it with a usb 'interrupt'
		if (must_report) {
			if (usbInterruptIsReady()) {
				must_report = 0;
				
				buttonsBuildReport(reportBuffer);
				usbSetInterrupt(reportBuffer, GCN64_REPORT_SIZE);
			}
		}
	}
	
	// delay while updating until we are finished delaying
	void delay(long milli) {
		unsigned long last = millis();
	  while (milli > 0) {
	    unsigned long now = millis();
	    milli -= now - last;
	    last = now;
	    update();
	  }
	}
	
	void setButtons(unsigned char low) {
		last_built_report[0] = low;
	}
	
	void setButtons(char low,char high) {
		setButtons(*reinterpret_cast<unsigned char *>(&low));
	}
	
};

// Create global singleton object for users to make use of
DigiButtonsDevice DigiButtons = DigiButtonsDevice();





#ifdef __cplusplus
extern "C"{
#endif 
	// USB_PUBLIC uchar usbFunctionSetup
	
	uchar usbFunctionSetup(uchar data[8]) {
		usbRequest_t *rq = (usbRequest_t *)data;

		usbMsgPtr = reportBuffer;
		if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) { // class request type
			if (rq->bRequest == USBRQ_HID_GET_REPORT){ // wValue: ReportType (highbyte), ReportID (lowbyte)
				// we only have one report type, so don't look at wValue
				//curGamepad->buildReport(reportBuffer);
				//return curGamepad->report_size;
				return GCN64_REPORT_SIZE;
			} else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
				usbMsgPtr = &idle_rate;
				return 1;
			} else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
				idle_rate = rq->wValue.bytes[1];
			}
		} else {
			/* no vendor specific requests implemented */
		}
		return 0;
	}

	uchar usbFunctionDescriptor(struct usbRequest *rq) {
		if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_STANDARD) {
			return 0;
		}

		if (rq->bRequest == USBRQ_GET_DESCRIPTOR) {
			// USB spec 9.4.3, high byte is descriptor type
			switch (rq->wValue.bytes[1]) {
				case USBDESCR_DEVICE:
					usbMsgPtr = rt_usbDeviceDescriptor;
					return rt_usbDeviceDescriptorSize;
					break;
					
				case USBDESCR_HID_REPORT:
					usbMsgPtr = rt_usbHidReportDescriptor;
					return rt_usbHidReportDescriptorSize;
					break;
					
			}
		}
		
		return 0;
	}
	
#ifdef __cplusplus
} // extern "C"
#endif


#endif // __DigiKeyboard_h__
