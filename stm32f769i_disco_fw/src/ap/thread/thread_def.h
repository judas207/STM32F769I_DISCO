/*
 * thread_def.h
 *
 *  Created on: 2022. 3. 3.
 *      Author: hs
 */

#ifndef SRC_AP_THREAD_THREAD_DEF_H_
#define SRC_AP_THREAD_THREAD_DEF_H_

#ifdef __cplusplus
 extern "C" {
#endif




typedef enum
{
  EVENT_SDCARD_CONNECTED,
  EVENT_SDCARD_DISCONNECTED,
  EVENT_TOUCH_PRESSED,
  EVENT_TOUCH_RELEASED,
  //EVENT_BT_OPEN,
  //EVENT_BT_CLOSE
} Event_t;

typedef enum
{
  THREAD_ID_INFO,
  THREAD_ID_CLI,
  THREAD_ID_LCD,
  //THREAD_ID_BTSTACK,
  THREAD_ID_MAX
} ThreadId_t;


typedef struct thread_t_ thread_t;

typedef struct thread_t_
{
  bool is_init;
  bool is_start;

  const char *name;
  uint32_t freq;
  uint32_t hearbeat;

  thread_t *list;

  //bool (*init)(thread_t *p_thread);
  bool (*notify)(Event_t event);
  bool (*onEvent)(Event_t event);

} thread_t;



#ifdef __cplusplus
}
#endif



#endif /* SRC_AP_THREAD_THREAD_DEF_H_ */
