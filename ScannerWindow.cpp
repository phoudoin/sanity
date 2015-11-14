
#include <app/Application.h>
#include <interface/Window.h>
#include <interface/Box.h>
#include <interface/ScrollView.h>
#include <interface/MenuBar.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>
#include <interface/PopUpMenu.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <LayoutBuilder.h>

#include "BeSANE.h"

#include "Sanity.h"
#include "SanityStrings.h"
#include "ScannerWindow.h"
#include "ControlsWindow.h"
#include "PreviewView.h"
#include "TranslatorSavePanel.h"
#include "StackView.h"
#include "CollapsableBox.h"
#include "ScannerOptionView.h"
#include "ScannerInfoView.h"

//#define HAVE_PRESET_MENUFIELD

// --------------------------------------------------------------
/*ScannerWindow::ScannerWindow(BRect frame, BBitmap **outBitmap)
	: BWindow(frame, SOFTWARE_NAME, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	BMenuBar *		menu_bar;
	BMenu *			menu;
	BMenuItem *		item;
	BMenuField *	menu_field;
	BRect			r;
	float w, h;
	
	
	m_image = NULL;
	m_save_panel = NULL;
	
	m_translator_out_bitmap = outBitmap;
	m_standalone = !outBitmap;
	
	r = Bounds();

	if (m_standalone) {
		menu_bar = new BMenuBar(r, "menu_bar");
		
		// File Menu
		menu = new BMenu(_T("File"));

		// File         
		// ========
		// New Window
		// Close
		// --------
		// Save as...
		// --------
		// About
		// Quit
		
		//item = new BMenuItem(_T("New Window"), new BMessage(Application::NEW_WINDOW_MSG), 'N');
		//item->SetTarget(be_app);
		//menu->AddItem(item);

		//menu->AddItem(new BMenuItem(_T("Close"), new BMessage(B_QUIT_REQUESTED), 'W'));

		//menu->AddSeparatorItem();

		menu->AddItem(new BMenuItem(_T("Save As" B_UTF8_ELLIPSIS), new BMessage(SAVE_AS_MSG)));
		
		menu->AddSeparatorItem();

		item = new BMenuItem(_T("About"), new BMessage(B_ABOUT_REQUESTED));
		item->SetTarget(be_app);
		menu->AddItem(item);
		
		menu->AddSeparatorItem();

		item = new BMenuItem(_T("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
		item->SetTarget(be_app);
		menu->AddItem(item);

		menu_bar->AddItem(menu);

		// Okay, add our menu bar
		AddChild(menu_bar);
		

		r.top	+= menu_bar->Bounds().Height() + 1;
	}
	r.right	= 280;

	// Tooltips
	m_tooltip = new ToolTip(this, "controls_tooltip");
	
	// Add a scan panel
	m_panel = new BBox(r, NULL, B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
						B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
						B_PLAIN_BORDER);

	AddChild(m_panel);
	
	r = m_panel->Bounds();
	r.InsetBy(4, 4);
	m_options_rect = m_panel->Bounds();
#ifdef HAVE_PRESET_MENUFIELD
	m_preset_menu = new BPopUpMenu(_T("<none available>"));
	m_preset_menu->SetRadioMode(true);
	menu_field = new BMenuField(r, "preset_menu", PRESET_LABEL, m_preset_menu,
									B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
									B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	menu_field->SetDivider(be_plain_font->StringWidth(PRESET_LABEL "#"));

	m_panel->AddChild(menu_field);
	menu_field->ResizeToPreferred();

	m_tooltip->SetText(menu_field, _T("Load options values preset or save current options values as new preset."));
	
	m_options_rect.top = menu_field->Frame().bottom + 5;
#else
	m_options_rect.top = 82;
#endif

	r = Bounds();
	if (m_standalone)
		r.top	+= menu_bar->Bounds().Height() + 1;
	r.left	= m_options_rect.right + 1;

	BBox *box = new BBox(r, NULL, B_FOLLOW_ALL,	B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
						B_PLAIN_BORDER);
	AddChild(box);

	r = box->Bounds();
	r.InsetBy(20, 20);
	//r.top += 56;
	m_preview_view = new PreviewView(r, BRect(0, 0, 2100, 2970));
	box->AddChild(m_preview_view);

#if 0
	r = Bounds();
	r.top += menu_bar->Bounds().Height() + 1;
	CSplitterView *splitter = new CSplitterView(r, "splitter", m_panel, sv,
									B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);
	AddChild(splitter);
#endif

	m_device		= NULL;
	m_device_info	= NULL;
	
	m_devices_roster_thread_id 	= -1;
	m_scan_thread_id			= -1;
	
	//r = box->Bounds();
	r = m_panel->Bounds();
	
	r.InsetBy(4, 4);
	
	m_devices_menu = new BPopUpMenu(_T("<none available>"));
	m_devices_menu->SetRadioMode(true);
	menu_field = new BMenuField(r, "device_menu", _T( DEVICE_LABEL ), m_devices_menu,
									B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
									B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	menu_field->SetDivider(be_plain_font->StringWidth(_T(DEVICE_LABEL)) + 5.0);

	//box->AddChild(menu_field);
	m_panel->AddChild(menu_field);
	menu_field->GetPreferredSize(&w, &h);
	//menu_field->ResizeToPreferred();

	m_tooltip->SetText(menu_field, _T("Please select a device."));
	
	r.top += h + 1;
	float progress_top = r.top;
	r.top += 12;
	
	m_accept_button = NULL;
	if (!m_standalone) {
		m_accept_button = new BButton(r, NULL, _T("Accept"), new BMessage(ACCEPT_MSG),
									B_FOLLOW_RIGHT | B_FOLLOW_TOP,
									B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
		//m_accept_button->MakeDefault(true);
		m_panel->AddChild(m_accept_button);
		m_accept_button->GetPreferredSize(&w, &h);
		m_accept_button->ResizeToPreferred();
		m_accept_button->MoveTo(r.right - w, r.top);
		m_accept_button->SetEnabled(false);
		
		m_tooltip->SetText(m_accept_button, _T("Accept the current picture."));
		r.top += h + 2;
		m_options_rect.top += 26;
	}

	m_scan_button = new BButton(r, NULL, _T(SCAN_LABEL), new BMessage(SCAN_MSG),
								B_FOLLOW_RIGHT | B_FOLLOW_TOP,
								B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_scan_button->MakeDefault(true);
	//box->AddChild(m_scan_button);
	m_panel->AddChild(m_scan_button);
	m_scan_button->GetPreferredSize(&w, &h);
	m_scan_button->ResizeToPreferred();
	r.right -= w;
	m_scan_button->MoveTo(r.right, r.top);
	
	m_tooltip->SetText(m_scan_button, _T("Start the scan process."));


	r.right -= 8;

	m_status_bar = new BStatusBar(r, NULL, _T("Scanning..."), "");
	//box->AddChild(m_status_bar);
	m_panel->AddChild(m_status_bar);
	m_status_bar->SetResizingMode(B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	m_status_bar->SetFlags(m_status_bar->Flags() | (B_WILL_DRAW | B_FRAME_EVENTS));
	m_status_bar->Hide();

	m_status_bar->GetPreferredSize(&w, &h);
	m_status_bar->ResizeTo(r.Width(), h);

	m_options_stack = NULL;
	m_options_scroller = NULL;

	RescanDevices();
}*/

