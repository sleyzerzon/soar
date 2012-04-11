/** THIS IS AN AUTOMATICALLY GENERATED FILE.  DO NOT MODIFY
 * BY HAND!!
 *
 * Generated by lcm-gen
 **/

#include <string.h>
#include "sync_debug_t.h"

static int __sync_debug_t_hash_computed;
static int64_t __sync_debug_t_hash;
 
int64_t __sync_debug_t_hash_recursive(const __lcm_hash_ptr *p)
{
    const __lcm_hash_ptr *fp;
    for (fp = p; fp != NULL; fp = fp->parent)
        if (fp->v == __sync_debug_t_get_hash)
            return 0;
 
    const __lcm_hash_ptr cp = { p, (void*)__sync_debug_t_get_hash };
    (void) cp;
 
    int64_t hash = 0xc867cc54e3f80410LL
         + __int64_t_hash_recursive(&cp)
         + __int64_t_hash_recursive(&cp)
         + __int64_t_hash_recursive(&cp)
        ;
 
    return (hash<<1) + ((hash>>63)&1);
}
 
int64_t __sync_debug_t_get_hash(void)
{
    if (!__sync_debug_t_hash_computed) {
        __sync_debug_t_hash = __sync_debug_t_hash_recursive(NULL);
        __sync_debug_t_hash_computed = 1;
    }
 
    return __sync_debug_t_hash;
}
 
int __sync_debug_t_encode_array(void *buf, int offset, int maxlen, const sync_debug_t *p, int elements)
{
    int pos = 0, thislen, element;
 
    for (element = 0; element < elements; element++) {
 
        thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].rx_utime), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
        thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].sensor_time), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
        thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &(p[element].estimated_utime), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
    }
    return pos;
}
 
int sync_debug_t_encode(void *buf, int offset, int maxlen, const sync_debug_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __sync_debug_t_get_hash();
 
    thislen = __int64_t_encode_array(buf, offset + pos, maxlen - pos, &hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
 
    thislen = __sync_debug_t_encode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;
 
    return pos;
}
 
int __sync_debug_t_encoded_array_size(const sync_debug_t *p, int elements)
{
    int size = 0, element;
    for (element = 0; element < elements; element++) {
 
        size += __int64_t_encoded_array_size(&(p[element].rx_utime), 1);
 
        size += __int64_t_encoded_array_size(&(p[element].sensor_time), 1);
 
        size += __int64_t_encoded_array_size(&(p[element].estimated_utime), 1);
 
    }
    return size;
}
 
int sync_debug_t_encoded_size(const sync_debug_t *p)
{
    return 8 + __sync_debug_t_encoded_array_size(p, 1);
}
 
int __sync_debug_t_decode_array(const void *buf, int offset, int maxlen, sync_debug_t *p, int elements)
{
    int pos = 0, thislen, element;
 
    for (element = 0; element < elements; element++) {
 
        thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].rx_utime), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
        thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].sensor_time), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
        thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &(p[element].estimated_utime), 1);
        if (thislen < 0) return thislen; else pos += thislen;
 
    }
    return pos;
}
 
int __sync_debug_t_decode_array_cleanup(sync_debug_t *p, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {
 
        __int64_t_decode_array_cleanup(&(p[element].rx_utime), 1);
 
        __int64_t_decode_array_cleanup(&(p[element].sensor_time), 1);
 
        __int64_t_decode_array_cleanup(&(p[element].estimated_utime), 1);
 
    }
    return 0;
}
 
int sync_debug_t_decode(const void *buf, int offset, int maxlen, sync_debug_t *p)
{
    int pos = 0, thislen;
    int64_t hash = __sync_debug_t_get_hash();
 
    int64_t this_hash;
    thislen = __int64_t_decode_array(buf, offset + pos, maxlen - pos, &this_hash, 1);
    if (thislen < 0) return thislen; else pos += thislen;
    if (this_hash != hash) return -1;
 
    thislen = __sync_debug_t_decode_array(buf, offset + pos, maxlen - pos, p, 1);
    if (thislen < 0) return thislen; else pos += thislen;
 
    return pos;
}
 
