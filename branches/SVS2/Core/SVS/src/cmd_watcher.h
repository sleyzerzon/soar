#ifndef CMD_WATCHER_H
#define CMD_WATCHER_H

#include "soar_interface.h"

class svs_state;

class cmd_watcher {
public:
	virtual bool update_result() = 0;
	virtual bool early() = 0;
	virtual ~cmd_watcher() {};
};

cmd_watcher* make_cmd_watcher(svs_state *state, wme_hnd w);

#endif
