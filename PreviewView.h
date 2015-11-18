#ifndef PREVIEWVIEW_H
#define PREVIEWVIEW_H

#include <Application.h>
#include <InterfaceKit.h>
#include <Cursor.h>


class PreviewView : public BView
{
	public:
		// Constructors, destructors, operators...
						PreviewView(BRect area);
						~PreviewView();
		
		typedef BView	inherited;
	
		// Virtual function overrides
	public:	
		virtual void 	Draw(BRect invalid);
		virtual void 	AttachedToWindow(void);
		virtual void 	FrameResized(float width, float height);
		virtual void	MouseDown(BPoint p);
		virtual void	MouseUp(BPoint p);
		virtual void	MouseMoved(BPoint point, uint32 transit, const BMessage *message);

		// Public methods
	public:
		void			SetBitmap(BBitmap * bitmap);
		BBitmap *		GetBitmap();
		
		void			SetGeometry(BRect geometry);
		void			SetFrame(BRect frame);
		void			SetImageFrame(void);

		// From here, it's none of your business! ;-)
	private:
		BRect		CenterImage();
		void		DrawBorders(BRect r);

		rgb_color	m_background_color;
		BRect		m_geometry;
		BRect		m_frame;
		BRect		m_image_frame;
		BBitmap *	m_image;
		BBitmap *	m_background;

		bool		m_mouse_button;
		BPoint		m_mouse_point_1;

		BCursor	*	m_move_cursor;
};

#endif	// ifdef PREVIEWVIEW_H