int sync_debug_t_decode_cleanup(sync_debug_t *p)
{
    return __sync_debug_t_decode_array_cleanup(p, 1);
}
 
int __sync_debug_t_clone_array(const sync_debug_t *p, sync_debug_t *q, int elements)
{
    int element;
    for (element = 0; element < elements; element++) {
 
        __int64_t_clone_array(&(p[element].rx_utime), &(q[element].rx_utime), 1);
 
        __int64_t_clone_array(&(p[element].sensor_time), &(q[element].sensor_time), 1);
 
        __int64_t_clone_array(&(p[element].estimated_utime), &(q[element].estimated_utime), 1);
 
    }
    return 0;
}
 
sync_debug_t *sync_debug_t_copy(const sync_debug_t *p)
{
    sync_debug_t *q = (sync_debug_t*) malloc(sizeof(sync_debug_t));
    __sync_debug_t_clone_array(p, q, 1);
    return q;
}
 
void sync_debug_t_destroy(sync_debug_t *p)
{
    __sync_debug_t_decode_array_cleanup(p, 1);
    free(p);
}
 
int sync_debug_t_publish(lcm_t *lc, const char *channel, const sync_debug_t *p)
{
      int max_data_size = sync_debug_t_encoded_size (p);
      uint8_t *buf = (uint8_t*) malloc (max_data_size);
      if (!buf) return -1;
      int data_size = sync_debug_t_encode (buf, 0, max_data_size, p);
      if (data_size < 0) {
          free (buf);
          return data_size;
      }
      int status = lcm_publish (lc, channel, buf, data_size);
      free (buf);
      return status;
}

struct _sync_debug_t_subscription_t {
    sync_debug_t_handler_t user_handler;
    void *userdata;
    lcm_subscription_t *lc_h;
};
static
void sync_debug_t_handler_stub (const lcm_recv_buf_t *rbuf, 
                            const char *channel, void *userdata)
{
    int status;
    sync_debug_t p;
    memset(&p, 0, sizeof(sync_debug_t));
    status = sync_debug_t_decode (rbuf->data, 0, rbuf->data_size, &p);
    if (status < 0) {
        fprintf (stderr, "error %d decoding sync_debug_t!!!\n", status);
        return;
    }

    sync_debug_t_subscription_t *h = (sync_debug_t_subscription_t*) userdata;
    h->user_handler (rbuf, channel, &p, h->userdata);

    sync_debug_t_decode_cleanup (&p);
}

sync_debug_t_subscription_t* sync_debug_t_subscribe (lcm_t *lcm, 
                    const char *channel, 
                    sync_debug_t_handler_t f, void *userdata)
{
    sync_debug_t_subscription_t *n = (sync_debug_t_subscription_t*)
                       malloc(sizeof(sync_debug_t_subscription_t));
    n->user_handler = f;
    n->userdata = userdata;
    n->lc_h = lcm_subscribe (lcm, channel, 
                                 sync_debug_t_handler_stub, n);
    if (n->lc_h == NULL) {
        fprintf (stderr,"couldn't reg sync_debug_t LCM handler!\n");
        free (n);
        return NULL;
    }
    return n;
}

int sync_debug_t_subscription_set_queue_capacity (sync_debug_t_subscription_t* subs, 
                              int num_messages)
{
    return lcm_subscription_set_queue_capacity (subs->lc_h, num_messages);
}

int sync_debug_t_unsubscribe(lcm_t *lcm, sync_debug_t_subscription_t* hid)
{
    int status = lcm_unsubscribe (lcm, hid->lc_h);
    if (0 != status) {
        fprintf(stderr, 
           "couldn't unsubscribe sync_debug_t_handler %p!\n", hid);
        return -1;
    }
    free (hid);
    return 0;
}

