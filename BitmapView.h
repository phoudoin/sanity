#ifndef BITMAPVIEW_H
#define BITMAPVIEW_H

#include <InterfaceKit.h>

class BitmapView : public BView
{
public:
		BitmapView(BRect frame, BBitmap * bitmap);
		~BitmapView();
		
		typedef BView	inherited;
	
virtual void 	Draw(BRect invalid);
virtual void 	AttachedToWindow(void);
virtual void 	FrameResized(float width, float height);

		void			SetBitmap(BBitmap * bitmap);
		BBitmap *		GetBitmap();

		// From here, it's none of your business! ;-)
private:
		void 		SetupScrollbars(void);

		BBitmap *	m_image;
		BBitmap *	m_background;
};

#endif	// ifdef BITMAPVIEW_H