ScannerWindow::ScannerWindow(BRect frame, BBitmap **outBitmap)
	: BWindow(frame, SOFTWARE_NAME, B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
			B_AUTO_UPDATE_SIZE_LIMITS | B_ASYNCHRONOUS_CONTROLS)
{
	BMenuBar *		menu_bar;
	BMenu *			menu;
	BMenuItem *		item;
	BMenuField *	menu_field;
	
	m_image = NULL;
	m_save_panel = NULL;
	
	m_translator_out_bitmap = outBitmap;
	m_standalone = !outBitmap; /* act as an app */
		
	menu_bar = new BMenuBar("menu_bar");
		
	menu = new BMenu(_T("File"));
	
	menu->AddItem(new BMenuItem(_T("Save As" B_UTF8_ELLIPSIS), new BMessage(SAVE_AS_MSG)));

	menu->AddSeparatorItem();

	item = new BMenuItem(_T("About"), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem(_T("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu_bar->AddItem(menu);
	
	if (!m_standalone)
		menu_bar->Hide();
	
	// Add a scan panel
	m_panel = new BBox("panel", B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_FANCY_BORDER);
	//m_panel->SetExplicitSize(BSize(192, B_SIZE_UNSET));

	m_preview_view = new PreviewView(BRect(0, 0, 2100, 2970));
	//m_panel->SetExplicitMinSize(BSize(256, B_SIZE_UNSET));

	m_device		= NULL;
	m_device_info	= NULL;
	
	m_devices_roster_thread_id 	= -1;
	m_scan_thread_id			= -1;
	
	m_devices_menu = new BPopUpMenu(_T("<none available>"));
	m_devices_menu->SetRadioMode(true);
	menu_field = new BMenuField("device_menu", _T( DEVICE_LABEL ), m_devices_menu,
				B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);

	menu_field->SetToolTip(_T("Please select a device."));

	m_accept_button = new BButton("Accept", _T("Accept"), new BMessage(ACCEPT_MSG),
				B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_accept_button->SetEnabled(true);
	m_accept_button->SetToolTip(_T("Accept the current picture."));
	m_accept_button->Hide();

	m_scan_button = new BButton("Scan", _T(SCAN_LABEL), new BMessage(SCAN_MSG),								
								B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_scan_button->MakeDefault(true);
	
	m_scan_button->SetToolTip(_T("Start the scan process."));

	m_status_bar = new BStatusBar("StatusBar");
	m_status_bar->SetFlags(m_status_bar->Flags() | (B_WILL_DRAW | B_FRAME_EVENTS));	
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(0, 0, 0, 0)
		.Add(menu_bar)
		.AddSplit(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL)		
			.AddGroup(B_VERTICAL)
				.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
				.AddGroup(B_HORIZONTAL)
					.Add(menu_field)
					.Add(m_scan_button)
				.End()
				.Add(m_status_bar)
				.Add(m_accept_button)
				.Add(m_panel)
			.End()
			.AddGroup(B_VERTICAL)
				.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
				.Add(m_preview_view)
			.End()
		.End();

	m_options_stack = NULL;
	m_options_scroller = NULL;
	
	RescanDevices();
}



// --------------------------------------------------------------
ScannerWindow::~ScannerWindow()
{
	status_t ret;
	if ( DevicesRosterThreadID() >= 0 )
		wait_for_thread(DevicesRosterThreadID(), &ret);
	if ( m_device )
		sane_close(m_device);
		
	// wait_thread(m_devices_roster_thread_id);

	delete m_image;
	delete m_save_panel;
}


// --------------------------------------------------------------
bool ScannerWindow::QuitRequested()
{
	be_app->PostMessage(Application::WINDOW_CLOSED_MSG);
	return true;
}


// --------------------------------------------------------------
void ScannerWindow::MessageReceived
	(
	BMessage *	msg
	) 
{
	switch (msg->what)
		{
		case SAVE_AS_MSG: {
			if ( ! m_image ) {
				BAlert *alert = new BAlert(NULL, _T("No image to save."), _T("Doh!"),
									NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->Go();
				break;
			};
		
			if ( ! m_save_panel )				
				m_save_panel = new TranslatorSavePanel(_T("TranslatorSavePanel"),
											new BMessenger(this), NULL, 0, false,
											new BMessage(SAVE_FILE_PANEL_MSG)); 
	

			m_save_panel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
			m_save_panel->Show();
			break;
		};
			
		case SAVE_FILE_PANEL_MSG: {
			// Recover the necessary data from the message
			translator_id * id;
			uint32 			format;
			ssize_t 		length = sizeof(translator_id);

			if ( msg->FindData("translator_id", B_RAW_TYPE, (const void **)&id, &length) != B_OK)
				break;
			if ( msg->FindInt32("translator_format", (int32 *)&format) != B_OK)
				break;

			entry_ref dir;
			if ( msg->FindRef("directory", &dir) != B_OK)
				break;

			BDirectory bdir(&dir);
			const char *name;

			if ( msg->FindString("name", &name) != B_OK)
				break;
			if ( name == NULL )
				break;
	
			// Clobber any existing file or create a new one if it didn't exist
			BFile file(&bdir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
			if (file.InitCheck() != B_OK) {
				BAlert *alert = new BAlert(NULL, _T("Could not create file."), _T("OK"));
				alert->Go();
				break;
			};
	
			BTranslatorRoster * roster = BTranslatorRoster::Default();
			BBitmapStream stream(m_image);
	
			// If the id is no longer valid or the translator fails for any other
			// reason, catch it here
			if ( roster->Translate(*id, &stream, NULL, &file, format) != B_OK) {
				BAlert * alert = new BAlert(NULL, _T("Could not save the image."), _T("OK"));
				alert->Go();
			};
	
			// Reclaim the ownership of the bitmap, otherwise it would be deleted
			// when stream passes out of scope
			stream.DetachBitmap(&m_image);
			break;
		};
			
		case SET_DEVICE_BY_NAME_MSG:
			{
			ssize_t 			data_size;
			const SANE_Device *	device_info = NULL;
			BMenuItem *item;
			BMessage *itemMsg;
			BMessage setDevMsg(SET_DEVICE_MSG);
			BString dev;
			int i;
			if ( msg->FindString("device", &dev) != B_OK )
				break;
			msg->PrintToStream();
			for (i = 0; m_devices_menu->ItemAt(i); i++) {
				printf("checking dev %d\n", i);
				item = m_devices_menu->ItemAt(i);
				itemMsg = item->Message();
				itemMsg->PrintToStream();
				if ( !itemMsg )
					continue;
				if ( itemMsg->FindPointer("device", (void **) &device_info) != B_OK )
					continue;
				printf("CMP: %s:%s\n", dev.String(), device_info->name);
				if ( dev == device_info->name ) {
					item->SetMarked(true);
					setDevMsg.AddPointer("device", device_info);
					BMessenger msgr(this);
					msgr.SendMessage(&setDevMsg);
					break;
				}
			}
			
			break;
			}
		case SET_DEVICE_MSG:
			SetDevice(msg);
			break;
			
		case SCAN_MSG: {
			if ( ! m_device )
				break;
				
			if ( m_scan_thread_id != -1 ) {
				// already launched...
				m_cancel_scan = true;
				break;
			};
			
			m_scan_thread_id = spawn_thread(_ScanThread, "scan", B_NORMAL_PRIORITY, this);
			resume_thread(m_scan_thread_id);
			break;
		};	
		
		case FORMAT_CHANGED_MSG: {
			SANE_Status			status;
			SANE_Parameters 	parm;

			if (!m_device)
				break;

		    status = sane_get_parameters(m_device, &parm);
		    if (status == SANE_STATUS_GOOD) {
				printf(	"format:          ");
				switch (parm.format) {
				case SANE_FRAME_GRAY:	printf("SANE_FRAME_GRAY\n");	break;
				case SANE_FRAME_RGB:	printf("SANE_FRAME_RGB\n"); 	break;
				case SANE_FRAME_RED: 	printf("SANE_FRAME_RED\n");		break;
				case SANE_FRAME_GREEN: 	printf("SANE_FRAME_RED\n");		break;
				case SANE_FRAME_BLUE: 	printf("SANE_FRAME_RED\n");		break;
				default:
					printf("unknown (%d) frame format!\n", parm.format);
				};

				printf(	"last_frame:      %d\n"
						"bytes_per_line:  %d\n"
						"pixels_per_line: %d\n"
						"lines:           %d\n"
						"depth:           %d\n",
					parm.last_frame, parm.bytes_per_line, parm.pixels_per_line,
					parm.lines, parm.depth);

				BRect r(0, 0, parm.pixels_per_line, parm.lines);
				m_preview_view->SetGeometry(r);
		    } else
			 	printf("sane_get_parameters: %s\n", sane_strstatus (status));

			break;
		};

		case UPDATED_IMAGE_MSG: {
			BRect	r;
			
			if ( msg->FindRect("updated_rect", &r) != B_OK )
				break;
				
			// printf("UPDATED_IMAGE_MSG [%f, %f, %f, %f]\n", r.left, r.top, r.right, r.bottom);
			m_preview_view->Invalidate();
			break;
		};
			
		case ACCEPT_MSG:
			{
			if ( ! m_image )
				{
				BAlert *alert = new BAlert(NULL, _T("No image to save."), _T("Doh!"),
									NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->Go();
				break;
				};
		
			if ( m_translator_out_bitmap ) {
				*m_translator_out_bitmap = new BBitmap(m_image);
			}
			
			Quit();

			break;
			};

		default:	
			inherited::MessageReceived(msg);
	}
}


// #pragma mark -


// --------------------------------------------------------------
status_t ScannerWindow::SetImage
	(
	BBitmap * bitmap
	)
{
	if (m_image == bitmap)
		return B_OK;

	if ( m_image)
		delete m_image;
		
	m_image = bitmap;
	m_preview_view->SetBitmap(m_image);
	return B_OK;
}

		
// --------------------------------------------------------------
status_t ScannerWindow::SetDevice(BMessage *msg)
{
	SANE_Status	status;
	const SANE_Device *	device_info;
	SANE_Handle device;
			
	if ( msg->FindPointer("device", (void **) &device_info) != B_OK )
		return B_BAD_VALUE;
	PRINT(("Sanity:SetDevice(%s)\n", device_info->name));
	// Try to
	status = sane_open(device_info->name, &device);
	if ( status != SANE_STATUS_GOOD )	{
	 	fprintf (stderr, "sane_open(%s): %s\n", device_info->name, sane_strstatus (status));
		BAlert * alert = new BAlert("sane_open", sane_strstatus(status), _T("Argh"));
		alert->Go();
		return B_ERROR;
	};

	if ( m_device )
		// Close previous device opened, if any
		sane_close(m_device);
	m_device = device;
	m_device_info = device_info;

	return BuildControls();
}


// --------------------------------------------------------------
status_t ScannerWindow::BuildControls()
{
	// Build options panel
	SANE_Status	status;
	const SANE_Option_Descriptor *	desc;
	BRect r;
	PRINT(("Sanity:BuildControls()\n"));

	if ( m_options_stack )
		m_options_stack->RemoveSelf();
	if (m_options_scroller)
		m_options_scroller->RemoveSelf();
				
	r = m_panel->Bounds();
	r.right	-= B_V_SCROLL_BAR_WIDTH + 32;

	delete m_options_stack;
	delete m_options_scroller;
	m_options_stack		= new StackView(r, "options_lv", B_FOLLOW_ALL_SIDES);
	m_options_scroller 	= new BScrollView("options_sv", m_options_stack,
										B_FOLLOW_ALL_SIDES,
										B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
										false, true,	// vertical scrollbar
										B_PLAIN_BORDER);	// B_FANCY_BORDER);
	m_panel->AddChild(m_options_scroller);

	// printf("Options for device %s:\n", m_device_info->name);
	r = m_options_stack->Bounds();

	CollapsableBox *box = NULL;
	float box_height = 0;
	float w;
	BRect ovr = r;
	int nb_advanced_opts = 0;

	// skip first option (option 0 = number of options)
	for (int opt = 1; (desc = sane_get_option_descriptor(m_device, opt)) != NULL; opt++) {

		if (desc->cap & SANE_CAP_AUTOMATIC)   {
		    status = sane_control_option(m_device, opt, SANE_ACTION_SET_AUTO, 0, 0);
	      	if (status != SANE_STATUS_GOOD)
				printf("failed to set option --%s to automatic (%s)\n",	desc->name, sane_strstatus (status));
			};

		if (desc->type == SANE_TYPE_GROUP) {
			if (box) {
				box->ResizeTo(r.Width(), box_height + 2);
				RemoveChild(box);
				box->Show();
				m_options_stack->AddChild(box);

				if (nb_advanced_opts > (box->CountChildren() / 2))
					// 50% of advanced options in this group: collapse it by default.
					box->SetValue(B_CONTROL_OFF);
			};

			// new box
			box = new CollapsableBox(r, desc->name, desc->title, new BMessage(StackView::RESTACK_MSG), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
			box->SetTarget(m_options_stack);
			box->GetPreferredSize(&w, &box_height);
			box->SetFont(be_bold_font);
			box->Hide();

			AddChild(box);
					
			ovr = box->Bounds();
			ovr.InsetBy(6, 0);
					
			nb_advanced_opts = 0;
			continue;
		};	

		// All options kind except "group" ones
		if (!box) {
			// No options group box created so far
			// Create a default one, non collapsable (we don't have a group name!), to host them
			box = new CollapsableBox(r, NULL, _T("Main options"), new BMessage(StackView::RESTACK_MSG), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT); 
			box->SetTarget(m_options_stack);
			box->GetPreferredSize(&w, &box_height);
			box->SetFont(be_bold_font);
			box->Hide();

			AddChild(box);

			box_height = 2;
			ovr = box->Bounds();
			ovr.InsetBy(6, 0);

			nb_advanced_opts = 0;
		};
					
		ScannerOptionView * ov = new ScannerOptionView(ovr, desc->name, B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT, 0,
															m_device, opt);
		ov->Hide();
		AddChild(ov);
					
		if ( ov->Build() != B_OK ) {
			ov->RemoveSelf();
			delete ov;
			continue;
		};

		RemoveChild(ov);
		ov->Show();
		box->AddChild(ov);
		ov->MoveTo(6, box_height);
		box_height = ov->Frame().bottom;
							
		if (desc->cap & SANE_CAP_ADVANCED)
			nb_advanced_opts++;
						
		if (strlen(desc->desc))
			ov->SetToolTip(desc->desc);

	} 	// for every scanner options
	
	// add last group box
	if (box) {
		box->ResizeTo(r.Width(), box_height + 2);
		RemoveChild(box);
		box->Show();
		m_options_stack->AddChild(box);

		if (nb_advanced_opts > (box->CountChildren() / 2))
			// 50% of advanced options in this group: collapse it by default.
			box->SetValue(B_CONTROL_OFF);
	};

	// Add a pseudo control box, displaying some info about device/backends, whatever...
	AddDeviceInfoBox();

	// Modify window title
	BString title = "";
/*
	if ( strlen(m_device_info->model) )
		title << m_device_info->model << " - ";
	else
*/
	if ( strlen(m_device_info->name) )
		title << m_device_info->name << " - ";
	title << SOFTWARE_NAME;

	SetTitle(title.String());
				
	// m_options_scroller->MakeFocus();
	m_options_stack->MakeFocus();
	return B_OK;
}


// --------------------------------------------------------------
int32 ScannerWindow::ScanThread()
{
	SANE_Status			status;
	int 				len;
	SANE_Parameters 	parm;
	BBitmap *			image = NULL;
	bool				first_frame;

	if (!m_device)
		return B_ERROR;

	Lock();

	m_status_bar->Show();
	rgb_color default_color = m_status_bar->BarColor();
	m_scan_button->SetLabel(_T( "Cancel"));
//	m_status_bar->Reset(_T("Scanning..."));
	if (!m_standalone && !m_accept_button->IsHidden()) {
		m_accept_button->Hide();
	}

	Unlock();
	
	first_frame = true;
	
	m_cancel_scan = false;
	do	{ //  for each frame...
		// start frame reading
		if ( m_cancel_scan )
			break;
		
		status = sane_start(m_device);
		if ( status != SANE_STATUS_GOOD ) {
		 	fprintf (stderr, "sane_start: %s\n", sane_strstatus (status));
			break;
		};
	
		// get frame parameters
	    status = sane_get_parameters(m_device, &parm);
	    if (status != SANE_STATUS_GOOD)	{
		 	fprintf (stderr, "sane_get_parameters: %s\n", sane_strstatus (status));
			break;
		};
			
		if (parm.lines >= 0) {
			float total_bytes = parm.bytes_per_line * parm.lines;
			if (parm.format != SANE_FRAME_GRAY && parm.format != SANE_FRAME_RGB)
				total_bytes *= 3.0;
			
			Lock();
			m_status_bar->SetMaxValue(total_bytes);
			Unlock();
	
			fprintf (stderr, "scanning image of size %dx%d pixels at "
	 				"%d bits/pixel\n",
	 				parm.pixels_per_line, parm.lines,
	 				8 * parm.bytes_per_line / parm.pixels_per_line);
				
			if (first_frame) {
				image = new BBitmap(BRect(0, 0, parm.pixels_per_line - 1, parm.lines - 1),
						((parm.depth == 1) ? B_GRAY1 : B_RGBA32));
				
				if (! image) {
					BAlert * alert = new BAlert("BBitmap", _T("Failed to create image buffer"), _T("Glup."));
					alert->Go();
					break;
				};
				// TODO: fill image with transparent background (B_TRANSPARENT_COLOR or alpha = 0)
			};
			
		} else {
			fprintf (stderr, "scanning image %d pixels wide and "
					 "variable height at %d bits/pixel\n",
			 		parm.pixels_per_line, 8 * parm.bytes_per_line / parm.pixels_per_line);
		
			BAlert * alert = new BAlert("Well...", _T("Variable height scanning not supported (yet?)"), _T("Sorry"));
			alert->Go();
			break;
		};
		
		Lock();
		PostMessage(FORMAT_CHANGED_MSG);
		Unlock();
	
		uint8 * buffer, * data;
		uint8 * 	ptr;
		int32		x, y;
		int			line_len;
		int			padding_bytes;
		int			channel;
		rgb_color	red_color	= { 255, 0, 0, 255 };
		rgb_color	green_color	= { 0, 255, 0, 255 };
		rgb_color	blue_color	= { 0, 0, 255, 255 };
		BRect		updated_rect;
			
		x = y = 0;
		channel = 0;
		
		updated_rect.Set(0, 0, parm.pixels_per_line, 0);

		Lock();
		switch(parm.format) {
		case SANE_FRAME_RED:
			channel = 2;
			m_status_bar->SetBarColor(red_color);
			break;
		case SANE_FRAME_GREEN:
			channel = 1;
			m_status_bar->SetBarColor(green_color);
			break;
		case SANE_FRAME_BLUE:
			channel = 0;
			m_status_bar->SetBarColor(blue_color);
			break;
		default:
			channel = 0;
			m_status_bar->SetBarColor(default_color);
			break;
		};
		Unlock();								
			
		buffer = (uint8 *) malloc(parm.bytes_per_line);
	
		line_len = 0;
		padding_bytes = 0;
	
		while (1) { // read until end of frame or error
			if ( m_cancel_scan )
				break;
			
			len = 0;
			status = sane_read(m_device, buffer, parm.bytes_per_line, &len);
			if (status == SANE_STATUS_EOF)
				break;
				
			if (status != SANE_STATUS_GOOD) {
				fprintf (stderr, "sane_read: %s\n", sane_strstatus (status));

				BAlert * alert = new BAlert("sane_read()", sane_strstatus(status), _T("Glup."));
				alert->Go();
				break;
			};
					
			Lock();
			m_status_bar->Update((float) len);
			SetImage(image);
			Unlock();
	
			image->LockBits();
					
			ptr = (uint8 *) image->Bits();
			ptr += (y * image->BytesPerRow());
			ptr += (4*x);
	
			data = buffer;
			while ( len > 0 ) {
				if (padding_bytes) {
					// eat any padding bytes
					padding_bytes--;
					data++;
					len--;
					continue;
				};
					
		      	switch (parm.format) {
				case SANE_FRAME_RED:
				case SANE_FRAME_GREEN:
				case SANE_FRAME_BLUE: {
					uint8	value;
	
					if (parm.depth == 16) {
						value = (*((uint16 *) data)) >> 8;
						data += 2;
						len -= 2;
						line_len += 2;
					} else {
						value = *((uint8 *) data);
						data++;
						len--;
						line_len++;
					};
	
					*(ptr + channel) = value;
					*(ptr + 3)		= 255;	// Alpha channel
					break;
				};
							
				case SANE_FRAME_RGB: {
					uint8 red, green, blue;
							
					if (parm.depth == 16) {
						red = (*((uint16 *) data)) >> 8;
						data += 2;
						green = (*((uint16 *) data)) >> 8;
						data += 2;
						blue = (*((uint16 *) data)) >> 8;
						data += 2;
						len -= 6;
						line_len += 6;
					} else {
						red = *((uint8 *) data++);
						green = *((uint8 *) data++);
						blue = *((uint8 *) data++);
						len -= 3;
						line_len += 3;
					};
	
					*ptr 		= blue;
					*(ptr+1) 	= green;
					*(ptr+2) 	= red;	// red channel
					*(ptr+3) 	= 255;		// Alpha channel
					break;
				};
							
				case SANE_FRAME_GRAY: {
					uint8	value = 0;
							
					if (parm.depth == 1 ) {
						*ptr = *((uint8 *) data++);
						len--;
						line_len++;
						break;
					};
							
					if (parm.depth == 16) {
						value = (*((uint16 *) data)) >> 8;
						data += 2;
						len -= 2;
						line_len += 2;
					} else if ( parm.depth == 8 ) {
						value = *((uint8 *) data++);
						len--;
						line_len++;
					};
					*ptr 		= value;	// blue channel
					*(ptr+1) 	= value;	// green channel
					*(ptr+2) 	= value;	// red channel
					*(ptr+3) 	= 255;		// Alpha channel
					break;
				};
				};
						
				// Next pixel;		
				x	+= ((parm.depth == 1) ? 8 : 1);
				ptr += ((image->ColorSpace() == B_GRAY1) ? 1 : 4);

				if ( x >= parm.pixels_per_line ) {
					y++;
					if ( y >= parm.lines )
						break;
					x = 0;
						
					padding_bytes = parm.bytes_per_line - line_len;
					line_len = 0;

					// skip end of row padding bytes...
					ptr = (uint8 *) image->Bits();
					ptr += (y * image->BytesPerRow());
				};
			};
			image->UnlockBits();
	
			BMessage * msg = new BMessage(UPDATED_IMAGE_MSG);
			updated_rect.bottom = y;
			msg->AddRect("updated_rect", updated_rect);
	
			Lock();
			PostMessage(msg);
			Unlock();
				
			delete msg;
				
			updated_rect.top = y;
		};	// while sane_read()
				
		free(buffer);
		first_frame = false;

	} while ( ! parm.last_frame && status == SANE_STATUS_EOF );
	
	sane_cancel(m_device);
	
	Lock();
	m_scan_button->SetLabel(_T("Scan"));
	m_status_bar->Hide();
	m_status_bar->SetBarColor(default_color);

	if (!m_standalone && m_accept_button->IsHidden()) {
		m_accept_button->Show();
		m_accept_button->MakeDefault(true);
	}

	Unlock();
	
	m_scan_thread_id = -1;
	return B_OK;
}

void ScannerWindow::RescanDevices()
{
	status_t ret;
	
	if ( DevicesRosterThreadID() >= 0 )
		wait_for_thread(DevicesRosterThreadID(), &ret);	
	
	m_devices_roster_thread_id = spawn_thread(_DevicesRosterThread, "devices_roster", B_NORMAL_PRIORITY, this);
	resume_thread(m_devices_roster_thread_id);
}

// --------------------------------------------------------------
int32 ScannerWindow::DevicesRosterThread()
{
	BMenuItem *				item;
	SANE_Status				status;
	const SANE_Device ** 	devices_list;
	SANE_Handle				device;

	system("scanimage -L");

	while ( (item = m_devices_menu->RemoveItem((int32)0)) != NULL)
		delete item;

	status = sane_get_devices(&devices_list, SANE_FALSE);
    if (status == SANE_STATUS_GOOD)
		{
		int	i;
		for (i = 0; devices_list[i]; ++i)
			{
			BString 	label;
			BMessage * 	msg;

/*			
			if (strcmp(devices_list[i]->vendor, "Noname") != 0)
			  label << devices_list[i]->vendor << " ";
			if (devices_list[i]->model)
				label << devices_list[i]->model;
*/			if (devices_list[i]->name)
				label << devices_list[i]->name;

			msg = new BMessage(SET_DEVICE_MSG);
			msg->AddPointer("device", devices_list[i]); 
			
			status = sane_open(devices_list[i]->name, &device);
			switch (status)
				{
				case SANE_STATUS_GOOD:
					sane_close(device); break;
					
				case SANE_STATUS_DEVICE_BUSY:
					label << " " << _T("[busy]"); break;
					
				case SANE_STATUS_ACCESS_DENIED:
					label << " " << _T("[access denied]"); break;
					
				default:
					label << " " << _T("[error]"); break;
				};

			item = new BMenuItem(label.String(), msg);
				
			if ( status != SANE_STATUS_GOOD )
				item->SetEnabled(false);
				
			m_devices_menu->AddItem(item);
			item->SetMarked(true);
			PostMessage(msg);
			};
		};

	return B_OK;
}


// --------------------------------------------------------------
status_t ScannerWindow::AddDeviceInfoBox()
{
	CollapsableBox *box;
	BRect r, ivr;
	float box_height = 0;
	float w;
	
	if (! m_device_info || ! m_options_stack)
		return B_ERROR;
	
	r = m_options_stack->Bounds();
	box = new CollapsableBox(r, "device_info", _T("Device info"),
			new BMessage(StackView::RESTACK_MSG), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
	if (!box)
		return B_ERROR;
		
	box->GetPreferredSize(&w, &box_height);
	box->SetTarget(m_options_stack);
	box->SetFont(be_bold_font);
					
	box->Hide();
	AddChild(box);

	ivr = box->Bounds();
	ivr.InsetBy(6, 0);

	ScannerInfoView *iv = new ScannerInfoView(ivr, B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT, 0, m_device_info);
	box->AddChild(iv);
	iv->MoveTo(6, box_height);

	box_height = iv->Frame().bottom;
	box->ResizeTo(r.Width(), box_height + 2);
	RemoveChild(box);
	box->Show();
	
	m_options_stack->AddChild(box);

	return B_OK;
}
