#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CFBundle.h>

#define MAXSTRLEN 512 // must be at least 512 bytes to comply with rfc1459
typedef char string[MAXSTRLEN];
inline char *s_strncpy(char *d, const char *s, size_t m) { strncpy(d, s, m); d[m-1] = 0; return d; };
inline char *s_strcpy(char *d, const char *s, size_t m = MAXSTRLEN) { return s_strncpy(d, s, m); }
inline char *s_strcat(char *d, const char *s) { size_t n = strlen(d); return s_strncpy(d+n, s, MAXSTRLEN-n); };
#ifndef NSUInteger
#if __LP64__ || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
typedef unsigned long NSUInteger;
#else
typedef unsigned int NSUInteger;
#endif
#endif

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
 * 0x1040 = 10.4
 * 0x1050 = 10.5
 * 0x1060 = 10.6
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
    return path ? s_strcpy(dir, [path fileSystemRepresentation]) : NULL;
}

const char *mac_sauerbratendir() {
    static string dir;
    NSString *path = [[NSWorkspace sharedWorkspace] fullPathForApplication:@"sauerbraten"];
    if(path) path = [path stringByAppendingPathComponent:@"Contents/gamedata"];
    return path ? s_strcpy(dir, [path fileSystemRepresentation]) : NULL;
}

const char *mac_resourcedir(const char *what)
{
    static string dir;
    CFURLGetFileSystemRepresentation(CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFStringCreateWithCString(NULL, what, kCFStringEncodingASCII), NULL, NULL), true, (UInt8*)dir, MAXSTRLEN);
    return dir;
}
