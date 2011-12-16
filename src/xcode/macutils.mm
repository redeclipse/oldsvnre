#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CFBundle.h>

char *mac_pasteconsole(int *cblen)
{
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSString *type = [pasteboard availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]];
    if(type != nil)
    {
        NSString *contents = [pasteboard stringForType:type];
        if(contents != nil)
        {
            NSUInteger len = [contents lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1; // 10.4+
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
 * 0x1030 = 10.3
 * 0x1040 = 10.4
 * 0x1050 = 10.5
 */
int mac_osversion() 
{
    SInt32 MacVersion;
    Gestalt(gestaltSystemVersion, &MacVersion);
    return MacVersion;
}

const char *mac_personaldir() {
    static string dir;
    NSString *path = nil;
    FSRef folder;
    if(FSFindFolder(kUserDomain, kApplicationSupportFolderType, NO, &folder) == noErr) 
    {
        CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, &folder);
        path = [(NSURL *)url path];
        CFRelease(url);
    }
    return path ? copystring(dir, [path fileSystemRepresentation]) : NULL;
}

const char *mac_sauerbratendir() {
    static string dir;
    NSString *path = [[NSWorkspace sharedWorkspace] fullPathForApplication:@"sauerbraten"];
    if(path) path = [path stringByAppendingPathComponent:@"Contents/gamedata"];
    return path ? copystring(dir, [path fileSystemRepresentation]) : NULL;
}

const char *mac_resourcedir(const char *what)
{
    static string dir;
    CFURLGetFileSystemRepresentation(CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFStringCreateWithCString(NULL, what, kCFStringEncodingASCII), NULL, NULL), true, (UInt8*)dir, MAXSTRLEN);
    return dir;
}
