/*******************************************************************************
/
/	File:			SpinButton.cpp
/
/	Description:	A Button that increases or decreases a numeric value.
/
/	Copyright (C) 2000, Carlos Hasan
/
*******************************************************************************/

#include <Alert.h>
#include <Application.h>
#include "SpinButton.h"

enum {
	kSpinButtonNone,
	kSpinButtonUp,
	kSpinButtonDown,
	kSpinButtonWheel
};

enum {
	kSpinButtonThreshold = 5
};

static const unsigned char kVerticalResizeCursor[] = {
	16, 1, 7, 7,
	
	0x00, 0x00,	0x00, 0x00,
	0x00, 0x00,	0x01, 0x00,
	0x03, 0x80,	0x07, 0xc0,
	0x00, 0x00,	0x3f, 0xf8,
	0x00, 0x00,	0x07, 0xc0,
	0x03, 0x80,	0x01, 0x00,
	0x00, 0x00,	0x00, 0x00,
	0x00, 0x00,	0x00, 0x00,
	
	0x00, 0x00,	0x00, 0x00,
	0x01, 0x00,	0x03, 0x80,
	0x07, 0xc0,	0x0f, 0xe0,
	0x7f, 0xfc,	0x7f, 0xfc,
	0x7f, 0xfc,	0x0f, 0xe0,
	0x07, 0xc0,	0x03, 0x80,
	0x01, 0x00,	0x00, 0x00,
	0x00, 0x00,	0x00, 0x00
};

SpinButton::SpinButton(BRect frame, const char *name, BMessage *message,
	int32 minValue, int32 maxValue, int32 defaultValue, int32 stepValue,
	uint32 resizingFlags, uint32 flags) :
	BControl(frame, name, B_EMPTY_STRING, message, resizingFlags, flags),
	fMinValue(minValue), fMaxValue(maxValue), fDefaultValue(defaultValue),
	fStepValue(stepValue), fButton(kSpinButtonNone), fPoint()
{
	SetValue(fDefaultValue);
	ResizeToPreferred();
}

SpinButton::~SpinButton()
{
}

void SpinButton::GetLimitValues(int32 *minimum, int32 *maximum) const
{
	*minimum = fMinValue;
	*maximum = fMaxValue;
}

void SpinButton::SetLimitValues(int32 minimum, int32 maximum)
{
	fMinValue = minimum;
	fMaxValue = maximum;
}

void SpinButton::SetValue(int32 value)
{
	if (value <= fMinValue)
		value = fMinValue;
	if (value >= fMaxValue)
		value = fMaxValue;
	BControl::SetValue(value);
}

void SpinButton::GetPreferredSize(float *width, float *height)
{
	*width = 14;
	*height = 17;
}

void SpinButton::Draw(BRect updateRect)
{
	BRect rect = Bounds();
	
	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(rect);
	}
	else {
		DrawButton(rect, kSpinButtonNone);
	}

	DrawButton(ButtonFrame(kSpinButtonUp), kSpinButtonUp);
	DrawButton(ButtonFrame(kSpinButtonDown), kSpinButtonDown);
}

void SpinButton::DrawButton(BRect rect, int32 button)
{
	rgb_color color = ViewColor();
	
	rgb_color topColor = tint_color(color,
		button == kSpinButtonNone ?
			(IsEnabled() ? B_DARKEN_2_TINT : B_DARKEN_2_TINT) :
			(button == fButton ? B_DARKEN_1_TINT : B_LIGHTEN_MAX_TINT));
	
	rgb_color bottomColor = tint_color(color,
		button == kSpinButtonNone ?
			(IsEnabled() ? B_DARKEN_4_TINT : B_DARKEN_3_TINT) :
			(button == fButton ? B_LIGHTEN_2_TINT : B_DARKEN_1_TINT));
	
	BeginLineArray(4);
	
	AddLine(rect.LeftBottom(), rect.LeftTop(), topColor);
	AddLine(rect.LeftTop(), rect.RightTop(), topColor);
	
	AddLine(rect.RightTop(), rect.RightBottom(), bottomColor);
	AddLine(rect.RightBottom(), rect.LeftBottom(), bottomColor);
	
	EndLineArray();
	
	if (button != kSpinButtonNone) {
		rect.InsetBy(1, 1);
		SetHighColor(tint_color(color, B_LIGHTEN_1_TINT));
		FillRect(rect);

		BPoint point(0.5f * (rect.left + rect.right) - 2.0f,
			0.5f * (rect.top + rect.bottom) - 0.25f +
			(button == kSpinButtonUp ? 1.0f : -1.0f) +
			(button == fButton ? 1.0f : 0.0f));
		
		rgb_color arrowColor = tint_color(color,
			IsEnabled() ? B_DARKEN_4_TINT : B_DISABLED_LABEL_TINT);

		BeginLineArray(3);
		
		for (int32 delta = 0; delta < 3; delta++) {
			AddLine(point, point + BPoint(4 - delta - delta, 0), arrowColor);
			point += BPoint(1, button == kSpinButtonUp ? -1 : 1);
		}
		
		EndLineArray();
	}
}

