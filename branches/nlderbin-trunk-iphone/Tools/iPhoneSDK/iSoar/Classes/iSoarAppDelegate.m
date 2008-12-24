//
//  iSoarAppDelegate.m
//  iSoar
//
//  Created by Nate on 12/24/08.
//  Copyright The Family 2008. All rights reserved.
//

#import "iSoarAppDelegate.h"

@implementation iSoarAppDelegate

@synthesize window;


- (void)applicationDidFinishLaunching:(UIApplication *)application {    

    // Override point for customization after application launch
    [window makeKeyAndVisible];
}


- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
