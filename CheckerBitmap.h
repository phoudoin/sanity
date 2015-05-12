#ifndef CHECKERBITMAP_H
#define CHECKERBITMAP_H

#include <InterfaceKit.h>

//Just creates a checkered bitmap in the classic image editor style
//white and grey, with tiles of 'size' 
class CheckerBitmap : public BBitmap
{

	public:
							CheckerBitmap(uint32 size);
							~CheckerBitmap();
		
		typedef BBitmap		inherited;
};

#endif
