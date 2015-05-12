
#include <app/Application.h>
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>

#include "Sanity.h"

#include "MainWindow.h"
#include "ControlsWindow.h"
#include "ScannerOptionView.h"

#include "ToolTip.h"
#include "SpinControl.h"

#include "BeSANE.h"

static const char * get_unit2 (SANE_Unit unit);


#ifdef CODEWARRIOR
	#pragma mark [Constructor & destructor]
#endif

// Constructor & destructor
// ------------------------

// --------------------------------------------------------------
ControlsWindow::ControlsWindow
	(
	MainWindow *	parent_window
	)
: BWindow(BRect(50, 50, 300, 500), "Controls",
		  B_TITLED_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
		  B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_WILL_ACCEPT_FIRST_CLICK | B_ASYNCHRONOUS_CONTROLS)
{
	m_device		= NULL;
	m_device_info	= NULL;
	
	m_parent_window 			= parent_window;
	m_devices_roster_thread_id 	= -1;
	m_scan_thread_id			= -1;
	
	m_tooltip = new ToolTip(this, "controls_tooltip");

	BRect r;
	float w, h;
	
	r = Bounds();
	
	// Add a scan panel
	m_panel	= new BBox(r, NULL, B_FOLLOW_ALL,
						B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
						B_PLAIN_BORDER);
	
	r.OffsetBy(4, 4);
	
	// First child view : Device PopUpMenu

	#define DEVICE_LABEL	"Device:"

	m_devices_menu = new BPopUpMenu("<none available>");
	m_devices_menu->SetRadioMode(true);
	m_devices_field = new BMenuField(r, "device_menu", DEVICE_LABEL, m_devices_menu,
									B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
									B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_devices_field->SetDivider(be_plain_font->StringWidth(DEVICE_LABEL "##"));

	m_panel->AddChild(m_devices_field);
	m_tooltip->SetText(m_devices_field, "Please select a device.");
	
	m_devices_field->ResizeToPreferred();
	m_devices_field->GetPreferredSize(&w, &h);

	m_scan_button = new BButton(r, NULL, "Scan", new BMessage(SCAN_MSG),
								B_FOLLOW_RIGHT | B_FOLLOW_TOP,
								B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	m_scan_button->MakeDefault(true);

	m_scan_button->ResizeToPreferred();
	m_scan_button->GetPreferredSize(&w, &h);
	r.right -= (w + 12);
	m_scan_button->MoveTo(r.right, 40);
	
	m_panel->AddChild(m_scan_button);
	m_tooltip->SetText(m_scan_button, "Start the scan process.");

	r.right -= 8;
	r.left 	+= 8;
	r.top 	+= 40;

	m_status_bar = new BStatusBar(r, NULL, "Scanning...", "1.58 Mb");
	// m_status_bar->ResizeToPreferred();
	m_status_bar->SetResizingMode(B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	m_status_bar->SetFlags(m_status_bar->Flags() | (B_WILL_DRAW | B_FRAME_EVENTS));
	m_status_bar->Hide();

	m_panel->AddChild(m_status_bar);

	r = m_panel->Bounds();
	r.top		= 80;
	r.right 	-= B_V_SCROLL_BAR_WIDTH;
	r.InsetBy(4,4);

	// m_options_lv	= new BListView(r, "options_lv", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES );
	// m_options_sv 	= new BScrollView("options_sv", m_options_lv,
	//									B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS, false, true);
	// view->AddChild(m_options_sv);

	AddChild(m_panel);
	
	AddToSubset(parent_window);

	m_devices_roster_thread_id = spawn_thread(_DevicesRosterThread, "devices_roster", B_NORMAL_PRIORITY, this);
	resume_thread(m_devices_roster_thread_id);
}


// --------------------------------------------------------------
ControlsWindow::~ControlsWindow()
{
	if ( m_device )
		sane_close(m_device);
}

#ifdef CODEWARRIOR
	#pragma mark [Public methods]
#endif

// Public methods
// --------------


static const char * get_unit2 (SANE_Unit unit)
{
  switch (unit)
    {
    case SANE_UNIT_PIXEL:
      return "pixels";
    case SANE_UNIT_BIT:
      return "bit";
    case SANE_UNIT_MM:
      return "mm";
    case SANE_UNIT_DPI:
      return "dpi";
    case SANE_UNIT_PERCENT:
      return "%";
    case SANE_UNIT_MICROSECOND:
      return "us";
    default:
      return "";
    };
}


// --------------------------------------------------------------
bool ControlsWindow::QuitRequested()
{
	return true;
}


// --------------------------------------------------------------
void ControlsWindow::MessageReceived
	(
	BMessage *	msg
	)
{
	switch (msg->what)
		{
		case SET_DEVICE_MSG:
			{
			ssize_t 			data_size;
			SANE_Status			status;
			const SANE_Device *	device_info;
			
			if ( msg->FindData("device", B_RAW_TYPE, (const void **) &device_info, &data_size) != B_OK )
				break;
					
			m_device_info = device_info;
					
			if ( m_device )
				sane_close(m_device);
			m_device 		= NULL;
			
			ScannerOptionView * option;
			BView *	child;

			child = m_panel->ChildAt(0);
			while ( child )
				{
				option = dynamic_cast<ScannerOptionView *>(child);
				if ( option )
					option->RemoveSelf();
			
				child = child->NextSibling();
				};
				
			status = sane_open(m_device_info->name, &m_device);
			if ( status != SANE_STATUS_GOOD )
				{
			 	fprintf (stderr, "sane_open: %s\n", sane_strstatus (status));

				BAlert * alert = new BAlert("sane_open", sane_strstatus(status), "Argh");
				alert->Go();
				break;
				};


			const SANE_Option_Descriptor *	desc;
			
			// m_options_lv->MakeEmpty();
			printf("Options for device %s:\n", m_device_info->name);
			int opt = 1;	// skip first option (option 0 = number of options)
			BRect r = m_panel->Bounds();
			r.top = 80;
			r.InsetBy(8, 8);

			while ( (desc = sane_get_option_descriptor(m_device, opt)) != NULL )
				{
				if (desc->type != SANE_TYPE_GROUP)
					{
					ScannerOptionView * ov = new ScannerOptionView(r, desc->name, B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT, 0,
																m_device, opt);
					if ( ov->Build() == B_OK )
						{
						m_panel->AddChild(ov);
						r.top += ov->Bounds().Height();
						m_tooltip->SetText(ov, desc->desc);
						}
					else
						delete ov;
					};

				BString label;
				if (desc->type == SANE_TYPE_GROUP)
					label << "-- ";
				label << desc->title;
				if (desc->type == SANE_TYPE_GROUP)
					label << " --";

				printf("  %d: name = %s\n"
				       "      title = %s\n"
				       "      desc = %s\n"
				       "      type = %d\n"
				       "      unit = %s\n"
					   "      size = %d\n"
				       "      cap  = 0x%0x\n", opt, desc->name, desc->title, desc->desc,
						desc->type, get_unit2(desc->unit), desc->size, desc->cap);


				// m_options_lv->AddItem(new BStringItem(label.String()));

				opt++;
				};

			BMessage * msg;
		
			msg = new BMessage(MainWindow::DEVICE_CHANGED_MSG);
			msg->AddString("device_name", m_device_info->name);
			
			m_parent_window->PostMessage(msg);
			delete msg;
			break;
			};
			
		case SCAN_MSG:
			{
			SANE_Handle		device;
			
			device = Device();
						
			if ( ! device )
				break;
				
			if ( m_scan_thread_id != -1 )
				{
				// already launched...
				m_cancel_scan = true;
				break;
				};
			
			m_cancel_scan = false;

			m_scan_thread_id = spawn_thread(_ScanThread, "scan", B_NORMAL_PRIORITY, this);
			resume_thread(m_scan_thread_id);
			break;
			};	
		
		default:	
			inherited::MessageReceived(msg);
	}
}


// --------------------------------------------------------------
int32 ControlsWindow::ScanThread()
{
	SANE_Status						status;
	SANE_Handle						device;
	int 							len;
	SANE_Parameters 				parm;
	BBitmap *						image;
	bool							first_frame;
	
	device = Device();

	Lock();
	m_status_bar->Show();

	rgb_color default_color = m_status_bar->BarColor();
	
	m_scan_button->SetLabel("Cancel");
	m_status_bar->Reset("Scanning...");
	Unlock();
	
	first_frame = true;
	
	do	//  for each frame...
		{
		// start frame reading
		if ( m_cancel_scan )
			break;
		
		status = sane_start(device);
		if ( status != SANE_STATUS_GOOD )
			{
		 	fprintf (stderr, "sane_start: %s\n", sane_strstatus (status));
			break;
			};
	
		// get frame parameters
	    status = sane_get_parameters(device, &parm);
	    if (status != SANE_STATUS_GOOD)
			{
		 	fprintf (stderr, "sane_get_parameters: %s\n", sane_strstatus (status));
			break;
			};
			
		if (parm.lines >= 0)
			{
			Lock();
			m_status_bar->SetMaxValue(((parm.format == SANE_FRAME_RGB) ? 1.0 : 3.0)  * parm.bytes_per_line * parm.lines);
			Unlock();
	
			fprintf (stderr, "scanning image of size %dx%d pixels at "
	 				"%d bits/pixel\n",
	 				parm.pixels_per_line, parm.lines,
	 				8 * parm.bytes_per_line / parm.pixels_per_line);
				
			if ( first_frame )
				{
				image = new BBitmap(BRect(0, 0, parm.pixels_per_line - 1, parm.lines - 1),
						((parm.depth == 1) ? B_GRAY1 : B_RGBA32));
				
				m_parent_window->Lock();
				m_parent_window->SetImage(image);
				m_parent_window->Unlock();
				};
			}
	     else
			{
			fprintf (stderr, "scanning image %d pixels wide and "
				 "variable height at %d bits/pixel\n",
		 		parm.pixels_per_line, 8 * parm.bytes_per_line / parm.pixels_per_line);
		
			BAlert * alert = new BAlert("Well...", "Variable height scanning not supported (yet?)", "Sorry");
			alert->Go();
			break;
			};
		
	
		uint8 * buffer, * data;
		uint8 * 	ptr;
		int32		x, y;
		int			channel;
		rgb_color	red_color	= { 255, 0, 0, 255 };
		rgb_color	green_color	= { 0, 255, 0, 255 };
		rgb_color	blue_color	= { 0, 0, 255, 255 };
		BRect		updated_rect;
			
		x = y = 0;
		channel = 0;
		
		updated_rect.Set(0, 0, parm.pixels_per_line, 0);
		Lock();
		switch(parm.format)
			{
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
	
		while (1)	// read until end of frame or error
			{
			if ( m_cancel_scan )
				break;
			
			len = 0;
			status = sane_read(device, buffer, parm.bytes_per_line, &len);
			if (status != SANE_STATUS_GOOD)
			    {
			    if (status != SANE_STATUS_EOF)
					{
					fprintf (stderr, "sane_read: %s\n", sane_strstatus (status));

					BAlert * alert = new BAlert("sane_read()", sane_strstatus(status), "Glup.");
					alert->Go();
					};
				break;
				};
					
			Lock();
			m_status_bar->Update((float) len);
			Unlock();
	
			if ( image )
				{
				image->Lock();
					
				ptr = (uint8 *) image->Bits();
				ptr += (y * image->BytesPerRow());
				ptr += (4*x);
	
				data = buffer;
				while ( len > 0 )
					{
			      	switch (parm.format)
						{
						case SANE_FRAME_RED:
						case SANE_FRAME_GREEN:
						case SANE_FRAME_BLUE:
							{
							uint8	value;
	
							if (parm.depth == 16)
								{
								value = (*((uint16 *) data)) >> 8;
								data += 2;
								len -= 2;
								}
							else
								{
								value = *((uint8 *) data);
								data++;
								len--;
								};
	
							*(ptr + channel) = value;
							*(ptr + 3)		= 255;	// Alpha channel
							break;
							};
							
						case SANE_FRAME_RGB:
							{
							uint8 red, green, blue;
							
							if (parm.depth == 16)
								{
								red = (*((uint16 *) data)) >> 8;
								data += 2;
								green = (*((uint16 *) data)) >> 8;
								data += 2;
								blue = (*((uint16 *) data)) >> 8;
								data += 2;
								len -= 6;
								}
							else
								{
								red = *((uint8 *) data);
								data++;
								green = *((uint8 *) data);
								data++;
								blue = *((uint8 *) data);
								data++;
								len -= 3;
								};
	
							*ptr 		= blue;
							*(ptr+1) 	= green;
							*(ptr+2) 	= red;	// red channel
							*(ptr+3) 	= 255;		// Alpha channel
							break;
							};
							
						case SANE_FRAME_GRAY:
							{
							uint8	value;
							
							if (parm.depth == 1 )
								{
								*ptr = *((uint8 *) data);
								data++;
								len--;
								break;
								};
							
							if (parm.depth == 16)
								{
								value = (*((uint16 *) data)) >> 8;
								data += 2;
								len -= 2;
								}
							else if ( parm.depth == 8 )
								{
								value = *((uint8 *) data);
								data++;
								len--;
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
	
					if ( x >= parm.pixels_per_line )
						{
						y++;
						if ( y >= parm.lines )
							break;
						x = 0;
						// skip end of row padding bytes...
						ptr = (uint8 *) image->Bits();
						ptr += (y * image->BytesPerRow());
						};
	
					};
				image->Unlock();
	
				BMessage *	msg;						
				msg = new BMessage(MainWindow::UPDATED_IMAGE_MSG);
				updated_rect.bottom = y;
				msg->AddRect("updated_rect", updated_rect);
	
				m_parent_window->Lock();
				m_parent_window->PostMessage(msg);
				m_parent_window->Unlock();
				
				delete msg;
				
				updated_rect.top = y;
				};
			
			};	// while sane_read()
				
		free(buffer);
			
		first_frame = false;
			
		}
	while ( ! parm.last_frame );
	
	sane_cancel(device);
	
	Lock();
	m_scan_button->SetLabel("Scan");
	m_status_bar->Hide();
	m_status_bar->SetBarColor(default_color);
	Unlock();
	
	m_scan_thread_id = -1;
	return B_OK;
}

// --------------------------------------------------------------
int32 ControlsWindow::DevicesRosterThread()
{
	BMenuItem *				item;
	SANE_Status				status;
	const SANE_Device ** 	devices_list;
	SANE_Handle				device;

	status = sane_get_devices(&devices_list, SANE_FALSE);
    if (status == SANE_STATUS_GOOD)
		{
		int	i;
		for (i = 0; devices_list[i]; ++i)
			{
			BString 	label;
			BMessage * 	msg;
			
			label << devices_list[i]->name;
			// label << " (" << devices_list[i]->vendor << " " << devices_list[i]->model << ")";

			msg = new BMessage(SET_DEVICE_MSG);
			msg->AddData("device", B_RAW_TYPE, devices_list[i], sizeof(devices_list[i])); 
			
			status = sane_open(devices_list[i]->name, &device);
			switch (status)
				{
				case SANE_STATUS_GOOD:
					sane_close(device); break;
					
				case SANE_STATUS_DEVICE_BUSY:
					label << " [busy]"; break;
					
				case SANE_STATUS_ACCESS_DENIED:
					label << " [access denied]"; break;
					
				default:
					label << " [error]"; break;
				};

			item = new BMenuItem(label.String(), msg);
				
			if ( status != SANE_STATUS_GOOD )
				item->SetEnabled(false);
				
			m_devices_menu->AddItem(item);
			};
		};

	return B_OK;
}



