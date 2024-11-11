#ifdef __APPLE__

#include "main.hpp"

#define CommentType CommentType2
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#undef CommentType

#include <Geode/Geode.hpp> // my mortal enemy

void godFuckingDamnIt(cocos2d::CCImage* image, const char* filePath, int width, int height) {
    /*
	from ninXout:
	- yeah saveToFile doesn't work // steal from PRNTSCRN
	- i did manually // and windows i use saveToFile
	https://discord.com/channels/911701438269386882/911702535373475870/1305406054565154838
	*/
	CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, image->getData(), (width * height * 4), NULL);

	CGImageRef cgImg = CGImageCreate(
		width, height,
		8, 8 * 4, width * 4,
		CGColorSpaceCreateDeviceRGB(),
		kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast,
		provider,   // data provider
		NULL,       // decode
		true,       // should interpolate
		kCGRenderingIntentDefault
	);

	CFStringRef file = CFStringCreateWithCString(kCFAllocatorDefault,
		filePath,
		kCFStringEncodingMacRoman);
	CFStringRef type = CFSTR("public.png");
	CFURLRef urlRef = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, file, kCFURLPOSIXPathStyle, false );
	CGImageDestinationRef destination = CGImageDestinationCreateWithURL( urlRef, kUTTypePNG, 1, NULL );
	CGImageDestinationAddImage( destination, image, NULL );
	CGImageDestinationFinalize( destination );
	if (!destination) {
		return;
	}

	CGImageDestinationAddImage(destination, image, nil);

	if (!CGImageDestinationFinalize(destination)) {
		CFRelease(destination);
		return;
	}

	CFRelease(destination);
}

#endif