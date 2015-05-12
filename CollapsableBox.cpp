
#include "CollapsableBox.h"

#ifdef CODEWARRIOR
	#pragma mark [Constructor & destructor]
#endif

// Constructor & destructor
// ------------------------

// --------------------------------------------------------------
CollapsableBox::CollapsableBox
	(
	BRect		frame,
    const char 	*name,
    const char 	*label,
    BMessage 	*msg,
    uint32 		resize_mask, 
  	uint32 		flags
	)
: BControl(frame, name, label, msg, resize_mask,
	flags | B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE | B_NAVIGABLE_JUMP)
{
	m_pressing = false;

	font_height fh;
	GetFontHeight(&fh);
	float min_height = (float) ceil(6.0f + fh.ascent + fh.descent) + 2.0f;
	if (Bounds().Height() < min_height)
		ResizeTo(Bounds().Width(), min_height);
		
	m_collapsed_rect = BRect(0, 0, Bounds().Width(), min_height);
	m_expanded_rect = Bounds();

	SetValue(B_CONTROL_ON);
}


#ifdef CODEWARRIOR
	#pragma mark [Public methods]
#endif

// Public methods
// --------------

void CollapsableBox::AttachedToWindow(void)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	// if (Parent())
	//	SetViewColor(Parent()->ViewColor());
}


void CollapsableBox::SetValue(int32 new_value)
{
	if (new_value != Value()) {
		BRect r = Bounds();
		
		if (!new_value)
			ResizeTo(r.Width(), m_collapsed_rect.Height());
		else
			ResizeTo(r.Width(), m_expanded_rect.Height());
		
		BView *child;
		
		child = ChildAt(0);
		while (child) {
			if (new_value)
				child->Show();
			else
				child->Hide();
			child = child->NextSibling();
		};
			
		Invalidate();
		BControl::SetValue(new_value);
		Invoke();
	};
	
}

void CollapsableBox::GetPreferredSize(float *width, float *height)
{
	font_height fh;
	GetFontHeight(&fh);

	*height = (float) ceil(6.0f + fh.ascent + fh.descent) + 2.0f;
	*width = 12.0f + fh.ascent;
	
	if (Label())
		*width += StringWidth(Label());

	*width = (float) ceil(*width);
}

void CollapsableBox::FrameResized(float width, float height)
{
	if (height > m_collapsed_rect.Height())
		m_expanded_rect = BRect(0, 0, width, height);
	BControl::FrameResized(width, height);
	Invoke();
}

void CollapsableBox::Draw(BRect updateRect)
{
	font_height fh;
	GetFontHeight(&fh);
	BRect r = Bounds();
	
	bool show_focus = IsEnabled() && IsFocus() && Window()->IsActive();

	// If the focus is changing, just redraw the focus indicator
	if (IsFocusChanging()) {
		float x = (float) ceil(10.0f + fh.ascent);
		float y = 5.0f + (float)ceil(fh.ascent);

		SetHighColor(show_focus ? ui_color(B_KEYBOARD_NAVIGATION_COLOR) : ViewColor());
		StrokeLine(BPoint(x, y), BPoint(x + StringWidth(Label()), y));

		if (show_focus)
			SetHighColor(255, 255, 255, 255);
		else			
			SetHighColor(ViewColor());
		StrokeLine(BPoint(x, y+1), BPoint(x + StringWidth(Label()), y+1));
		return;
	};

	SetHighColor(tint_color(ViewColor(), B_LIGHTEN_MAX_TINT));
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.top));
	StrokeLine(BPoint(r.left+1.0f, r.top), BPoint(r.right, r.top));
	SetHighColor(tint_color(ViewColor(), B_DARKEN_2_TINT));
	StrokeLine(BPoint(r.left+1.0f, r.bottom), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right, r.bottom), BPoint(r.right, r.top+1.0f));

	rgb_color no_tint = ui_color(B_PANEL_BACKGROUND_COLOR),
	lighten1 = tint_color(no_tint, B_LIGHTEN_1_TINT),
	lightenmax = tint_color(no_tint, B_LIGHTEN_MAX_TINT),
	darken1 = tint_color(no_tint, B_DARKEN_1_TINT),
	darken2 = tint_color(no_tint, B_DARKEN_2_TINT),
	darken3 = tint_color(no_tint, B_DARKEN_3_TINT),
	darken4 = tint_color(no_tint, B_DARKEN_4_TINT),
	darkenmax = tint_color(no_tint, B_DARKEN_MAX_TINT);

	if (m_pressing)
		draw_switch(PRESSED);
	else if (Value())
		draw_switch(EXPANDED);
	else
		draw_switch(COLLAPSED);

		// Label
	if (!IsEnabled())
		SetHighColor(tint_color(no_tint, B_DISABLED_LABEL_TINT));
	else
		SetHighColor(darkenmax);
	DrawString(Label(), BPoint((float)ceil(10.0f + fh.ascent),
		3.0f + (float)ceil(fh.ascent)));

		// Focus
	if (show_focus || !Window()->IsActive()) {
		float x = (float)ceil(10.0f + fh.ascent);
		float y = 5.0f + (float)ceil(fh.ascent);

		SetHighColor(show_focus ? ui_color(B_KEYBOARD_NAVIGATION_COLOR) : ViewColor());
		StrokeLine(BPoint(x, y), BPoint(x + StringWidth(Label()), y));

		if (show_focus)
			SetHighColor(255, 255, 255, 255);
		else			
			SetHighColor(ViewColor());
		StrokeLine(BPoint(x, y+1), BPoint(x + StringWidth(Label()), y+1));
	};
}

