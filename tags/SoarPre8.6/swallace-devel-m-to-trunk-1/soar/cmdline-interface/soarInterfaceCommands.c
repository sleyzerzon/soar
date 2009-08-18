
#include "soarInterfaceCommands.h"
#include "soarkernel.h"
#include "parsing.h"
#include "demo_adder.h"
#include "demo_toh.h"


hash_table *gSoarCommands;
list *gDirectoryStack = NULL;




/*
 * SECTION 1:  INTERFACE-DEFINED COMMANDS
 *
 *  Within this first section, we define a number of commands
 *  particular to this shell-interface to Soar.  These include
 *  callbacks which are registered with the Soar kernel at startup
 *  as well a few commands which may be invoked directly by the 
 *  user.
 *
 */


/*
 *  This function is registered with the Soar kernel, and called 
 *  upon system termination.  It is here mainly for illustrative
 *  purposes.
 */
void cb_exit ( agent *the_agent, soar_callback_data d,soar_call_data c ) {
  
  if (c == (soar_call_data) TRUE ) {
    print ("Bye.\n");
    exit( 0 );
  }
}
  
/*
 *  This funtion is registered with the Soar kernel and called when
 *  Soar generates output.  In this simple shell-interface, we need only
 *  print that output using standard IO function calls
 */
void cb_print ( agent *the_agent, soar_callback_data d,	soar_call_data c ) {
  
	printf( "%s", (char *)c );
}



/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             set <what> <value>
 *
 *  It is intended to help maintain some amount of consistency with
 *  the Tcl interface but as can be seen, it has only minimal functionality
 */
int interface_Set( int argc, const char **argv, soarResult *res ) {
  
  if ( argc < 2 ) { 
    setSoarResultResult( res, "Too few arguments to 'set'\n" );
    return SOAR_ERROR;
  }

  if ( !strcmp( argv[1], "max_elaborations" ) ) {
    return soar_MaxElaborations( argc-1, &argv[1], res );
  }
  setSoarResultResult( res, "Don't know how to set '%s'", argv[1] );
  return SOAR_ERROR;
}



/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             pushd <dir>
 *
 *  It is intended to help maintain some amount of consistency with
 *  the Tcl interface, this function is not a part of the api because
 *  the particualr implementation may vary based upon OS and interface
 *  details.
 */
int interface_pushd( int argc, const char **argv, soarResult *res ) {
  
  cons *c;
  char *cdir;

  
  if ( argc < 1 ) { return SOAR_ERROR; }
  
  c = (cons *) malloc( sizeof( cons ) );
  cdir = (char *) malloc( 1024 * sizeof( char ) );
  getcwd( cdir, 1024 );
  c->first = cdir;
  c->rest = gDirectoryStack;
  gDirectoryStack = c;
  
  chdir( argv[1] );
  print( "Changing Directory to '%s'\n", argv[1] );
  
  return SOAR_OK;
}



/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             popd
 *
 *  It is intended to help maintain some amount of consistency with
 *  the Tcl interface, this function is not a part of the api because
 *  the particualr implementation may vary based upon OS and interface
 *  details.
 */
int interface_popd ( int argc, const char **argv, soarResult *res ) {
  
  cons *c;

  if ( gDirectoryStack == NULL ) {
    print ( "Directory Stack is empty!\n" );
    return SOAR_ERROR;
  }
  chdir( (char *)(gDirectoryStack->first) );
  c = gDirectoryStack;
  gDirectoryStack = gDirectoryStack->rest;
  
  free( c );
  return SOAR_OK;
}
  

/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             source <filename>
 *
 *  It is intended to help maintain some amount of consistency with
 *  the Tcl interface, this function is not a part of the api because
 *  the particualr implementation may vary based upon OS and interface
 *  details.
 */
int interface_Source( int argc, const char **argv, soarResult *res ) {

  FILE *f;
  bool eof_reached;

  if ( argc < 2 ) {
    setSoarResultResult( res, "No Filename specified\n" );
    return SOAR_ERROR;
  }
  
  f = fopen( argv[1], "r" );
  
  if ( !f ) {
    setSoarResultResult( res, "Could not open file '%s'\n", argv[1] );
    return SOAR_ERROR;
  }

  eof_reached = FALSE;
  while( !eof_reached ) {
    executeCommand( getCommandFromFile( fgetc,f,  &eof_reached ) );
  }
  
  fclose( f );
  clearSoarResultResult( res );
  return SOAR_OK;
}
  

