#include <stdlib.h>

#include "StackView.h"

#ifdef CODEWARRIOR
	#pragma mark [Constructor & destructor]
#endif

// --------------------------------------------------------------
StackView::StackView
	(
	BRect			frame,
    const char *	name, 
    uint32 			resizeMask, 
  	uint32 			flags
	)
: BView(frame, name, resizeMask, flags | B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP)
{
	rgb_color bg = ui_color(B_PANEL_BACKGROUND_COLOR);
	SetViewColor(tint_color(bg, B_LIGHTEN_1_TINT));
	
	m_height = 0;
	m_width = frame.Width();
	m_scroller = NULL;
}


//	#pragma mark -

// --------------------------------------------------------------
void StackView::TargetedByScrollView
	(
	BScrollView *	scrollview
	)
{
	m_scroller = scrollview;
	SetupScroller();
}


// --------------------------------------------------------------
void StackView::FrameResized
	(
	float width,
	float height
	)
{
	inherited::FrameResized(width, height);
	
	SetupScroller();
}


// --------------------------------------------------------------
void StackView::AddChild
	(
	BView *	view
	)
{
	view->MoveTo(Bounds().left, m_height - Bounds().top);
	m_height += view->Bounds().Height() + 1.0;
	
	inherited::AddChild(view);


	SetupScroller();
}


// --------------------------------------------------------------
void StackView::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
	case RESTACK_MSG: {
		BView *child;
		BRect r = Bounds();

		m_height = 0;
		child = ChildAt(0);
		while (child) {
			child->MoveTo(r.left, m_height - r.top);
			m_height += child->Bounds().Height() + 1.0;
			child = child->NextSibling();
		};
	
		SetupScroller();
		break;
	}
	
	default:
		inherited::MessageReceived(msg);
	}
}

// #pragma mark -

// --------------------------------------------------------------
void StackView::SetupScroller()
{
	BScrollBar *sb;
	float visible_height;

	if ( ! m_scroller )
		return;

	visible_height = m_scroller->Bounds().Height();

	// Hacky: Scrollview's Bounds() don't account of (inner) border style!
	// Compensate by hand...
	float h = visible_height;
	if (m_scroller->Border() == B_FANCY_BORDER)
		h -= 4;
	else if (m_scroller->Border() == B_PLAIN_BORDER)
		h -= 2;

	if ( (m_height-1) < h )
		ResizeTo(Bounds().Width(), h);

	sb = m_scroller->ScrollBar(B_VERTICAL);
	if ( ! sb )
		return;

	sb->SetProportion(h / (m_height-1));
	if ( (m_height-1) <= h )
		sb->SetRange(0, 0);
	else 
		sb->SetRange(0, m_height - h - 1);	// 0 to  + (m_height-1) - h
}