void CollapsableBox::KeyDown(const char *bytes, int32 numBytes)
{
	if (bytes[0] == B_ENTER || bytes[0] == B_SPACE)
		SetValue(Value() == B_CONTROL_OFF ? B_CONTROL_ON : B_CONTROL_OFF);
	else
		BControl::KeyDown(bytes, numBytes);
}

void CollapsableBox::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;

	Window()->Activate();

	uint32 buttons;
	
	GetMouse(&where, &buttons);
	if ((buttons & B_PRIMARY_MOUSE_BUTTON) == 0)
		return;
		
	if (! m_collapsed_rect.Contains(where))
		return;
	
	m_click = where;
	SetMouseEventMask(B_POINTER_EVENTS,
		B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
	SetTracking(true);
		
	m_pressing = true;
	Draw(Bounds());
	Flush();
}

void CollapsableBox::MouseMoved(BPoint where, uint32 transit, const BMessage *message)
{
	if (!IsTracking())
		return;

	if (where == m_click)
		return;

	m_click = where;
	bool inside = m_collapsed_rect.Contains(where);

	if (m_pressing != inside) {
		m_pressing = inside;
		Draw(Bounds());
		Flush();
	};
}

void CollapsableBox::MouseUp(BPoint where)
{
	if (!IsTracking())
		return;

	bool inside = m_collapsed_rect.Contains(where);
	if (inside)
		SetValue(Value() == B_CONTROL_OFF ? B_CONTROL_ON : B_CONTROL_OFF);
	else {
		Draw(Bounds());
		Flush();
	};

	m_pressing = false;

	SetTracking(false);		
	SetMouseEventMask(0, 0);
}

void CollapsableBox::MessageReceived(BMessage *message)
{
	BControl::MessageReceived(message);
}


#ifdef CODEWARRIOR
	#pragma mark [Private stuffs]
#endif

// Privates methods
// ----------------


void CollapsableBox::draw_switch(int state)
{
	BRect rect(4, 4, 14, 14);
	
	SetHighColor(ViewColor());
	FillRect(rect);
	
	rgb_color switch_color = {150,150,255, 255};

	rgb_color light_color = tint_color(switch_color, B_LIGHTEN_2_TINT);
	rgb_color dark_color = tint_color(switch_color, B_DARKEN_2_TINT);

	rgb_color outlineColor = dark_color; // {0, 0, 0, 255};
	rgb_color middleColor = state == PRESSED ? light_color : switch_color;

	SetDrawingMode(B_OP_COPY);
	
	switch (state) {
		case COLLAPSED:
			BeginLineArray(6);
			
			AddLine(BPoint(rect.left + 3, rect.top + 1), 
					BPoint(rect.left + 3, rect.bottom - 1), outlineColor);
			AddLine(BPoint(rect.left + 3, rect.top + 1), 
					BPoint(rect.left + 7, rect.top + 5), outlineColor);
			AddLine(BPoint(rect.left + 7, rect.top + 5), 
					BPoint(rect.left + 3, rect.bottom - 1), outlineColor);
					
			AddLine(BPoint(rect.left + 4, rect.top + 3), 
					BPoint(rect.left + 4, rect.bottom - 3), middleColor);
			AddLine(BPoint(rect.left + 5, rect.top + 4), 
					BPoint(rect.left + 5, rect.bottom - 4), middleColor);
			AddLine(BPoint(rect.left + 5, rect.top + 5), 
					BPoint(rect.left + 6, rect.top + 5), middleColor);
			EndLineArray();
			break;

		case PRESSED:
			BeginLineArray(7);
				AddLine(BPoint(rect.left + 1, rect.top + 7), 
					BPoint(rect.left + 7, rect.top + 7), outlineColor);
				AddLine(BPoint(rect.left + 7, rect.top + 1), 
					BPoint(rect.left + 7, rect.top + 7), outlineColor);
				AddLine(BPoint(rect.left + 1, rect.top + 7), 
					BPoint(rect.left + 7, rect.top + 1), outlineColor);
					
				AddLine(BPoint(rect.left + 3, rect.top + 6), 
					BPoint(rect.left + 6, rect.top + 6), middleColor);
				AddLine(BPoint(rect.left + 4, rect.top + 5), 
					BPoint(rect.left + 6, rect.top + 5), middleColor);
				AddLine(BPoint(rect.left + 5, rect.top + 4), 
					BPoint(rect.left + 6, rect.top + 4), middleColor);
				AddLine(BPoint(rect.left + 6, rect.top + 3), 
					BPoint(rect.left + 6, rect.top + 4), middleColor);
			EndLineArray();
			break;

		case EXPANDED:
			BeginLineArray(6);
			AddLine(BPoint(rect.left + 1, rect.top + 3), 
				BPoint(rect.right - 1, rect.top + 3), outlineColor);
			AddLine(BPoint(rect.left + 1, rect.top + 3), 
				BPoint(rect.left + 5, rect.top + 7), outlineColor);
			AddLine(BPoint(rect.left + 5, rect.top + 7), 
				BPoint(rect.right - 1, rect.top + 3), outlineColor);

			AddLine(BPoint(rect.left + 3, rect.top + 4), 
				BPoint(rect.right - 3, rect.top + 4), middleColor);
			AddLine(BPoint(rect.left + 4, rect.top + 5), 
				BPoint(rect.right - 4, rect.top + 5), middleColor);
			AddLine(BPoint(rect.left + 5, rect.top + 5), 
				BPoint(rect.left + 5, rect.top + 6), middleColor);
			EndLineArray();
			break;
	}
}


