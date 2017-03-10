///////////////////////////////////////////////////////////////////
// DS1307.cpp
// 
///////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include <DS1307.h>
	
	const uint8_t daysInMonth [] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	static const char wday_name[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	
	static const char mon_name[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

///////////////////////////////////////////////////////////////////
//
// asctime(const struct tm *timeptr)
//
// 
//
///////////////////////////////////////////////////////////////////

char* asctime(const struct tm *timeptr)
{

	static char result[26];
	sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
		wday_name[timeptr->tm_wday],
		mon_name[timeptr->tm_mon],
		timeptr->tm_mday, timeptr->tm_hour,
		timeptr->tm_min, timeptr->tm_sec,
		1900 + timeptr->tm_year);
  return result;
}

unsigned int lulz(unsigned char const* nybbles, size_t length)
{
    unsigned int result(0);
    while (length--) {
        result = result * 100 + (*nybbles >> 4) * 10 + (*nybbles & 15);
        ++nybbles;
    }
    return result;
}

///////////////////////////////////////////////////////////////////
//
//	AscTime(void)
//
//	reads the time and returns formatted date and time.
//
//	Returns: pointer to char string containing formatted date and time
//
///////////////////////////////////////////////////////////////////

char* DS1307::AscTime(bool bRefresh){
	static char result[26];
	if( bRefresh) {
		Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);
	}
	sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d",
		wday_name[m_rtc_bcd[DS1307_REG_DOW] & DS1307_LO_DOW],
		mon_name[(10*((m_rtc_bcd[DS1307_REG_MONTH] & DS1307_HI_MTH)>>4))+(m_rtc_bcd[DS1307_REG_MONTH] & DS1307_LO_BCD)-1],
		(10*((m_rtc_bcd[DS1307_REG_DAY]  & DS1307_HI_DATE)>>4))+(m_rtc_bcd[DS1307_REG_DAY]  & DS1307_LO_BCD),
		(10*((m_rtc_bcd[DS1307_REG_HOUR] & DS1307_HI_HR)>>4))  +(m_rtc_bcd[DS1307_REG_HOUR] & DS1307_LO_BCD),
		(10*((m_rtc_bcd[DS1307_REG_MIN]  & DS1307_HI_MIN)>>4)) +(m_rtc_bcd[DS1307_REG_MIN]  & DS1307_LO_BCD),
		(10*((m_rtc_bcd[DS1307_REG_SEC]  & DS1307_HI_SEC)>>4)) +(m_rtc_bcd[DS1307_REG_SEC]  & DS1307_LO_BCD),
		(10*((m_rtc_bcd[DS1307_REG_YEAR] & DS1307_HI_YR)>>4))+(m_rtc_bcd[DS1307_REG_YEAR] & DS1307_LO_BCD)+DS1307_BASE_YR );
  return result;
}

///////////////////////////////////////////////////////////////////
//
//	Initialise(void)
//
//	checks to see that the RTC is both connected and Enabled. If 
//	the RTC is not enabled then enables RTC. finally reads the RTC
//	registers in to m_rtcbcd array.
//
//	Returns: True if connected, false otherwise
//
///////////////////////////////////////////////////////////////////

bool DS1307::Initialise(void){
	if(IsConnected()){
		if( !IsRunning()) {
			Start();
		}
		Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);
		return true;
	} else {
		return false;
	}
}
///////////////////////////////////////////////////////////////////
//
// IsConnected()
//
// NOTE: not compatible with error codes
// NOTE: the DS1307 chip does not have a who am I register so we 
// cannot use I2C::IsConnected() so we do a read at m_Device, 0, 
// seven bytes if successful then we are connected.
//
// returns: true if DS1307 is connected otherwise false if not
//
///////////////////////////////////////////////////////////////////

bool DS1307::IsConnected(void) {

	bool bResult = Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);

	return bResult;
}

///////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////

bool DS1307::IsLeapYear(int iYear)
{
return (iYear % 400 == 0) || ((iYear % 4 == 0) && (iYear % 100 != 0));
}

///////////////////////////////////////////////////////////////////
//
// PUBLIC FUNCTIONS
//
///////////////////////////////////////////////////////////////////
//
//	Get(int *rtc, bool refresh)
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Get(int *rtc, bool refresh)   // Aquire data from buffer and convert to int, refresh buffer if required
{
	if(refresh) {
		Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);
	}
	for(int i = 0; i<7; i++) {  // cycle through each component, create array of data
		rtc[i] = Get(i, false);
	}
}

