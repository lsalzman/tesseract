#import <Cocoa/Cocoa.h>

/*
 * 0x0A0400 = 10.4
 * 0x0A0500 = 10.5
 * 0x0A0600 = 10.6
 */
int mac_osversion() 
{
    SInt32 majorVersion = 0, minorVersion = 0, bugVersion = 0;
    Gestalt(gestaltSystemVersionMajor, &majorVersion);
    Gestalt(gestaltSystemVersionMinor, &minorVersion);
    Gestalt(gestaltSystemVersionBugFix, &bugVersion);
    return (majorVersion<<16) | (minorVersion<<8) | bugVersion;
}
