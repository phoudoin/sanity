#ifndef COLLAPSABLEBOX_H
#define COLLAPSABLEBOX_H

#include <InterfaceKit.h>

class CollapsableBox : public BControl
{
	public:
		// Constructors, destructors, operators...
		CollapsableBox(BRect frame, const char *name, const char *label, BMessage *msg,
						uint32 resize_mask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP | B_FRAME_EVENTS);

		typedef BControl		inherited;

		// Virtual function overrides
	public:
		virtual void AttachedToWindow(void);
		virtual void SetValue(int32 value);
		virtual void Draw(BRect updateRect);
		virtual void GetPreferredSize(float *width, float *height);
		virtual	void FrameResized(float new_width, float new_height);
		virtual void KeyDown(const char *bytes, int32 numBytes);
		virtual void MouseDown(BPoint point);
		virtual void MouseUp(BPoint point);
		virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *message);
		virtual void MessageReceived(BMessage *message);
		
		
		// From here, it's none of your business! ;-)
	private:
		enum {
			COLLAPSED,
			PRESSED,
			EXPANDED
		};

		bool 	m_pressing;
		BRect	m_collapsed_rect;
		BRect	m_expanded_rect;
		BPoint	m_click;
		
		void draw_switch(int state);
};

#endif // ifdef COLLAPSABLEBOX_H