///////////////////////////////////////////////////////////////////
//
//	Get(int c, bool refresh)
//
//	returns	int of reg value
//
///////////////////////////////////////////////////////////////////

int DS1307::Get(int c, bool refresh)  // aquire individual RTC item from buffer, return as int, refresh buffer if required
{
	if(refresh) {
		Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);
	}
	int v=-1;
	switch(c) {
		case DS1307_REG_SEC:
			v=(10*((m_rtc_bcd[DS1307_REG_SEC] & DS1307_HI_SEC)>>4))+(m_rtc_bcd[DS1307_REG_SEC] & DS1307_LO_BCD);
			break;
		case DS1307_REG_MIN:
			v=(10*((m_rtc_bcd[DS1307_REG_MIN] & DS1307_HI_MIN)>>4))+(m_rtc_bcd[DS1307_REG_MIN] & DS1307_LO_BCD);
			break;
		case DS1307_REG_HOUR:
			v=(10*((m_rtc_bcd[DS1307_REG_HOUR] & DS1307_HI_HR)>>4))+(m_rtc_bcd[DS1307_REG_HOUR] & DS1307_LO_BCD);
			break;
		case DS1307_REG_DOW:
			v=m_rtc_bcd[DS1307_REG_DOW] & DS1307_LO_DOW;
			break;
		case DS1307_REG_DAY:
			v=(10*((m_rtc_bcd[DS1307_REG_DAY] & DS1307_HI_DATE)>>4))+(m_rtc_bcd[DS1307_REG_DAY] & DS1307_LO_BCD);
			break;
		case DS1307_REG_MONTH:
			v=(10*((m_rtc_bcd[DS1307_REG_MONTH] & DS1307_HI_MTH)>>4))+(m_rtc_bcd[DS1307_REG_MONTH] & DS1307_LO_BCD);
			break;
		case DS1307_REG_YEAR:
			v=(10*((m_rtc_bcd[DS1307_REG_YEAR] & DS1307_HI_YR)>>4))+(m_rtc_bcd[DS1307_REG_YEAR] & DS1307_LO_BCD)+DS1307_BASE_YR;
			break;
	} // end switch
	return v;
}

///////////////////////////////////////////////////////////////////
//
//	Get(tm &dt)
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Get(tm &dt)
{
	Read( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0]);

	dt.tm_sec  = (10*((m_rtc_bcd[DS1307_REG_SEC]   & DS1307_HI_SEC)>>4))+(m_rtc_bcd[DS1307_REG_SEC]    & DS1307_LO_BCD);
	dt.tm_min  = (10*((m_rtc_bcd[DS1307_REG_MIN]   & DS1307_HI_MIN)>>4))+(m_rtc_bcd[DS1307_REG_MIN]    & DS1307_LO_BCD);
	dt.tm_hour = (10*((m_rtc_bcd[DS1307_REG_HOUR]  & DS1307_HI_HR) >>4))+(m_rtc_bcd[DS1307_REG_HOUR]   & DS1307_LO_BCD);
	dt.tm_wday = m_rtc_bcd[DS1307_REG_DOW] & DS1307_LO_DOW;
	dt.tm_mday = (10*((m_rtc_bcd[DS1307_REG_DAY]   & DS1307_HI_DATE)>>4))+(m_rtc_bcd[DS1307_REG_DAY]   & DS1307_LO_BCD);
	dt.tm_mon  = (10*((m_rtc_bcd[DS1307_REG_MONTH] & DS1307_HI_MTH) >>4))+(m_rtc_bcd[DS1307_REG_MONTH] & DS1307_LO_BCD) - 1 ;
	dt.tm_year = (10*((m_rtc_bcd[DS1307_REG_YEAR]  & DS1307_HI_YR)  >>4))+(m_rtc_bcd[DS1307_REG_YEAR]  & DS1307_LO_BCD)+DS1307_BASE_YR;
}

///////////////////////////////////////////////////////////////////
//
//	Set(const tm &dt)
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Set(const tm &dt)
{
	// Update buffer, then update the chip
	m_rtc_bcd[DS1307_REG_SEC]   = ((dt.tm_sec  / 10)<<4) + (dt.tm_sec  % 10);
	m_rtc_bcd[DS1307_REG_MIN]   = ((dt.tm_min  / 10)<<4) + (dt.tm_min  % 10);
	m_rtc_bcd[DS1307_REG_HOUR]  = ((dt.tm_hour / 10)<<4) + (dt.tm_hour % 10);
	m_rtc_bcd[DS1307_REG_DOW]   = dt.tm_wday;
	m_rtc_bcd[DS1307_REG_DAY]   = ((dt.tm_mday / 10)<<4) + (dt.tm_mday % 10);
	m_rtc_bcd[DS1307_REG_MONTH] = ((dt.tm_mon  / 10)<<4) + (dt.tm_mon  % 10)+1;
	m_rtc_bcd[DS1307_REG_YEAR]  = (((dt.tm_year-DS1307_BASE_YR) / 10)<<4) + ((dt.tm_year-DS1307_BASE_YR) % 10);

	Write( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0] );
}

