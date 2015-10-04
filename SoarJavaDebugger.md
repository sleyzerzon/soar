# Old Wish List #

  * If I try to connect to an agent at a bad IP address, the delay until the failure message comes up can be somewhat long, and the debugger appears to hang in the meantime.
    * DJP: There's not much we can do about the delay.  Winsock doesn't let you configure the timeout on a connect() call. Probably the best option is to have a progress dialog up in another thread while the connection is happening, so it's clear that the debugger is alive and trying to connect.  I'll move this to the wish list.

  * Shortcut keys.  Allow user configuration of a shortcut to go to each window.

  * Ability to register a string to be output when a given event occurs.  Like Soar's old "monitor" cmd. --Karen 15:29, 9 May 2006 (EDT)

  * Matches command output

# Bob Wray's Old Debugger Wish List #

Features from TSI that I find useful:

  * fine-grained control of stepping (in actual implementation, TSI didnt do this so well, but make it easy to step at different grain sizes).
Should also make it possible to set step decisions from Input to Input or from slot decision to slot decision)

  * WM view is often useful as well.  Some kind of integration of WM view with datamap would be v useful (esp if you could highlight things in memory not expected from datamap or type mismatches).

Features Id like to see in a debugger:

  * ability to add/delete WMEs directly to memory along with appropriate prefs (so I can use add-wme to set up a test situation but not crash when something comes along to remove the WME).

  * ability to monitor/find expensive conditions/productions (ie, ones with largest # of tokens).  I know how to do this, but it's tedious. Most ST Soar programmers dont know how to do this and it seems like a debugger should make this trivial.  Im not sure the best way to do this, but just generating a list of the 10 most expensive productions during a run would be a good 1st step.  More generally, a self profiling f(x) would be beenficial.

  * ability to set up monitors in a couple of ways: 1) independent of the elaboration cycle (so I get feedback on a change to pref memory in the elab phase, not the WM phase), 2) independent of the display (eg, so I can see program output in one window and debug info in another).

  * stop/interrupt processing on some specific condition (in the Soar sense).  So I could set a breakpoint when something like this evaluated to true: (state < s > <sup>problem-space debug </sup>my-debug-situation t).  This was done in the sdb, but it used prods and so didnt result in immediate changes.

  * save state suficient to regenerate a problem context (this is likely out of scope for the debugger, but I continually find myself trying to 'regenerate' specific state situations for testing)

  * integration with SoarDocs, so that clicking one a production/operator/etc could bring up the doc on the element.  At the moment, I have to move back and forth b/w SoarDocs and the TSI.  Ive always mentioned that one feature I want in SoarSoc is user defined production types (eg, an elaborate\*my-object type) and then I might want to load/excise productions of that type as well as the standard ones.

  * a more condition oriented interface, so that I can, eg, click on a condition and see all the ways in which that condition is used.  (We did some work on this in a recent project, and built a static prototype to integrate with SoarDoc).

  * also need some way to view/understand the quantitative indifferent prefs.

The #1 annoying thing about the TSI is having to use a mouse and take my hands off the keyboard, and, more generally, UI issues.

I'm also worried about ATE integration issues with the debugger.  Likely there's not a lot to do about this, but I am concerned that that Soar toolsets arent going to be compatible, meaning I may lose some functionality when choosing one vs another.

Hope you find this helpful.

Bob