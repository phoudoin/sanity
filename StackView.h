#ifndef STACKVIEW_H
#define STACKVIEW_H

#include <InterfaceKit.h>

/* A small view that stack vertically his children's views.
   Any subviews can sent him a RESTACK_MSG to force an stack layout update
*/

class StackView : public BView  
{
public:
		// Constructors, destructors, operators...
		StackView(BRect frame, const char * name,
					uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
					uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP);
						
		typedef BView 			inherited;

		enum {
			RESTACK_MSG = 1
		};

		// Virtual function overrides
virtual void	AddChild(BView * view);
virtual void	TargetedByScrollView(BScrollView * scrollview);
virtual void	FrameResized(float width, float height);
virtual void 	MessageReceived(BMessage *message);
		
		// From here, it's none of your business! ;-)
private:
		void			SetupScroller();
	
		BScrollView *	m_scroller;
		float			m_height;
		float			m_width;
};

#endif // ifdef STACKVIEW_H

