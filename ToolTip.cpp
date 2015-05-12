/*******************************************************************************
/
/	File:			ToolTip.cpp
/
/	Description:	A Window that displays tips for the views in a window.
/
/	Copyright (C) 2000, Carlos Hasan
/
*******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <Screen.h>
#include <MessageFilter.h>
#include "ToolTip.h"

enum {
	kTooltipStartMessage = 'ttSt',
	kTooltipShowMessage	 = 'ttSw'
};

enum {
	kTooltipDelayTime	 = 1000000,
	kTooltipHoldTime	 = 6000000,		// 6 seconds
	kTooltipSleepTime	 = 0
};

/* ToolTipWatcher */

class ToolTipWatcher : public BMessageFilter {
	public:
		ToolTipWatcher(ToolTip *tooltip);
		
		virtual ~ToolTipWatcher();
		
		virtual filter_result Filter(BMessage *message, BHandler **target);
	
	private:
		ToolTip *fToolTip;
};

ToolTipWatcher::ToolTipWatcher(ToolTip *tip) :
	BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE), fToolTip(tip)
{
}

ToolTipWatcher::~ToolTipWatcher()
{
	if (fToolTip->Lock())
		fToolTip->Quit();
}

filter_result ToolTipWatcher::Filter(BMessage *message, BHandler **target)
{
	if (message->what == B_MOUSE_MOVED) {
		BMessage showMessage(kTooltipStartMessage);
		showMessage.AddPoint("where", message->FindPoint("where"));
		fToolTip->PostMessage(&showMessage);
	}
	else if (message->what == B_MOUSE_DOWN
			 || message->what == B_MOUSE_UP
#if B_BEOS_VERSION >= 0x0500
			 || message->what == B_MOUSE_WHEEL_CHANGED)
#else
			 )
#endif
	{
		fToolTip->PostMessage(kTooltipShowMessage);
	}
	return B_DISPATCH_MESSAGE;
}
		

/* ToolTipView */

class ToolTipView : public BStringView {
	public:
		ToolTipView(BRect frame, const char *name, const char *text,
			uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW);
		
		virtual ~ToolTipView();
		
		virtual void AttachedToWindow();
		
		virtual void GetPreferredSize(float *width, float *height);
		
		virtual void Draw(BRect updateRect);
};

ToolTipView::ToolTipView(BRect frame, const char *name, const char *text,
	uint32 resizingMode, uint32 flags) :
	BStringView(frame, name, text, resizingMode, flags)
{
	SetAlignment(B_ALIGN_CENTER);
}

ToolTipView::~ToolTipView()
{
}

void ToolTipView::AttachedToWindow()
{
	BStringView::AttachedToWindow();
	
	// SetViewColor(ui_color(B_WINDOW_TAB_COLOR));
	SetViewColor(210,240,255);
	SetLowColor(ViewColor());
	SetHighColor(tint_color(tint_color(ViewColor(), B_DARKEN_4_TINT), B_DARKEN_4_TINT));
}

void ToolTipView::GetPreferredSize(float *width, float *height)
{
	BStringView::GetPreferredSize(width, height);
	
	*width += 4;
}

void ToolTipView::Draw(BRect updateRect)
{
	BRect rect = Bounds();

	rgb_color lightColor = tint_color(ViewColor(), B_LIGHTEN_2_TINT);
	rgb_color darkColor = tint_color(ViewColor(), B_DARKEN_1_TINT);

	BeginLineArray(4);
	
	AddLine(rect.LeftBottom(), rect.LeftTop(), lightColor);
	AddLine(rect.LeftTop(), rect.RightTop(), lightColor);
	
	AddLine(rect.RightTop(), rect.RightBottom(), darkColor);	
	AddLine(rect.RightBottom(), rect.LeftBottom(), darkColor);
	
	EndLineArray();

	BStringView::Draw(updateRect);
}


/* ToolTip */

ToolTip::ToolTip(BWindow *window, const char *name) :
	BWindow(BRect(), name, B_NO_BORDER_WINDOW_LOOK, // B_BORDERED_WINDOW_LOOK,
		B_FLOATING_ALL_WINDOW_FEEL, B_NOT_MOVABLE | B_AVOID_FOCUS),
	fWindow(window), fView(NULL), fWhere(), fTips()
{
	fView = new ToolTipView(Bounds(), "ToolTipView",
		NULL, B_FOLLOW_ALL, B_WILL_DRAW | B_PULSE_NEEDED);
	
	AddChild(fView);
	
	MoveTo(-1000, -1000);
	Show();

	SetPulseRate(kTooltipSleepTime);
	
	if (fWindow->Lock()) {
		fWindow->AddCommonFilter(new ToolTipWatcher(this));
		fWindow->Unlock();
	}
}

ToolTip::~ToolTip()
{
	for (int32 index = 0; index < fTips.CountItems(); index += 2) {
		free(fTips.ItemAt(index + 1));
	}
}

void ToolTip::SetText(BView *view, const char *text)
{
	int32 index = fTips.IndexOf(view);
	if (index >= 0) {
		free(fTips.ItemAt(index + 1));
		fTips.ReplaceItem(index + 1, strdup(text));
	}
	else {
		fTips.AddItem(view);
		fTips.AddItem(strdup(text));
	}
}

const char *ToolTip::Text(BView *view) const
{
	while (view != NULL) {
		int32 index = fTips.IndexOf(view);
		if (index >= 0)
			return (const char *) fTips.ItemAt(index + 1);
		view = view->Parent();
	}
	return NULL;
}

void ToolTip::DispatchMessage(BMessage *message, BHandler *target)
{
	if (message->what == kTooltipStartMessage) {
		BPoint where = message->FindPoint("where");
		if (where != fWhere) {
			fWhere = where;
			MoveTo(-1000, -1000);
			SetPulseRate(kTooltipDelayTime);
		}
	}
	else if (message->what == kTooltipShowMessage) {
		MoveTo(-1000, -1000);
		SetPulseRate(kTooltipDelayTime);
	}
	else if (message->what == B_PULSE) {
		if (PulseRate() == kTooltipDelayTime) {
			if (fWindow->Lock()) {
				if (fWindow->IsActive()) {
					const char *text = Text(fWindow->FindView(fWhere));
					if (text != NULL) {
						float width, height;

						fView->SetText(text);
						fView->GetPreferredSize(&width, &height);

						ResizeTo(width, height);
						
						BRect screen = BScreen(this).Frame();
						BPoint point = fWindow->ConvertToScreen(fWhere);
						point.x -= (point.x >= 0.5f * screen.Width() ? width + 8 : -16);
						point.y -= (point.y >= 0.5f * screen.Height() ? height + 8 : -16);
		
						UpdateIfNeeded();
	
						MoveTo(point);
					}
				}
				fWindow->Unlock();
			}
			SetPulseRate(kTooltipHoldTime);
		}
		else {
			MoveTo(-1000, -1000);
			SetPulseRate(kTooltipSleepTime);
		}
	}
	else {
		BWindow::DispatchMessage(message, target);
	}
}
