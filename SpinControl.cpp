/*******************************************************************************
/
/	File:			SpinControl.cpp
/
/	Description:	A TextControl for editing bounded numeric values.
/
/	Copyright (C) 2000, Carlos Hasan
/
*******************************************************************************/

#include <stdio.h>
#include "SpinControl.h"

enum {
	kSpinTextChanged   = 'spTx',
	kSpinButtonChanged = 'spBt'
};

SpinControl::SpinControl(BRect frame, const char *name, const char *label,
	BMessage *message, int32 minValue, int32 maxValue,
	int32 defaultValue, int32 stepValue, uint32 resizingFlags, uint32 flags) :
	BControl(frame, name, label, message, resizingFlags, flags),
	fTextControl(NULL), fSpinButton(NULL)
{
	fTextControl = new BTextControl(frame, "TextControl", label, B_EMPTY_STRING,
		new BMessage(kSpinTextChanged), B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);

	fSpinButton = new SpinButton(frame, "SpinButton",
		new BMessage(kSpinButtonChanged), minValue, maxValue,
		defaultValue, stepValue, B_FOLLOW_RIGHT | B_FOLLOW_TOP);

	fTextControl->TextView()->SetMaxBytes(10);

	for (uint32 code = 0; code < 256; code++) {
		if (!(code >= '0' && code <= '9') && code != '-' && code != '+')
			fTextControl->TextView()->DisallowChar(code);
	}
	
	SetValue(fSpinButton->Value());

	AddChild(fTextControl);
	AddChild(fSpinButton);
	
	if (frame.IsValid())
		FrameResized(frame.Width(), frame.Height());
}
	
SpinControl::~SpinControl()
{
}

void SpinControl::GetLimitValues(int32 *minValue, int32 *maxValue) const
{
	fSpinButton->GetLimitValues(minValue, maxValue);
}

void SpinControl::SetLimitValues(int32 minValue, int32 maxValue)
{
	fSpinButton->SetLimitValues(minValue, maxValue);
}

float SpinControl::Divider() const
{
	return fTextControl->Divider();
}

void SpinControl::SetDivider(float divider)
{
	fTextControl->SetDivider(divider);
}

void SpinControl::SetLabel(const char *label)
{
	fTextControl->SetLabel(label);
	BControl::SetLabel(label);
}

void SpinControl::SetValue(int32 value)
{
	char text[16];

	fSpinButton->SetValue(value);
	sprintf(text, "%ld", fSpinButton->Value());
	fTextControl->SetText(text);
	BControl::SetValue(fSpinButton->Value());
}

void SpinControl::SetEnabled(bool enabled)
{
	fTextControl->SetEnabled(enabled);
	fSpinButton->SetEnabled(enabled);
	BControl::SetEnabled(enabled);
}

void SpinControl::AttachedToWindow()
{
	SetValue(Value());
	fTextControl->SetTarget(this);
	fSpinButton->SetTarget(this);
	BControl::AttachedToWindow();
}

void SpinControl::FrameResized(float width, float height)
{
	float buttonWidth, buttonHeight;

	fSpinButton->GetPreferredSize(&buttonWidth, &buttonHeight);

	fTextControl->MoveTo(0, 0);
	fTextControl->ResizeTo(width - buttonWidth - 1, height);

	fSpinButton->MoveTo(width - buttonWidth, 0);
	fSpinButton->ResizeTo(buttonWidth, height - 1);

	BTextView *view = fTextControl->TextView();
	view->ScrollToOffset(0);
	view->ResizeTo(width - buttonWidth - fTextControl->Divider() - 10, height - 6);
	
	BControl::FrameResized(width, height);
}

void SpinControl::GetPreferredSize(float *width, float *height)
{
	float textWidth, textHeight, buttonWidth, buttonHeight;
	
	fTextControl->GetPreferredSize(&textWidth, &textHeight);
	fSpinButton->GetPreferredSize(&buttonWidth, &buttonHeight);
	
	*width = textWidth + buttonWidth;
	*height = max_c(textHeight, buttonHeight);
}

void SpinControl::MessageReceived(BMessage *message)
{
	if (message->what == kSpinButtonChanged) {
		SetValue(fSpinButton->Value());		
		Invoke();
	}
	else if (message->what == kSpinTextChanged) {
		int32 value;
		sscanf(fTextControl->Text(), "%ld", &value);
		SetValue(value);
		Invoke();
	}
#if B_BEOS_VERSION >= 0x0500
	else if (message->what == B_MOUSE_WHEEL_CHANGED) {
		fSpinButton->MessageReceived(message);
	}
#endif
	else {
		BControl::MessageReceived(message);
	}
}