/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             echo <what>
 *
 *  It is intended to help maintain some amount of consistency with
 *  the Tcl interface, this function is not a part of the api because
 *  the particualr implementation may vary based upon OS and interface
 *  details.  We use the soar "print" function to create output that 
 *  is treated in the same manner as that which is generated by the
 *  soar kernel.
 */
int interface_echo( int argc, const char **argv, soarResult *res ) {
  
  int i;
  
  for( i = 1; i < argc; i++ ) {
	print( "%s ", argv[i] );
  }
  print( "\n" );

  clearSoarResultResult( res );
  return SOAR_OK;
}


/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             counter-demo
 *
 *  It loads the agent, and initializes soar for running the demo
 */
int interface_counter_demo( int argc, const char **argv, soarResult *res ) {
  

  soar_cExciseAllProductions();
  soar_cReInitSoar();
  executeCommand( "source ./agents/adder.soar" );

  /*
   * remove any old input/output functions
   */
  soar_cRemoveAllCallbacksForEvent( soar_agent, INPUT_PHASE_CALLBACK );
  soar_cRemoveAllCallbacksForEvent( soar_agent, OUTPUT_PHASE_CALLBACK );

  /* 
   * Initialize global variables used by the IO functions
   */
  last_tt = -1;
  number_received = -1;


  /*
   *  Add input and output functions, to illustrate Soar IO.
   *  Built with this option, Soar can be run with the adder.soar
   *  agent.  This agent reads off the input-link and writes to the 
   *  output-link providing an extremely simple (but still useful) 
   *  example of how to connect Soar to an external environment.
   */
  
  soar_cAddOutputFunction( soar_cGetCurrentAgent(),
						   (soar_callback_fn) io_output_fn ,
						   NULL, NULL, "output-link" );

  soar_cAddInputFunction( soar_cGetCurrentAgent(),
						  (soar_callback_fn) io_input_fn ,
						  NULL, NULL,"input-link" );


  setSoarResultResult( res, "counter demo ready" );
  return SOAR_OK;
}



/*
 *  This is command is invoked by the user with the following
 *  syntax:
 *             toh-demo
 *
 *  It loads the agent, and initializes soar for running the demo
 */
int interface_toh_demo( int argc, const char **argv, soarResult *res ) {
  
  int type;
  
  if ( argc == 2 && !strcmp( argv[1], "-pause" ) ) type = 1;
  else if ( argc == 2 && !strcmp( argv[1], "-nowait" ) ) type = 2;
  else type = 0;
	
  soar_cExciseAllProductions();
  soar_cReInitSoar();
  executeCommand( "source ./agents/toh.soar" );
  soar_cSetWaitSNC( TRUE );
  /*
   * remove any old input/output functions
   */
  soar_cRemoveAllCallbacksForEvent( soar_agent, INPUT_PHASE_CALLBACK );
  soar_cRemoveAllCallbacksForEvent( soar_agent, OUTPUT_PHASE_CALLBACK );

  /*
   *  Add input and output functions, to illustrate Soar IO.
   *  Built with this option, Soar can be run with the adder.soar
   *  agent.  This agent reads off the input-link and writes to the 
   *  output-link providing an extremely simple (but still useful) 
   *  example of how to connect Soar to an external environment.
   */

  soar_cAddOutputFunction( soar_cGetCurrentAgent(),
						   (soar_callback_fn) toh_output_fn ,
						   (soar_callback_data)type, NULL, "output-link" );

  soar_cAddInputFunction( soar_cGetCurrentAgent(),
						  (soar_callback_fn) toh_input_fn ,
						  (soar_callback_data) type, NULL,"input-link" );

		
  switch ( type ) {
  case 0:
	setSoarResultResult( res, "toh demo ready (non-pausing-style)" );
	break;
  case 1:
	setSoarResultResult( res, "toh demo ready (pausing-style)" );
	break;
  case 2:
	setSoarResultResult(res,"toh demo ready (pausing-style w/ wait override)");
	soar_cPushCallback( soar_cGetCurrentAgent(), WAIT_CALLBACK,
						(soar_callback_fn) toh_wait_cb ,
						NULL, NULL );
	break;
  }
	


  return SOAR_OK;
}





