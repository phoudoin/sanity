#include <Application.h>
#include <Box.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <Window.h>
#include <Catalog.h>

#include "Sanity.h"
#include "ScannerWindow.h"
#include "ControlsWindow.h"
#include "PreviewView.h"
#include "TranslatorSavePanel.h"
#include "StackView.h"
#include "CollapsableBox.h"
#include "ScannerOptionView.h"
#include "ScannerInfoView.h"
#include "SaneUtils.h"

#include <BeBuild.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ScannerWindow"

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
		
	menu = new BMenu(B_TRANSLATE("File"));
	
	menu->AddItem(new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS), new BMessage(SAVE_AS_MSG)));

	menu->AddSeparatorItem();

	item = new BMenuItem(B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu_bar->AddItem(menu);
	
	if (!m_standalone)
		menu_bar->Hide();
	
	// Add a scan panel
	m_panel = new BBox("panel", B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_PLAIN_BORDER);

	m_preview_view = new PreviewView(BRect(0, 0, 2100, 2970));

	m_device		= NULL;
	m_device_info	= NULL;
	
	m_devices_roster_thread_id 	= -1;
	m_scan_thread_id			= -1;
	
	m_devices_menu = new BPopUpMenu(B_TRANSLATE("<none available>"));
	m_devices_menu->SetRadioMode(true);
	menu_field = new BMenuField("device_menu", B_TRANSLATE( DEVICE_LABEL ), m_devices_menu,
				B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);

	menu_field->SetToolTip(B_TRANSLATE("Please select a device"));

	m_accept_button = new BButton("Accept", B_TRANSLATE("Accept"), new BMessage(ACCEPT_MSG),
				B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_accept_button->SetEnabled(true);
	m_accept_button->SetToolTip(B_TRANSLATE("Accept the current picture."));

	m_save_as_button = new BButton("Save As", B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
								new BMessage(SAVE_AS_MSG), B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_save_as_button->SetToolTip(B_TRANSLATE("Save scanned image as" B_UTF8_ELLIPSIS));

	if (m_standalone) {
		m_save_as_button->SetEnabled(false);
		m_accept_button->Hide();
	} else {
		m_accept_button->SetEnabled(false);
		m_save_as_button->Hide();
	}

	m_scan_button = new BButton("Scan", B_TRANSLATE(SCAN_LABEL), new BMessage(SCAN_MSG),
								B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_scan_button->MakeDefault(true);
	m_scan_button->SetToolTip(B_TRANSLATE("Start the scan process"));
	
	m_close_button = new BButton("Close", B_TRANSLATE("Close"), new BMessage(B_QUIT_REQUESTED),
								B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_close_button->SetToolTip(B_TRANSLATE("Close application"));

	m_status_bar = new BStatusBar("StatusBar");
	m_status_bar->SetFlags(m_status_bar->Flags() | (B_WILL_DRAW | B_FRAME_EVENTS));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(0, 0, 0, 0)
		.Add(menu_bar)
		.AddSplit(B_HORIZONTAL, 0)
			.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
				.SetInsets( B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING,
							B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
				.Add(menu_field)
				.Add(m_panel)
				.Add(m_status_bar)
			.End()
			.AddGroup(B_VERTICAL, 0)
				.SetInsets( B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING,
							B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
				.Add(m_preview_view)
				.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
					.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0)
					.SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_BOTTOM))
					.AddGlue()
					.Add(m_scan_button)
					.Add(m_accept_button)
					.Add(m_save_as_button)
					.Add(m_close_button)
					.AddGlue()
				.End()				
			.End()
		.End();

	m_options_stack = NULL;
	m_options_scroller = NULL;
	
	CenterOnScreen();
	
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
void ScannerWindow::MessageReceived (BMessage *	msg)
{
	switch (msg->what)
		{
		case SAVE_AS_MSG: {
			if ( ! m_image ) {
				BAlert *alert = new BAlert(NULL, B_TRANSLATE("No image to save."), B_TRANSLATE("Doh!"),
									NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->Go();
				break;
			};
		
			if ( ! m_save_panel )				
				m_save_panel = new TranslatorSavePanel(B_TRANSLATE("TranslatorSavePanel"),
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
				BAlert *alert = new BAlert(NULL, B_TRANSLATE("Could not create file."), B_TRANSLATE("OK"));
				alert->Go();
				break;
			};
	
			BTranslatorRoster * roster = BTranslatorRoster::Default();
			BBitmapStream stream(m_image);
	
			// If the id is no longer valid or the translator fails for any other
			// reason, catch it here
			if ( roster->Translate(*id, &stream, NULL, &file, format) != B_OK) {
				BAlert * alert = new BAlert(NULL, B_TRANSLATE("Could not save the image."), B_TRANSLATE("OK"));
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
			
		case SCAN_MSG: {
			if ( ! m_device )
				break;
				
			if ( m_scan_thread_id != -1 ) {
				// already launched...
				m_cancel_scan = true;
				break;
			};

			SetImage(NULL);
			m_preview_view->SetImageFrame();

			m_scan_thread_id = spawn_thread(_ScanThread, "scan", B_NORMAL_PRIORITY, this);
			resume_thread(m_scan_thread_id);
			break;
		};	
		
		case SET_DEVICE_MSG:
			SetDevice(msg);
			PostMessage(FORMAT_CHANGED_MSG);
			break;

		case FORMAT_CHANGED_MSG: {
			SANE_Status			status;
			SANE_Parameters 	parm;

			if (!m_device)
				break;

		    status = sane_get_parameters(m_device, &parm);
		    if (status == SANE_STATUS_GOOD) {
				switch (parm.format) {
				case SANE_FRAME_GRAY:	printf("SANE_FRAME_GRAY\n");	break;
				case SANE_FRAME_RGB:	printf("SANE_FRAME_RGB\n"); 	break;
				case SANE_FRAME_RED: 	printf("SANE_FRAME_RED\n");		break;
				case SANE_FRAME_GREEN: 	printf("SANE_FRAME_RED\n");		break;
				case SANE_FRAME_BLUE: 	printf("SANE_FRAME_RED\n");		break;
				default:
					printf("unknown (%d) frame format!\n", parm.format);
				};

				BRect geometry(0, 0, parm.pixels_per_line, parm.lines);
				BRect frame = geometry;

				if ( m_tl_x != NULL && m_tl_y != NULL && m_br_x != NULL && m_br_y != NULL) {
					geometry.Set(0, 0, GetSaneMaxVal(m_device, "br-x"), GetSaneMaxVal(m_device, "br-y"));
					frame.Set(GetSaneVal(m_device, "tl-x"), GetSaneVal(m_device, "tl-y"),
						GetSaneVal(m_device, "br-x"), GetSaneVal(m_device, "br-y"));
				}

				m_preview_view->SetGeometry(geometry);
				m_preview_view->SetFrame(frame);
		    } else
			 	printf("sane_get_parameters: %s\n", sane_strstatus (status));
			break;
		};

		case UPDATED_IMAGE_MSG: {
			BRect	r;

			if ( msg->FindRect("updated_rect", &r) != B_OK )
				break;

			m_preview_view->Invalidate();
			break;
		};
			
		case ACCEPT_MSG:
			{
			if ( ! m_image )
				{
				BAlert *alert = new BAlert(NULL, B_TRANSLATE("No image to save."), B_TRANSLATE("Doh!"),
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

		case PARAM_CHANGED_MSG:
			{
				if (!m_device)
					break;

				BRect rect;
				if ( msg->FindRect("rect", &rect) != B_OK )
					break;

				if (m_tl_x) {
					SetSaneVal(m_device, "tl-x", rect.left <= rect.right ? rect.left : rect.right);
					m_tl_x->UpdateValue();
				}
				if (m_tl_y) {
					SetSaneVal(m_device, "tl-y", rect.top <= rect.bottom ? rect.top : rect.bottom);
					m_tl_y->UpdateValue();
				}
				if (m_br_x) {
					SetSaneVal(m_device, "br-x", rect.right >= rect.left ? rect.right : rect.left);
					m_br_x->UpdateValue();
				}
				if (m_br_y) {
					SetSaneVal(m_device, "br-y", rect.bottom >=rect.top ? rect.bottom : rect.top);
					m_br_y->UpdateValue();
				}

				PostMessage(FORMAT_CHANGED_MSG);
				break;
			}
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
	printf("Sanity:SetDevice(%s)\n", device_info->name);
	// Try to
	status = sane_open(device_info->name, &device);
	if ( status != SANE_STATUS_GOOD )	{
	 	fprintf (stderr, "sane_open(%s): %s\n", device_info->name, sane_strstatus (status));
		BAlert * alert = new BAlert("sane_open", sane_strstatus(status), B_TRANSLATE("Argh"));
		alert->Go();
		return B_ERROR;
	};

	// Close previous device opened, if any
	if ( m_device )
		sane_close(m_device);

	m_device = device;
	m_device_info = device_info;
	
	SetImage(NULL);

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

	m_tl_x = NULL;
	m_tl_y = NULL;
	m_br_x = NULL;
	m_br_y = NULL;

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
		if ((desc->type == SANE_TYPE_FIXED || desc->type == SANE_TYPE_INT)
			&& desc->size == sizeof (SANE_Int)
			&& (desc->unit == SANE_UNIT_DPI)
			&& (strcmp (desc->name, SANE_NAME_SCAN_RESOLUTION) == 0)) {
				m_preview_resolution = 300;
				int cnt = desc->constraint.word_list[0];
				for (int i = 1; i <= cnt && desc->constraint.word_list[i]; i++) {
					if ( desc->constraint.word_list[i] < m_preview_resolution )
						m_preview_resolution = desc->constraint.word_list[i];
				}
		}

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
			box = new CollapsableBox(r, NULL, B_TRANSLATE("Main options"), new BMessage(StackView::RESTACK_MSG), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT);
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

		if (strcmp(desc->name, SANE_NAME_SCAN_TL_X)==0)m_tl_x = ov;
		if (strcmp(desc->name, SANE_NAME_SCAN_TL_Y)==0)m_tl_y = ov;
		if (strcmp(desc->name, SANE_NAME_SCAN_BR_X)==0)m_br_x = ov;
		if (strcmp(desc->name, SANE_NAME_SCAN_BR_Y)==0)m_br_y = ov;

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

	title << SOFTWARE_NAME << " : ";

	if (strcmp(m_device_info->vendor, "Noname") != 0)
		  title << m_device_info->vendor << " ";
	if ( strlen(m_device_info->model) )
		title << m_device_info->model;

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

	rgb_color default_color = m_status_bar->BarColor();
	m_scan_button->SetLabel(B_TRANSLATE( "Cancel"));
	m_status_bar->Reset();
	if (!m_standalone && m_accept_button->IsEnabled()) {
		m_accept_button->SetEnabled(false);
	}
	if (m_standalone && m_save_as_button->IsEnabled()) {
		m_save_as_button->SetEnabled(false);
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
					BAlert * alert = new BAlert("BBitmap", B_TRANSLATE("Failed to create image buffer"), B_TRANSLATE("Glup."));
					alert->Go();
					break;
				};
				// TODO: fill image with transparent background (B_TRANSPARENT_COLOR or alpha = 0)
			};
			
		} else {
			fprintf (stderr, "scanning image %d pixels wide and "
					 "variable height at %d bits/pixel\n",
			 		parm.pixels_per_line, 8 * parm.bytes_per_line / parm.pixels_per_line);
		
			BAlert * alert = new BAlert("Well...", B_TRANSLATE("Variable height scanning not supported (yet?)"), B_TRANSLATE("Sorry"));
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

				BAlert * alert = new BAlert("sane_read()", sane_strstatus(status), B_TRANSLATE("Glup."));
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
	m_scan_button->SetLabel(B_TRANSLATE("Scan"));
	m_status_bar->Reset();
	m_status_bar->SetBarColor(default_color);

	if (!m_standalone && !m_accept_button->IsEnabled()) {
		m_accept_button->SetEnabled(true);
		m_accept_button->MakeDefault(true);
	}
	if (m_standalone && !m_save_as_button->IsEnabled()) {
		m_save_as_button->SetEnabled(true);
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
	BMessage * 				msg;
	SANE_Status				status;
	const SANE_Device ** 	devices_list;
	SANE_Handle				device;

	// Ugly hack
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

			
			if (strcmp(devices_list[i]->vendor, "Noname") != 0)
			  label << devices_list[i]->vendor << " ";
			if (devices_list[i]->model)
				label << devices_list[i]->model;
//			if (devices_list[i]->name)
//				label << devices_list[i]->name;

			msg = new BMessage(SET_DEVICE_MSG);
			msg->AddPointer("device", devices_list[i]); 
			
			status = sane_open(devices_list[i]->name, &device);
			switch (status)
				{
				case SANE_STATUS_GOOD:
					sane_close(device); break;
					
				case SANE_STATUS_DEVICE_BUSY:
					label << " " << B_TRANSLATE("[busy]"); break;
					
				case SANE_STATUS_ACCESS_DENIED:
					label << " " << B_TRANSLATE("[access denied]"); break;
					
				default:
					label << " " << B_TRANSLATE("[error]"); break;
				};

			item = new BMenuItem(label.String(), msg);
				
			if ( status != SANE_STATUS_GOOD )
				item->SetEnabled(false);
				
			m_devices_menu->AddItem(item);
			};
			
			if (item && m_standalone) {
				item->SetMarked(true);
				PostMessage(msg);
			}
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
	box = new CollapsableBox(r, "device_info", B_TRANSLATE("Device info"),
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
