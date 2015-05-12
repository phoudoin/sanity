/*!
 *     @header ToolTip
 *
 *   @abstract A Window that displays tips for the views in a window.
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

#ifndef _TOOL_TIP_H
#define _TOOL_TIP_H

#include <Message.h>
#include <Handler.h>
#include <Window.h>
#include <View.h>
#include <StringView.h>
#include <List.h>

class ToolTip : public BWindow {
	public:
		ToolTip(BWindow *window, const char *name);
		
		virtual ~ToolTip();

		const char *Text(BView *view) const;
		
		void SetText(BView *view, const char *text);

		virtual void DispatchMessage(BMessage *message, BHandler *target);

	private:
		BWindow *fWindow;
		BStringView *fView;
		BPoint fWhere;
		BList fTips;
};

#endif
