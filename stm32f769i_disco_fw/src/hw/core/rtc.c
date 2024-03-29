/*
 * rtc.c
 *
 *  Created on: 2022. 1. 11.
 *      Author: hs
 */




#include "rtc.h"



#ifdef _USE_HW_RTC

#define RTC_CLOCK_SOURCE_LSE
//#define RTC_CLOCK_SOURCE_LSI

RTC_HandleTypeDef RtcHandle;

#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV  0x7F    /* LSI as RTC clock */
#define RTC_SYNCH_PREDIV   0x00F9  /* LSI as RTC clock */
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
#define RTC_ASYNCH_PREDIV  0x7F    /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF  /* LSE as RTC clock */
#endif

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k


//#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
#define LEAP_YEAR(Y)     ( ((2000+Y)>0) && !((2000+Y)%4) && ( ((2000+Y)%100) || !((2000+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
static uint32_t reset_time = 0;
static bool is_init = false;


//static void RTC_CalendarConfig(void);
static void rtcBreakTime(time_t timeInput, RTC_DateTypeDef *p_date, RTC_TimeTypeDef *p_time);
static time_t rtcMakeTime(RTC_DateTypeDef *p_date, RTC_TimeTypeDef *p_time);



bool rtcInit(void)
{
  bool ret = true;

  //__HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follows:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */
  RtcHandle.Instance = RTC;
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    logPrintf("RTC     \t\t: Fail\r\n");
    return false;
  }
  logPrintf("RTC     \t\t: OK\r\n");
  return ret;
}

bool rtcIsInit(void)
{
  return is_init;
}

time_t rtcGetTime()
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;
  time_t ret = 0;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);


  ret = rtcMakeTime(&sdatestructureget, &stimestructureget);

  return ret;
}

uint32_t rtcGetSecondsFromPower(void)
{
  uint32_t pre_time;
  uint32_t cur_time;

  pre_time = rtcReadBackupData(RTC_CFG_DATA_2);
  cur_time = (uint32_t)rtcGetTime();

  return cur_time - pre_time;
}

uint32_t rtcGetSecondsFromReset(void)
{
  uint32_t pre_time;
  uint32_t cur_time;

  pre_time = reset_time;
  cur_time = (uint32_t)rtcGetTime();

  return cur_time - pre_time;
}

void rtcSetTime(time_t time_data)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;


  HAL_RTC_GetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BIN);


  rtcBreakTime(time_data, &sdatestructure, &stimestructure);

  //HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD);
  //HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD);
  HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BIN);
  HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BIN);
}

time_t rtcMakeTime(RTC_DateTypeDef *p_date, RTC_TimeTypeDef *p_time)
{
// assemble time elements into time_t
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= p_date->Year*(SECS_PER_DAY * 365);
  for (i = 0; i < p_date->Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < p_date->Month; i++) {
    if ( (i == 2) && LEAP_YEAR(p_date->Year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (p_date->Date-1) * SECS_PER_DAY;
  seconds+= p_time->Hours * SECS_PER_HOUR;
  seconds+= p_time->Minutes * SECS_PER_MIN;
  seconds+= p_time->Seconds;
  return (time_t)seconds;
}

void rtcBreakTime(time_t timeInput, RTC_DateTypeDef *p_date, RTC_TimeTypeDef *p_time)
{
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  p_time->Seconds = time % 60;
  time /= 60; // now it is minutes
  p_time->Minutes = time % 60;
  time /= 60; // now it is hours
  p_time->Hours = time % 24;
  time /= 24; // now it is days


  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  p_date->Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  p_date->Month = month + 1;  // jan is month 1
  p_date->Date = time + 1;     // day of month
}


void rtcWriteBackupData(uint32_t index, uint32_t data)
{
  HAL_RTCEx_BKUPWrite(&RtcHandle, index, data);
}


uint32_t rtcReadBackupData(uint32_t index)
{
  return HAL_RTCEx_BKUPRead(&RtcHandle, index);
}

uint32_t rtcBackupRegRead(uint32_t index)
{
  return HAL_RTCEx_BKUPRead(&RtcHandle, index);
}

void rtcBackupRegWrite(uint32_t index, uint32_t data)
{
  HAL_RTCEx_BKUPWrite(&RtcHandle, index, data);
}




void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
  /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();


  /*##-2- Configure LSE as RTC clock source ###################################*/
#ifdef RTC_CLOCK_SOURCE_LSE
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* Configures the External Low Speed oscillator (LSE) drive capability */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
#elif defined (RTC_CLOCK_SOURCE_LSI)
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
#else
#error Please select the RTC Clock source inside the main.h file
#endif /*RTC_CLOCK_SOURCE_LSE*/

  /*##-3- Enable RTC peripheral Clocks #######################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();

  /*##-4- Configure the NVIC for RTC Alarm ###################################*/
  //HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x0F, 0);
  //HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_RTC_DISABLE();

  /*##-2- Disables the PWR Clock and Disables access to the backup domain ###################################*/
  HAL_PWR_DisableBkUpAccess();
  __HAL_RCC_PWR_CLK_DISABLE();
}


#endif
