/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
///////////////////////////////////////////////////////////////////////////
//
// Desc: usarsim (simulator) Pyro driver for the Ringhorne project, 
//	will be connected to the fiducial interface
// Author: Erik Winter
// Date: 11 Mars 2005
//
// Modified: 
// 14 Mars 2005 	Erik Winter 	added RHPyro config 
// 14 Mars 2005    Erik Winter Started porting USARSim to Player1.6
// 15 Mars 2005    Erik Winter Continued porting, it compiles but gives segmentation faults 
// 18 Mars 2005    Erik Winter Changed the definitions of PutCommand and PutConfig for Player1.6 
///////////////////////////////////////////////////////////////////////////
#ifndef US_RH_PYRO_H
#define US_RH_PYRO_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>       // for atoi(3)
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#include "libplayercore/player.h"

#include "us_bot.h"

/// individual Ringhorne robot driver not updated yet
class RhPyro : public Driver
{ 
public:
  RhPyro(ConfigFile* cf, int section); // Constructor
  virtual ~RhPyro();  // Destructor
  virtual int Setup(); // Setup/shutdown routines.
  virtual int Shutdown();
  void Main();
  int ProcessMessage(QueuePointer& resp_queue, player_msghdr* hdr,void *	data); 	
private: 
   player_devaddr_t bot; // The bot that holds the device
  // The parent bot index
  int bot_index;
};
#endif //US_RH_PYRO

