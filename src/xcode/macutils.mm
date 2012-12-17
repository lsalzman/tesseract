#import <Cocoa/Cocoa.h>

char *mac_pasteconsole(int *cblen)
{	
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSString *type = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]];
    if(type != nil) 
    {
        NSString *contents = [pasteboard stringForType:type];
        if(contents != nil)
        {
            int len = (int)[contents lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1; // 10.4+
            if(len > 1)
            {
                char *buf = (char *)malloc(len);
                if(buf)
                {
                    if([contents getCString:buf maxLength:len encoding:NSUTF8StringEncoding]) // 10.4+
                    {
                        *cblen = len;
                        return buf;
                    }
                    free(buf);
                }
            }
        }
    }
    return NULL;
}

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
