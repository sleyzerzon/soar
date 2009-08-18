/*
 * =======================================================================
 *  File:  soarCommandUtils.c
 *
 * This file includes the utility routines for supporting the
 * Soar Command set, which is found in soarCommands.c.  
 *
 * =======================================================================
 *
 *
 * Copyright 1995-2003 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */



#include <ctype.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "soar.h"
#include "soarCommandUtils.h"
#include "soar_core_api.h"

#define MAXPATHLEN 1024



extern Tcl_Interp *tcl_soar_agent_interpreters[MAX_SIMULTANEOUS_AGENTS];



   
















#if 0

  FILE *f;
  bool eof, readnext, wme_added;
  int  cycle, numargs;
  unsigned long new_timetag, old_timetag;
  long loc;
  char id[256], attr[256], value[256];
  char input[1024], cmd[1024], result[1024];

  /* print("\nin replay_input_wme\n"); */
  f= current_agent(replay_fileID);
  eof = FALSE;
  readnext = TRUE;
  loc = ftell(f);
  /* really should switch on mode...ignore for now */


  while ((!feof(f)) && (readnext)) { 
    numargs = fscanf(f,"%i : %i :", &cycle, &old_timetag);
    /* print("\n\t numargs = %d \t cycle = %d \t timetag = %d\n",
       numargs,cycle,old_timetag); */
    if (cycle <= current_agent(d_cycle_count)) {
      fgets(input,1024,f);
      /* if this is a remove-wme, we need to use the old timetag */
      if (strstr(input,"remove-wme")) {
		sprintf(cmd,"remove-wme %d\n",
				current_agent(replay_timetags)[old_timetag]);
		
		
      } else {
		if (strstr(input,"add-wme")) wme_added = TRUE;
		/* add-wme or other can just be copied to be eval'd */
		sprintf(cmd,"%s\n",input);
      }

      Tcl_Eval(  tcl_soar_agent_interpreters[current_agent(id)],cmd);
      strcpy(result,  tcl_soar_agent_interpreters[current_agent(id)]->result);
	  /*      print("\nfrom replay file: %s\n", input); */
      /* if cmd was add-wme, need to compare and index the timetags */
      if (wme_added) {
		numargs = sscanf(result,"%d: ", &new_timetag);
		/*	print("wme_added: numargs = %d\n",numargs); */
		current_agent(replay_timetags)[old_timetag] = new_timetag;
		if (old_timetag != new_timetag && !current_agent(timetag_mismatch)) 
		  print("\n\nWARNING: timetags from replay file differ from current run!!");
		current_agent(timetag_mismatch) = TRUE;
		/*print("          result: %s\n", result);*/
      }
      loc = ftell(f);
    } else {
      fseek(f,loc,SEEK_SET);
      readnext = FALSE;
    }
  } 
  if (feof(f)) {
    /* warn user; turn off replay;  close file */
    strcpy(cmd,"echo WARNING!!!  Replay end of file reached.  Closing file.\n");
    Tcl_Eval( tcl_soar_agent_interpreters[current_agent(id)], cmd);
    strcpy(cmd,"replay-input -close\n");
    Tcl_Eval(  tcl_soar_agent_interpreters[current_agent(id)], cmd);
  }
}

#endif


/* DJP 4/3/96 */
/*----------------------------------------------------------------------
 *
 * install_tcl_soar_cmd --
 *
 *	This procedure installs a new Soar command in the Tcl
 *      interpreter.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	Command is installed in the Tcl interpreter.
 *
 *----------------------------------------------------------------------
 */

void
install_tcl_soar_cmd (agent * the_agent, 
		      char * cmd_name, 
		      Tcl_CmdProc * cmd_proc)
{
  Tcl_CreateCommand( tcl_soar_agent_interpreters[the_agent->id], 
		    cmd_name, cmd_proc, 
		    (ClientData) the_agent, NULL);
}





/*
 *----------------------------------------------------------------------
 *
 * soar_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_callback_to_tcl (soar_callback_agent the_agent, 
		      soar_callback_data data,
		      soar_call_data call_data)
{
  int code;

  code = Tcl_GlobalEval( tcl_soar_agent_interpreters[((agent *)the_agent)->id], 
			(char *) data);
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) data);
      print("Reason: %s\n", tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result);
      control_c_handler(0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * soar_input_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar input callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_input_callback_to_tcl (soar_callback_agent the_agent, 
			    soar_callback_data data,
			    soar_call_data call_data)
{
  Tcl_DString command;
  int mode;
  char * mode_name;
  int code;

  mode = (int) call_data;
  if (mode == TOP_STATE_JUST_CREATED)
    {
      mode_name = "top-state-just-created";
    }
  else if (mode == NORMAL_INPUT_CYCLE)
    {
      mode_name = "normal-input-cycle";
    }
  else if (mode == TOP_STATE_JUST_REMOVED)
    {
      mode_name = "top-state-just-removed";
    }
  else 
    {
      print("Error calling Tcl input procedure callback function -- unrecognized mode: %d", mode);
      return;
    }

  Tcl_DStringInit(&command);
  Tcl_DStringAppend(&command, data, strlen(data));
  Tcl_DStringAppendElement(&command, mode_name);

  code = Tcl_GlobalEval( tcl_soar_agent_interpreters[((agent *)the_agent)->id], 
			(char *) Tcl_DStringValue(&command));
 
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) Tcl_DStringValue(&command));
      print("Reason: %s\n", tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result);
      Tcl_DStringFree(&command);
      control_c_handler(0);
    }

 Tcl_DStringFree(&command);
}

/*
 *----------------------------------------------------------------------
 *
 * soar_output_callback_to_tcl --
 *
 *	This procedure invokes the Tcl interpreter of an agent
 *      from a Soar output callback.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Anything -- depends on script to invoke.
 *
 *----------------------------------------------------------------------
 */