BRect SpinButton::ButtonFrame(int32 button) const
{
	BRect rect = Bounds();
	
	rect.InsetBy(1, 1);
	float height = 0.5f * (rect.bottom - rect.top);
	
	if (button == kSpinButtonUp)
		return BRect(rect.left, rect.top, rect.right, rect.top + height - 1);
	
	if (button == kSpinButtonDown)	
		return BRect(rect.left, rect.top + height, rect.right, rect.bottom);
	
	return rect;
}

int32 SpinButton::ButtonAt(BPoint point) const
{
	if (ButtonFrame(kSpinButtonUp).Contains(point))
		return kSpinButtonUp;

	if (ButtonFrame(kSpinButtonDown).Contains(point))
		return kSpinButtonDown;
	
	return kSpinButtonNone;
}

void SpinButton::UpdateValue(int32 value)
{
	if (value != Value()) {
		SetValue(value);
		Invoke();
	}
	if (fButton != kSpinButtonNone && fButton != kSpinButtonWheel) {
		Invalidate(ButtonFrame(fButton));
	}
}

void SpinButton::KeyDown(const char *bytes, int32 numBytes)
{
	if (numBytes == 1) {
		if (bytes[0] == B_UP_ARROW) {
			UpdateValue(Value() + 1);
		}
		else if (bytes[0] == B_DOWN_ARROW) {
			UpdateValue(Value() - 1);
		}
		if (bytes[0] == B_PAGE_UP) {
			UpdateValue(Value() + fStepValue);
		}
		else if (bytes[0] == B_PAGE_DOWN) {
			UpdateValue(Value() - fStepValue);
		}
		else if (bytes[0] == B_HOME) {
			UpdateValue(fMaxValue);
		}
		else if (bytes[0] == B_END) {
			UpdateValue(fMinValue);
		}
		else if (bytes[0] == B_ENTER || bytes[0] == B_SPACE) {
			UpdateValue(fDefaultValue);
		}
		else {
			BControl::KeyDown(bytes, numBytes);
		}
	}
	else {
		BControl::KeyDown(bytes, numBytes);
	}
}

void SpinButton::MouseDown(BPoint point)
{
	if (IsEnabled()) {
		fButton = ButtonAt(point);
		if (fButton != kSpinButtonNone) {
			SetMouseEventMask(B_POINTER_EVENTS,
				B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS);

			if (fButton == kSpinButtonUp)
				UpdateValue(Value() + 1);

			if (fButton == kSpinButtonDown)
				UpdateValue(Value() - 1);
		}
		fPoint = point;
	}
	BControl::MouseDown(point);
}

void SpinButton::MouseUp(BPoint point)
{
	if (IsEnabled()) {
		if (fButton != kSpinButtonNone) {
			UpdateValue(Value());
			fButton = kSpinButtonNone;
		}
#if B_BEOS_VERSION >= 0x0500
		be_app->SetCursor(B_CURSOR_SYSTEM_DEFAULT);
#else
		be_app->SetCursor(B_HAND_CURSOR);
#endif
	}
	BControl::MouseUp(point);
}

void SpinButton::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	if (fButton != kSpinButtonNone) {
		int32 delta = int32(point.y - fPoint.y);
		if (delta <= -kSpinButtonThreshold || delta >= kSpinButtonThreshold) {
			UpdateValue(Value() + (delta < 0 ? fStepValue : -fStepValue));
			fPoint = point;
			fButton = kSpinButtonWheel;
			be_app->SetCursor(kVerticalResizeCursor);
		}
	}
	BControl::MouseMoved(point, transit, message);
}

void SpinButton::MessageReceived(BMessage *message)
{
#if B_BEOS_VERSION >= 0x0500
	if (message->what == B_MOUSE_WHEEL_CHANGED) {
		float delta = message->FindFloat("be:wheel_delta_y");
		UpdateValue(Value() + (delta < 0 ? fStepValue : -fStepValue));
	}
	else {
		BControl::MessageReceived(message);
	}
#else
	BControl::MessageReceived(message);
#endif
}

