//
//  main.m
//  iSoar
//
//  Created by Nate on 12/24/08.
//  Copyright The Family 2008. All rights reserved.
//

#include "sml_Connection.h"
#include "sml_Client.h"
#include "ElementXML.h"
#include <iostream>

#import <UIKit/UIKit.h>

// simple console print callback
void iSoar_PrintCallbackHandler( sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage ) {
	std::cout << pMessage;	// simply display whatever comes back through the event
}

int main(int argc, char *argv[]) {
	
	/////////////////////////////////////////////////////////////////
	// Soar Demo
	//
	// 1. scons iphone=yes device=no
	// 2. Create iPhone project in XCode
	// 3. Project->Edit Project Settings->Build->Header Search Paths, point to:
	//    - Core/ClientSML/include
	//    - Core/ConnectionSML/include
	//    - Core/ElementXML/include
	//    - Core/shared
	// 4. Add all lib*.a in SoarLibrary/lib to project (by reference)
	// 5. Relevant source files should be named *.mm instead of *.m (Objective-C++)
	// 6. Develop under simulator
	// 7. When ready for device: 
	//    - scons iphone=yes device=yes (compiles to arm)
	//    - target info: build=code signing, properties=identifier/name
	/////////////////////////////////////////////////////////////////
	sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread();
	
	assert( pKernel );
	if ( pKernel->HadError() ) 
	{
		std::cout << "Error: " << pKernel->GetLastErrorDescription() << std::endl;
		exit(1);
	}
	
	sml::Agent* pAgent;
	pAgent = pKernel->CreateAgent( "iSoar" ) ;
	assert( pAgent );
	
	pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, iSoar_PrintCallbackHandler, 0 );
	
	std::string output;	
	output = pKernel->ExecuteCommandLine( "print --depth 100 --tree s1", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "sp {test (state <s> ^type <t>) --> (<s> ^howdy <t>) }", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "step", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "print --depth 100 --tree s1", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "rl", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "step", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "step", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "step", "iSoar" );
	std::cout << output;
	output = pKernel->ExecuteCommandLine( "step", "iSoar" );
	std::cout << output;
	
	delete pKernel;	
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}