void
soar_output_callback_to_tcl (soar_callback_agent the_agent, 
			     soar_callback_data data,
			     soar_call_data call_data)
{
  Tcl_DString command;
  output_call_info * output_info;
  int mode;
  io_wme * outputs, * wme;
  char * mode_name;
  int code;

  output_info = (output_call_info *) call_data;
  mode = output_info->mode;
  outputs = output_info->outputs;

  if (mode == ADDED_OUTPUT_COMMAND)
    {
      mode_name = "added-output-command";
    }
  else if (mode == MODIFIED_OUTPUT_COMMAND)
    {
      mode_name = "modified-output-command";
    }
  else if (mode == REMOVED_OUTPUT_COMMAND)
    {
      mode_name = "removed-output-command";
    }
  else 
    {
      print("Error calling Tcl output procedure callback function -- unrecognized mode: %d", mode);
      return;
    }

  Tcl_DStringInit(&command);
  Tcl_DStringAppend(&command, data, strlen(data));
  Tcl_DStringAppendElement(&command, mode_name);
  Tcl_DStringAppend(&command, " { ", 3);

  /* Build output list */
  for (wme = outputs; wme != NIL; wme = wme->next)
    {
      char wme_string[1000];
      char obj_string[1000];
      char attr_string[1000];
      char value_string[1000];

      symbol_to_string(wme->id,    FALSE, obj_string);
      symbol_to_string(wme->attr,  FALSE, attr_string);
      symbol_to_string(wme->value, FALSE, value_string);

      sprintf(wme_string, "%s %s %s", obj_string, attr_string, value_string);

      Tcl_DStringAppendElement(&command, wme_string);
    }

  Tcl_DStringAppend(&command, " }", 2);

  code = Tcl_GlobalEval( tcl_soar_agent_interpreters[((agent *)the_agent)->id], 
			(char *) Tcl_DStringValue(&command));
 
  if (code != TCL_OK)
    {
      print("Error: Failed callback attempt to globally eval: %s\n",
	     (char *) Tcl_DStringValue(&command));
      print("Reason: %s\n", tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result);
      Tcl_DStringFree(&command);
      control_c_handler(0);
    }

 Tcl_DStringFree(&command);
}





void
soar_ask_callback_to_tcl (soar_callback_agent the_agent, 
						  soar_callback_data data,
						  soar_call_data call_data)
{
  Tcl_DString command;
  int result;
  int num_candidates, code;
  preference *cand;


  Tcl_DStringInit(&command);
  Tcl_DStringAppend(&command, data, strlen(data));
  Tcl_DStringAppend(&command, " { ", 3);

  /* Build output list */
  num_candidates = 0;
  for ( cand = ((soar_apiAskCallbackData *)call_data)->candidates;
		cand != NIL; cand = cand->next_candidate ) {

	char candId[56];		

	symbol_to_string(cand->value, FALSE, candId);	
	Tcl_DStringAppendElement(&command, candId);
	num_candidates++;
  }

  Tcl_DStringAppend(&command, " }", 2);


  code = Tcl_GlobalEval( tcl_soar_agent_interpreters[((agent *)the_agent)->id], 
			(char *) Tcl_DStringValue(&command));

  if (code == TCL_OK) {
	result = atoi( tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result );
	if ( result < 0 || result >= num_candidates ) {
	  print( "Error: ask callback returned a value out of bounds: %d (0-%d)\n",
			 result, num_candidates-1);
	  result = 0;
	}
	
  }
  else {
	print("Error: Failed callback attempt to globally eval: %s\n",
		  (char *) Tcl_DStringValue(&command));
	print("Reason: %s\n", tcl_soar_agent_interpreters[((agent *)the_agent)->id]->result);
	Tcl_DStringFree(&command);
	result = 0;
	control_c_handler(0);
  }


  cand = ((soar_apiAskCallbackData *)call_data)->candidates;
  while( result > 0 ) { 
	cand = cand->next_candidate;
	result--;
  }
  *((soar_apiAskCallbackData *)call_data)->selection = cand;
 
 Tcl_DStringFree(&command);
}




Soar_TextWidgetPrintData *
Soar_MakeTextWidgetPrintData (Tcl_Interp * interp, const char * widget_name)
{
  Soar_TextWidgetPrintData * data;

  data = (Soar_TextWidgetPrintData *) 
         malloc (sizeof (Soar_TextWidgetPrintData));
  data->interp = interp;
  data->text_widget = savestring(widget_name);

  return data;
}

extern void Soar_FreeTextWidgetPrintData (Soar_TextWidgetPrintData * data)
{
  free((void *) data->text_widget);
  free((void *) data);
}

/* kjh(CUSP-B2) begin */
extern Symbol *make_symbol_for_current_lexeme (void);

typedef struct wme_filter_struct {
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  bool   adds;
  bool   removes;
} wme_filter;

