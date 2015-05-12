#ifndef SCANNERINFOVIEW_H
#define SCANNERINFOVIEW_H

#include <InterfaceKit.h>
#include <SupportKit.h>

#include "BeSANE.h"

class ScannerInfoView : public BView  
{
public:
		// Constructors, destructors, operators...
		ScannerInfoView(BRect frame, uint32 resizeMask, uint32 flags, const SANE_Device *device_info);
		~ScannerInfoView();
							
		typedef BView 	inherited;

	// Virtual function overrides
virtual void 			AttachedToWindow(void);
virtual void 			MouseDown(BPoint where);
virtual void 			Draw(BRect updateRect);

	// From here, it's none of your business! ;-)
private:
		void 			InitIcon();

		BBitmap *		m_icon;
};

#endif // ifdef SCANNERINFOVIEW_H

