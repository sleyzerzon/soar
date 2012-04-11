/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <stdint.h>
#include <stdlib.h>
#include <lcm/lcm_coretypes.h>
#include <lcm/lcm.h>

#ifndef _nmea_t_h
#define _nmea_t_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _nmea_t nmea_t;
struct _nmea_t
{
    int64_t    utime;
    char*      nmea;
};
 
nmea_t   *nmea_t_copy(const nmea_t *p);
void nmea_t_destroy(nmea_t *p);

typedef struct _nmea_t_subscription_t nmea_t_subscription_t;
typedef void(*nmea_t_handler_t)(const lcm_recv_buf_t *rbuf, 
             const char *channel, const nmea_t *msg, void *user);

int nmea_t_publish(lcm_t *lcm, const char *channel, const nmea_t *p);
nmea_t_subscription_t* nmea_t_subscribe(lcm_t *lcm, const char *channel, nmea_t_handler_t f, void *userdata);
int nmea_t_unsubscribe(lcm_t *lcm, nmea_t_subscription_t* hid);
int nmea_t_subscription_set_queue_capacity(nmea_t_subscription_t* subs, 
                              int num_messages);


int  nmea_t_encode(void *buf, int offset, int maxlen, const nmea_t *p);
int  nmea_t_decode(const void *buf, int offset, int maxlen, nmea_t *p);
int  nmea_t_decode_cleanup(nmea_t *p);
int  nmea_t_encoded_size(const nmea_t *p);

// LCM support functions. Users should not call these
int64_t __nmea_t_get_hash(void);
int64_t __nmea_t_hash_recursive(const __lcm_hash_ptr *p);
int     __nmea_t_encode_array(void *buf, int offset, int maxlen, const nmea_t *p, int elements);
int     __nmea_t_decode_array(const void *buf, int offset, int maxlen, nmea_t *p, int elements);
int     __nmea_t_decode_array_cleanup(nmea_t *p, int elements);
int     __nmea_t_encoded_array_size(const nmea_t *p, int elements);
int     __nmea_t_clone_array(const nmea_t *p, nmea_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif
