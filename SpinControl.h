/*!
 *     @header SpinControl
 *
 *   @abstract A TextControl for editing bounded numeric values.
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

#ifndef _SPIN_CONTROL_H
#define _SPIN_CONTROL_H

#include <Control.h>
#include <TextControl.h>
#include "SpinButton.h"

class SpinControl : public BControl {
	public:
		SpinControl(BRect frame, const char *name,
			const char *label, BMessage *message, int32 minValue,
			int32 maxValue, int32 defaultValue, int32 stepValue,
			uint32 resizingFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
		
		virtual ~SpinControl();

		void GetLimitValues(int32 *minValue,
			int32 *maxValue) const;
		
		void SetLimitValues(int32 minValue, int32 maxValue);
		
		float Divider() const;
		
		void SetDivider(float divider);
		
		virtual void SetLabel(const char *label);
		
		virtual void SetValue(int32 value);

		virtual void SetEnabled(bool enabled);
		
		virtual void AttachedToWindow();
		
		virtual void FrameResized(float width, float height);
		
		virtual void GetPreferredSize(float *width, float *height);
		
		virtual void MessageReceived(BMessage *message);
		
	private:
		BTextControl *fTextControl;
		SpinButton *fSpinButton;
};


#endif
