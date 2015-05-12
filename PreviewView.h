#ifndef PREVIEWVIEW_H
#define PREVIEWVIEW_H

#include <InterfaceKit.h>


class PreviewView : public BView
{
	public:
		// Constructors, destructors, operators...
						PreviewView(BRect frame, BRect area);
						~PreviewView();
		
		typedef BView	inherited;
	
		// Virtual function overrides
	public:	
		virtual void 	Draw(BRect invalid);
		virtual void 	AttachedToWindow(void);
		virtual void 	FrameResized(float width, float height);

		// Public methods
	public:
		void			SetBitmap(BBitmap * bitmap);
		BBitmap *		GetBitmap();
		
		void			SetGeometry(BRect geometry);

		// From here, it's none of your business! ;-)
	private:
		BRect		CenterImage();
		void		DrawBorders(BRect r);
	
		rgb_color	m_background_color;
		BRect		m_geometry;
		BBitmap *	m_image;
		BBitmap *	m_background;
};

#endif	// ifdef PREVIEWVIEW_H
