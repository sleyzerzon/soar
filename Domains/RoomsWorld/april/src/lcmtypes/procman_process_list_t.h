/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <stdint.h>
#include <stdlib.h>
#include <lcm/lcm_coretypes.h>
#include <lcm/lcm.h>

#ifndef _procman_process_list_t_h
#define _procman_process_list_t_h

#ifdef __cplusplus
extern "C" {
#endif

#include "procman_process_t.h"
typedef struct _procman_process_list_t procman_process_list_t;
struct _procman_process_list_t
{
    int64_t    utime;
    int64_t    init_utime;
    int8_t     exit;
    int32_t    nprocs;
    procman_process_t *processes;
};
 
procman_process_list_t   *procman_process_list_t_copy(const procman_process_list_t *p);
void procman_process_list_t_destroy(procman_process_list_t *p);

typedef struct _procman_process_list_t_subscription_t procman_process_list_t_subscription_t;
typedef void(*procman_process_list_t_handler_t)(const lcm_recv_buf_t *rbuf, 
             const char *channel, const procman_process_list_t *msg, void *user);

int procman_process_list_t_publish(lcm_t *lcm, const char *channel, const procman_process_list_t *p);
procman_process_list_t_subscription_t* procman_process_list_t_subscribe(lcm_t *lcm, const char *channel, procman_process_list_t_handler_t f, void *userdata);
int procman_process_list_t_unsubscribe(lcm_t *lcm, procman_process_list_t_subscription_t* hid);
int procman_process_list_t_subscription_set_queue_capacity(procman_process_list_t_subscription_t* subs, 
                              int num_messages);


int  procman_process_list_t_encode(void *buf, int offset, int maxlen, const procman_process_list_t *p);
int  procman_process_list_t_decode(const void *buf, int offset, int maxlen, procman_process_list_t *p);
int  procman_process_list_t_decode_cleanup(procman_process_list_t *p);
int  procman_process_list_t_encoded_size(const procman_process_list_t *p);

// LCM support functions. Users should not call these
int64_t __procman_process_list_t_get_hash(void);
int64_t __procman_process_list_t_hash_recursive(const __lcm_hash_ptr *p);
int     __procman_process_list_t_encode_array(void *buf, int offset, int maxlen, const procman_process_list_t *p, int elements);
int     __procman_process_list_t_decode_array(const void *buf, int offset, int maxlen, procman_process_list_t *p, int elements);
int     __procman_process_list_t_decode_array_cleanup(procman_process_list_t *p, int elements);
int     __procman_process_list_t_encoded_array_size(const procman_process_list_t *p, int elements);
int     __procman_process_list_t_clone_array(const procman_process_list_t *p, procman_process_list_t *q, int elements);

#ifdef __cplusplus
}
#endif

#endif