/*
 * SECTION 2:  THE COMMAND TABLE
 *
 *  Within this second section, we define a number of functions
 *  used to deal with the global command table.  In essence functions
 *  which define the capabilities of the interface.
 *
 */


/*
 *  Initialize the command table.
 *
 *  The command table is a global structure (a hash table) which
 *  stores allows commands to be looked up by name, and invoked via a
 *  function pointer.  This function serves only to initilize this
 *  command table, and name each one of the commands availble to the
 *  user.  Note that commands may be added under more that one name,
 *  but that names must be unique.
 */
void init_soar_command_table( void ) {
  
  gSoarCommands = make_hash_table( 6, (hash_function)hash_soar_command );
  
  add_to_hash_table( gSoarCommands,
		     new_soar_command( ".", interface_Source ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "add-wme", soar_AddWme ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "build-info", soar_BuildInfo ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "counter-demo", interface_counter_demo ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "echo", interface_echo ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "excise", soar_Excise ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "indifferent-selection", 
				       soar_IndifferentSelection ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "init-soar",
				       soar_ReInitSoar ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "learn", soar_Learn ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "log", soar_Log ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "matches", soar_Matches ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "multi-attributes", 
				       soar_MultiAttributes ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "p", soar_Print ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "pf", soar_ProductionFind ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "popd", interface_popd ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "pref", soar_Preferences ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "print", soar_Print ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "pushd", interface_pushd ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "quit", soar_Quit ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "remove-wme", soar_RemoveWme ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "rete-net", soar_ReteNet ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "run", soar_Run ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "set", interface_Set ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "source", interface_Source ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "sp", soar_Sp ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "stats", soar_Stats ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "toh-demo", interface_toh_demo ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "v", soar_Verbose ) );  
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "verbose", soar_Verbose ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "w", soar_Watch ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "watch", soar_Watch ) );
#ifdef USE_CAPTURE_REPLAY
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "capture", soar_CaptureInput ) );
  add_to_hash_table( gSoarCommands,
		     new_soar_command( "replay", soar_ReplayInput ) );
#endif
}






int hash_soar_command_string( char *name, short nbits ) {
  
  int val, length;

  if ( name == NULL ) return 0;

  length = strlen( name );  
  val = length + 26 * (int)name[0];
  
  switch (length) {
    
  default:
    val += (int)name[3];
    
  case 3:
    val += (int)name[2];
    
  case 2:
    val += (int)name[1];

  case 1:
    break;
  }

  return val & masks_for_n_low_order_bits[nbits];
    
}  


/* 
 *   Return the Hash value for a soar_command structure 
 */

int hash_soar_command ( void *item, short nbits ) {
  return hash_soar_command_string( ((soar_command *)item)->command_name, 
				   nbits );
}



/*
 *  Create a new command structure, to be added into the global comman table
 */
soar_command *new_soar_command( char *name, 
				int (*cmd)(int, const char **, soarResult *) ) {

  soar_command *c;
  
  c = malloc( sizeof( soar_command ) );
  c->command_name = malloc( (strlen( name ) + 1) * sizeof(char) );
  strcpy( c->command_name, name );
  c->command = cmd;
  
  return c;
}
  
/*
 *  Check for the existence of a command.  If the command does indeed
 *  exist, return a structure containing its registry information
 *  (including a pointer to the function itself)
 */
soar_command *find_soar_command_structure( char *name ) {

  int commandHash;
  soar_command *theCommand;

  commandHash = hash_soar_command_string( name, gSoarCommands->log2size );
  theCommand = (soar_command *) (*(gSoarCommands->buckets + commandHash ));
  
  for ( ; theCommand != NIL; theCommand = theCommand->next ) {
    
    if ( !strcmp( name, theCommand->command_name ) ) {
      break; 
      
    }
  }
  return theCommand;
}
