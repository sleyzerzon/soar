/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.33
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

#ifndef SWIG_playercore_java_WRAP_H_
#define SWIG_playercore_java_WRAP_H_

class SwigDirector_Driver : public Driver, public Swig::Director {

public:
    void swig_connect_director(JNIEnv *jenv, jobject jself, jclass jcls, bool swig_mem_own, bool weak_global);
    SwigDirector_Driver(JNIEnv *jenv, ConfigFile *cf, int section, bool overwrite_cmds, size_t queue_maxlen, int interf);
    SwigDirector_Driver(JNIEnv *jenv, ConfigFile *cf, int section, bool overwrite_cmds = true, size_t queue_maxlen = PLAYER_MSGQUEUE_DEFAULT_MAXLEN);
    virtual void StartThread();
    virtual void StartThreadSwigPublic() {
      Driver::StartThread();
    }
    virtual void StopThread();
    virtual void StopThreadSwigPublic() {
      Driver::StopThread();
    }
    virtual void Publish(player_devaddr_t addr, QueuePointer &queue, uint8_t type, uint8_t subtype, void *src = NULL, size_t deprecated = 0, double *timestamp = NULL, bool copy = true);
    virtual void Publish(player_devaddr_t addr, uint8_t type, uint8_t subtype, void *src = NULL, size_t deprecated = 0, double *timestamp = NULL, bool copy = true);
    virtual void Publish(QueuePointer &queue, player_msghdr_t *hdr, void *src, bool copy = true);
    virtual void Publish(player_msghdr_t *hdr, void *src, bool copy = true);
    virtual ~SwigDirector_Driver();
    virtual int Setup();
    virtual int Shutdown();
    virtual void Main();
    virtual void MainQuit();
    virtual void Update();
    virtual int ProcessInternalMessages(QueuePointer &resp_queue, player_msghdr *hdr, void *data);
    virtual bool RegisterProperty(char const *key, Property *prop, ConfigFile *cf, int section);
    virtual bool RegisterProperty(Property *prop, ConfigFile *cf, int section);
public:
    bool swig_overrides(int n) {
      return (n < 24 ? swig_override[n] : false);
    }
protected:
    bool swig_override[24];
};


#endif
