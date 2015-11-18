#include <Application.h>
#include <Roster.h>		// for app_info
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <Catalog.h>
#include <IconUtils.h>

#include <stdlib.h>
#include <sane/sane.h>

#include "Sanity.h"
#include "ScannerInfoView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ScannerInfoView"

// Constructor & destructor
// ------------------------

// --------------------------------------------------------------
ScannerInfoView::ScannerInfoView(BRect frame, uint32 resizeMask, uint32 flags, 	
			const SANE_Device *device_info)
	: BView(frame, "scanner_info_view", resizeMask, flags | B_FRAME_EVENTS)
{
	BString text;
	BRect r = Bounds();

	InitIcon();
	if (m_icon) {
		SetFlags(Flags() | B_WILL_DRAW);
		r.left += m_icon->Bounds().Width() + 2;
	}
	
	text = "";
	if (device_info) {
		if (device_info->name)
			text << B_TRANSLATE("Name: ") << device_info->name << "\n";
		if (device_info->vendor && strcmp("Noname", device_info->vendor) != 0)
			text << B_TRANSLATE("Vendor: ") << device_info->vendor << "\n";
		if (device_info->model)
			text << B_TRANSLATE("Model: ") << device_info->model << "\n";
		if (device_info->type)
			text << B_TRANSLATE("Type: ") << device_info->type << "\n";
	}

	BTextView *tv = new BTextView(r, "scanner_info_textview",  r, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	AddChild(tv);

	tv->SetText(text.String(), text.Length());
	tv->MakeEditable(false);
	tv->MakeSelectable(false);
	tv->SetWordWrap(true);
	
	// Now resize the box vertically so that all the text is visible
	float h = tv->TextHeight(0, tv->CountLines());
	tv->ResizeTo(r.Width(), h);
	r.OffsetTo(0, 0);
	r.bottom = h;
	tv->SetTextRect(r);

	if (m_icon && h < m_icon->Bounds().Height())
		// Take care of icon full visibility
		h = m_icon->Bounds().Height();
	
	ResizeTo(Bounds().Width(), h + 2);
}

// --------------------------------------------------------------
ScannerInfoView::~ScannerInfoView()
{
	delete m_icon;
}

//	#pragma mark -

// Public methods
// --------------

// --------------------------------------------------------------
void ScannerInfoView::AttachedToWindow()
{
	if ( Parent() )
		SetViewColor(Parent()->ViewColor());

	BTextView *tv = dynamic_cast<BTextView *>(FindView("scanner_info_textview"));
	if (tv)
		tv->SetViewColor(Parent()->ViewColor());

	inherited::AttachedToWindow();
}


// --------------------------------------------------------------
void ScannerInfoView::MouseDown(BPoint where)
{
	Window()->Activate();
	inherited::MouseDown(where);
}

// --------------------------------------------------------------
void ScannerInfoView::Draw(BRect updateRect)
{
	inherited::Draw(updateRect);
	
	if (!m_icon)
		return;

	PushState();
	SetDrawingMode(B_OP_OVER);
	DrawBitmapAsync(m_icon, BPoint(0, 0));
	PopState();
}

//------------------------------------------------------------------------------
void ScannerInfoView::InitIcon()
{
	char app_path[B_PATH_NAME_LENGTH+1];
	BBitmap* icon = NULL;

	if (GetBinaryPath(app_path, (void*)GetBinaryPath) < B_OK)
		return;

	BFile file(app_path, B_READ_ONLY);
	if (file.InitCheck() == B_OK) {
		BResources Resources;
		if (Resources.SetTo(&file) == B_OK) {
			// Try to load an icon resource named after the option name...
			size_t size;
			const void *data = Resources.LoadResource(B_VECTOR_ICON_TYPE, "device_info", &size);
			if (data) {
				icon = new BBitmap(BRect(0, 0, 31, 31), B_RGBA32);
				if (icon->InitCheck() == B_OK)
					BIconUtils::GetVectorIcon((const uint8 *)data, size, icon);
			}
		}
	}
	m_icon = icon;
}



