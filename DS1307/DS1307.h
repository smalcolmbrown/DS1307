///////////////////////////////////////////////////////////////////
// DS1307.h - library for DS1307 real Time Clock RTC
//
//
///////////////////////////////////////////////////////////////////


// ensure this library description is only included once
#ifndef DS1307_h
#define DS1307_h

// include types & constants of Wire ic2 lib
#include <inttypes.h>
#include <Wire.h>
#include <I2C.h>


enum DS1307_REG_t { 
	DS1307_REG_SEC = 0,
	DS1307_REG_MIN,
	DS1307_REG_HOUR,
	DS1307_REG_DOW,
	DS1307_REG_DAY,
	DS1307_REG_MONTH,
	DS1307_REG_YEAR 
};


#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define SECONDS_FROM_1970_TO_2000 946684800

#define DS1307_BASE_YR 2000

//#define DS1307_ADDRESS B1101000			//DS1307  (0xD1>>1)
#define DS1307_ADDRESS (0xD1>>1)		//DS1307  (0xD1>>1)

 // Define register bit masks
#define DS1307_CLOCKHALT	0x80		// B10000000
#define DS1307_CLOCKSTART	0x7F		// b1111111

#define DS1307_LO_BCD  B00001111
#define DS1307_HI_BCD  B11110000

#define DS1307_HI_SEC  B01110000
#define DS1307_HI_MIN  B01110000
#define DS1307_HI_HR   B00110000
#define DS1307_LO_DOW  B00000111
#define DS1307_HI_DATE B00110000
#define DS1307_HI_MTH  B00110000
#define DS1307_HI_YR   B11110000


struct tm {
		int tm_sec;				// seconds after the minute		0-60
		int tm_min;				// minutes after the hour		0-59
		int tm_hour;			// hours since midnight			0-23
		int tm_mday;			// day of the month				1-31
		int tm_mon;				// months since January			0-11
		int tm_year;			// years since 1900				
		int tm_wday;			// days since Sunday			0-6
		int tm_yday;			// days since January 1			0-365
		int tm_isdst;			// Daylight Saving Time flag	
};




// library interface description
class DS1307 : public I2C {
	public:
					DS1307() : I2C(DS1307_ADDRESS, 0, 0) {}
		bool		Initialise(void);
		bool		IsConnected(void);
		bool		IsLeapYear(int iYear);
		void		Get(int *, bool);
		int			Get(int, bool);
		void		Get(tm &dt);
		char*		AscTime(bool bRefresh=true);
		void		Set(int, int);
		void		Set(const tm &dt);
		bool		IsRunning();
		void		Start(void);
		void		Stop(void);

  // library-accessible "private" interface
  private:
		uint8_t		m_rtc_bcd[7]; // used prior to read/set ds1307 registers;
};

char* asctime(const struct tm *timeptr);

#endif