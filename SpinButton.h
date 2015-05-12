/*!
 *     @header SpinButton
 *
 *   @abstract A Button that increases or decreases a numeric value.
 * @discussion
 *             Copyright &copy; 2000 Carlos Hasan
 *
 *             This software is provided 'as-is', without any express or implied
 *             warranty. In no event will the author be held liable for any
 *             damages arising from the use of this software. Permission is
 *             granted to anyone to use this software for any purpose, including
 *             commercial applications, and to alter it and redistribute it
 *             freely, subject to the following restrictions:
 *
 *             1. The origin of this software must not be misrepresented; you
 *             must not claim that you wrote the original software. If you use
 *             this software in a product, an acknowledgment in the product
 *             documentation would be appreciated but is not required.
 *
 *             2. Altered source versions must be plainly marked as such, and
 *             must not be misrepresented as being the original software.
 *
 *             3. This notice may not be removed or altered from any source
 *             distribution.
 */

#ifndef _SPIN_BUTTON_H
#define _SPIN_BUTTON_H

#include <Control.h>

class SpinButton : public BControl {
	public:
		SpinButton(BRect frame, const char *name,
			BMessage *message, int32 minValue, int32 maxValue,
			int32 defaultValue, int32 stepValue,
			uint32 resizingFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);
		
		virtual ~SpinButton();

		void GetLimitValues(int32 *minimum, int32 *maximum) const;
		
		void SetLimitValues(int32 minimum, int32 maximum);
		
		virtual void SetValue(int32 value);
		
		virtual void Draw(BRect updateRect);
		
		virtual void GetPreferredSize(float *width, float *height);
		
		virtual void KeyDown(const char *bytes, int32 numBytes);
		
		virtual void MouseDown(BPoint point);
		
		virtual void MouseUp(BPoint point);

		virtual void MouseMoved(BPoint point, uint32 transit,
						const BMessage *message);
		
		virtual void MessageReceived(BMessage *message);
		
	private:
		void DrawButton(BRect rect, int32 button);
		
		int32 ButtonAt(BPoint point) const;
		
		BRect ButtonFrame(int32 button) const;
		
		void UpdateValue(int32 value);

	private:
		int32 fMinValue;
		int32 fMaxValue;
		int32 fDefaultValue;
		int32 fStepValue;
		int32 fButton;
		BPoint fPoint;
};

#endif