///////////////////////////////////////////////////////////////////
//
//	Set(int c, int v)
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Set(int c, int v)  // Update buffer, then update the chip
{
	switch(c) {
		case DS1307_REG_SEC:
			if(v<60 && v>-1) {
				//preserve existing clock state (running/stopped)
				int state=m_rtc_bcd[DS1307_REG_SEC] & DS1307_CLOCKHALT;
				m_rtc_bcd[DS1307_REG_SEC]=state | ((v / 10)<<4) + (v % 10);
			}
			break;
		case DS1307_REG_MIN:
			if(v<60 && v>-1) {
				m_rtc_bcd[DS1307_REG_MIN]=((v / 10)<<4) + (v % 10);
				Serial.print( "Setting Minuite to: " ) ; Serial.println( v );
			}
			break;
		case DS1307_REG_HOUR:
			// TODO : AM/PM  12HR/24HR
			if(v<24 && v>-1) {
				m_rtc_bcd[DS1307_REG_HOUR]=((v / 10)<<4) + (v % 10);
				Serial.print( "Setting Hour to: " ) ; Serial.println( v );
			}
			break;
		case DS1307_REG_DOW:
			if(v<8 && v>-1) {
				m_rtc_bcd[DS1307_REG_DOW]=v;
			}
			break;
		case DS1307_REG_DAY:
			if(v<31 && v>-1) {
				m_rtc_bcd[DS1307_REG_DAY]=((v / 10)<<4) + (v % 10);
				Serial.print( "Setting Day to: " ) ; Serial.println( v );
			}
			break;
		case DS1307_REG_MONTH:
			if(v<13 && v>-1) {
				m_rtc_bcd[DS1307_REG_MONTH]=((v / 10)<<4) + (v % 10);
				Serial.print( "Setting Month to: " ) ; Serial.println( v );
			}
			break;
		case DS1307_REG_YEAR:
			if( v >= DS1307_BASE_YR ) {
				v -= DS1307_BASE_YR;
			}
			if( v<99 && v>-1 ) {
				m_rtc_bcd[DS1307_REG_YEAR]=((v / 10)<<4) + (v % 10);
				Serial.print( "Setting Year to: " ) ; Serial.println( v );
			}
			break;
	} // end switch
	Write( DS1307_REG_SEC, 7, (uint8_t*)&m_rtc_bcd[0] );
}

///////////////////////////////////////////////////////////////////
//
//	IsRunning(void)
//
//	Gets the state of the enable bit of the DS1307
//
//	Returns:	true if RTC is running, false if not
//
///////////////////////////////////////////////////////////////////

bool DS1307::IsRunning(void) {
	uint8_t ucData;
	Read(DS1307_REG_SEC, ucData) ;
	return !(ucData>>7);
}


///////////////////////////////////////////////////////////////////
//
//	Stop(void)
//
//	set the ClockHalt bit high to stop the rtc
//	this is bit 7 of the seconds byte
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Stop(void) {
	Read( DS1307_REG_SEC, m_rtc_bcd[DS1307_REG_SEC]);
	m_rtc_bcd[DS1307_REG_SEC] |= DS1307_CLOCKHALT;
	Write( DS1307_REG_SEC, m_rtc_bcd[DS1307_REG_SEC]);

}

///////////////////////////////////////////////////////////////////
//
//	Start(void)
//
//	clear the ClockHalt bit to start the rtc
//	this is bit 7 of the seconds byte
//
//	returns	nothing
//
///////////////////////////////////////////////////////////////////

void DS1307::Start(void) {
	Read(DS1307_REG_SEC, m_rtc_bcd[DS1307_REG_SEC]);
	m_rtc_bcd[DS1307_REG_SEC] &= DS1307_CLOCKSTART ;
	Write(DS1307_REG_SEC, m_rtc_bcd[DS1307_REG_SEC]);

}

////////////////////////// end of DS1307.cpp //////////////////////