#include <Application.h>
#include <Roster.h>		// for app_info
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <SupportKit.h>
#include <TranslationKit.h>
#include <Catalog.h>

#include <stdlib.h>

#include "Sanity.h"
#include "ScannerWindow.h"
#include "ScannerOptionView.h"
#include "SpinControl.h"
#include "CollapsableBox.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ScannerOptionView"

static bool			reload_option(void *ov, void *device);

// Constructor & destructor
// ------------------------

BList ScannerOptionView::m_options_list = BList();
BLocker ScannerOptionView::m_options_list_locker = BLocker();


// --------------------------------------------------------------
ScannerOptionView::ScannerOptionView
	(
	BRect			frame,
    const char *	name, 
    uint32 			resizeMask, 
  	uint32 			flags, 	
	SANE_Handle 	device,
	int 			option_number
	)
	: BView(frame, name, resizeMask, flags & ~B_WILL_DRAW | B_FRAME_EVENTS)
{
	m_device	= device;
	m_opt		= option_number;
	m_desc 		= sane_get_option_descriptor(m_device, m_opt);
	m_auto_mode = false;

	m_last_value = NULL;
	m_enabled	= 8;
	
	m_icon = NULL;
	
	switch (m_desc->type)
		{
		case SANE_TYPE_INT:
		case SANE_TYPE_FIXED:
			m_nb_values = m_desc->size / sizeof(SANE_Word);
			break;

		default:
			m_nb_values = 1;
			break;
		};
		
	switch (m_desc->type)
		{
		case SANE_TYPE_BOOL:
			m_type = BOOLEAN;
			break;
			
		case SANE_TYPE_BUTTON:
			m_type = COMMAND;
			break;
			
		case SANE_TYPE_STRING:
			if (m_desc->constraint_type == SANE_CONSTRAINT_STRING_LIST)
				m_type = STRING_LIST;
			else
				m_type = STRING;
			break;
		
		case SANE_TYPE_INT:
		case SANE_TYPE_FIXED:
			if ( m_nb_values > 1 )
				{
				m_type = NUMERIC_ARRAY;
				break;
				};
			
			if (m_desc->constraint_type == SANE_CONSTRAINT_WORD_LIST)
				m_type = NUMERIC_LIST;
			else if (m_desc->constraint_type == SANE_CONSTRAINT_RANGE)
				m_type = NUMERIC_RANGE;
			else
				m_type = NUMERIC;
			break;
			
		default:
			m_type = UNKNOWN;
			break;
		};
		
	m_enabled	= 8; // SANE_OPTION_IS_SETTABLE(m_desc->cap) && SANE_OPTION_IS_ACTIVE(m_desc->cap);

	// Add ourself to the global ScannerOptionViews list
	m_options_list_locker.Lock();
	m_options_list.AddItem(this);
	m_options_list_locker.Unlock();
	
	PrintToStream();
}

// --------------------------------------------------------------
ScannerOptionView::~ScannerOptionView()
{
	// Remove ourself from the global ScannerOptionsViews list
	m_options_list_locker.Lock();
	m_options_list.RemoveItem(this);
	m_options_list_locker.Unlock();
	
	free(m_last_value);
	
	delete m_icon;
}

//	#pragma mark -

// Public methods
// --------------

// --------------------------------------------------------------
void ScannerOptionView::AttachedToWindow()
{
	if ( Parent() )
		SetViewColor(Parent()->ViewColor());
	//	SetViewColor((rand() % 256), (rand() % 256), (rand() % 256));
		
	inherited::AttachedToWindow();
}


// --------------------------------------------------------------
void ScannerOptionView::MouseDown(BPoint where)
{
	Window()->Activate();
	inherited::MouseDown(where);
}

// --------------------------------------------------------------
void ScannerOptionView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
		{
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		case BROWSE_FILE_MSG:
		case BROWSE_DIR_MSG: {
			// msg->PrintToStream();

			if ( ! m_enabled )
				break;

		   	entry_ref ref;

  			// Look for a ref in the message
   			if( msg->FindRef("refs", &ref) == B_OK ) {
				BEntry 	entry(&ref);
				BPath	path;
				
				if ( entry.InitCheck() != B_OK )
					break;
				
				entry.GetPath(&path);
					
				BTextControl * control = dynamic_cast<BTextControl *>(FindView("value"));
				if ( control ) {
					// Support drop path to a textcontrol
					control->SetText(path.Path());
					Looper()->PostMessage(SET_VALUE_MSG, this);
				};
				break;
	   		};

			// Call inherited if we didn't handle the message
   			inherited::MessageReceived(msg);
			break;
		}
			
		case SET_VALUE_MSG:
			SetValue(msg);
			break;
			
		case RELOAD_MSG:
			UpdateValue();
			break;
			
		default:	
			inherited::MessageReceived(msg);
		};
}


void ScannerOptionView::Draw(BRect updateRect)
{
	inherited::Draw(updateRect);
	
	if (!m_icon)
		return;

	PushState();
	if (!m_enabled) {
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
		SetHighColor(0, 0, 0, 32);
	} else
		SetDrawingMode(B_OP_OVER);
	DrawBitmapAsync(m_icon, BPoint(0, 0));
	PopState();
}

// #pragma mark -

// --------------------------------------------------------------
status_t ScannerOptionView::Build(void)
{
	if ( ! m_desc )
		return B_ERROR;

	BString label;
	BRect 	r = Bounds();
	float	w, h;
	
	w = r.Width();
	h = r.Height();

	label = m_desc->title;
	
	InitIcon();
	if (m_icon) {
		SetFlags(Flags() | B_WILL_DRAW);
		r.left += m_icon->Bounds().Width() + 2;
	}
	
	switch (m_type) {
	case BOOLEAN: {
		BCheckBox * cb;
		cb = new BCheckBox(r, "value", label.String(),
						   new BMessage(SET_VALUE_MSG),
							B_FOLLOW_LEFT | B_FOLLOW_TOP);
		AddChild(cb);
		cb->ResizeToPreferred();
		cb->GetPreferredSize(&w, &h);
			
		cb->SetTarget(this);
		break;
	}
			
	case COMMAND: {
		BButton * bt;
		r.top += 6;
		bt = new BButton(r, "value", label.String(),
								new BMessage(SET_VALUE_MSG));
		AddChild(bt);
		bt->ResizeToPreferred();
		bt->GetPreferredSize(&w, &h);
		
		h += 12;
			
		bt->SetTarget(this);
		break;
	}
			
	case NUMERIC: {
		SpinControl * sc;
		r.bottom = r.top + 24;
		if (UnitLabel())
			label << " (" << UnitLabel() << ")";
		label << ":";
		sc = new SpinControl(r, "value", label.String(), new BMessage(SET_VALUE_MSG),
								0, 100, 0, 10,
								B_FOLLOW_LEFT | B_FOLLOW_TOP);
		AddChild(sc);
			
		label << "#";
		sc->SetDivider(be_plain_font->StringWidth(label.String()));

		sc->ResizeToPreferred();
		sc->GetPreferredSize(&w, &h);
		sc->ResizeTo(w, sc->Frame().Height());
			
		sc->SetTarget(this);
		break;
	}
			
	case NUMERIC_LIST:
	case STRING_LIST: {
		BMenuField *	mf;
		BPopUpMenu *	pum;
			
		pum = new BPopUpMenu(B_TRANSLATE("<pick one>"));
		pum->SetRadioMode(true);
		pum->SetTargetForItems(this);
		pum->SetTriggersEnabled(false);
			
		label << ":";
		mf = new BMenuField(r, "value", label.String(), pum,
							B_FOLLOW_LEFT | B_FOLLOW_TOP);
		AddChild(mf);
			
		label << "#";
			
		mf->SetDivider(be_plain_font->StringWidth(label.String()));
		mf->ResizeToPreferred();
		mf->GetPreferredSize(&w, &h);
			
		h++;	// Account the menu field shadow
		break;
	}
			
	case NUMERIC_RANGE: {
		BSlider *slider;
		BString min_label, max_label;

		r.top += 2;	// Add some extra top margin, slider's label looks better that way...

		label << ": ";
		slider = new BSlider(r, "value", label.String(),
							new BMessage(SET_VALUE_MSG),
							RangeMin(), RangeMax(),
							B_BLOCK_THUMB,
							// B_TRIANGLE_THUMB,
							ResizingMode());

		AddChild(slider);
		slider->SetModificationMessage(new BMessage(SET_VALUE_MSG));	// For live change
		
		if (m_desc->type == SANE_TYPE_FIXED) {
			float min = FloatRangeMin();
			float max = FloatRangeMax();
			const char *l = UnitLabel(&min);
			min_label << min << " " << l;
			l = UnitLabel(&max);
			max_label << max << " " << l;
		} else {
			int min = RangeMin();
			int max = RangeMax();
			const char *l = UnitLabel(&min);
			min_label << min << " " << l;
			l = UnitLabel(&max);
			max_label << max << " " << l;
		};
					
		slider->SetLimitLabels(min_label.String(), max_label.String());
/*
		rgb_color color = {102, 152, 203, 255};
		slider->UseFillColor(true, &color);
*/
		if ( m_desc->constraint.range->quant > 0) {
			int steps = (m_desc->constraint.range->max - m_desc->constraint.range->min) /  m_desc->constraint.range->quant;
			if (steps > 1 && steps < 20) {
				slider->SetHashMarks(B_HASH_MARKS_TOP);
				slider->SetHashMarkCount(steps);
			};
			slider->SetKeyIncrementValue(m_desc->constraint.range->quant);
		};
					
		slider->ResizeToPreferred();
		slider->GetPreferredSize(&w, &h);
	
		h += 8;	// Add some space at the bottom, slider don't stack that well visually...

		slider->SetTarget(this);
		break;
	}
			
	case STRING: {
		BTextControl * 	tc;
		
		if (UnitLabel())
			label << " (" << UnitLabel() << ")";
		label << ":";
		tc = new BTextControl(r, "value", label.String(), "", // value
								new BMessage(SET_VALUE_MSG),
								ResizingMode());
		AddChild(tc);

		label << "#";
		
		tc->SetDivider(be_plain_font->StringWidth(label.String()));
		tc->GetPreferredSize(&w, &h);
		tc->ResizeTo(r.Width(), h);
			
		tc->SetTarget(this);
		break;
	}
						
	case UNKNOWN:
	default: {
		// if (UnitLabel())
		//	label << " (" << UnitLabel() << ")";
		label << " " << B_TRANSLATE("[Unsupported]");
	
		BStringView * sv;
		sv = new BStringView(r, m_desc->name, label.String(),
							ResizingMode());
		AddChild(sv);
		sv->ResizeToPreferred();
		sv->GetPreferredSize(&w, &h);
		break; };
	};

	if (m_icon && h < m_icon->Bounds().Height())
		// Take care of icon full visibility
		h = m_icon->Bounds().Height();

	ResizeTo(Bounds().Width(), h);
	return UpdateValue();
}

float ScannerOptionView::FloatRangeMin()
{
	if (m_type != NUMERIC_RANGE)
		return 0;

	return SANE_UNFIX(m_desc->constraint.range->min);
}

float ScannerOptionView::FloatRangeMax()
{
	if (m_type != NUMERIC_RANGE)
		return 0;

	return SANE_UNFIX(m_desc->constraint.range->max);
}

int ScannerOptionView::RangeMin()
{
	if (m_type != NUMERIC_RANGE)
		return 0;

	return m_desc->constraint.range->min;
}

int ScannerOptionView::RangeMax()
{
	if (m_type != NUMERIC_RANGE)
		return 0;

	return m_desc->constraint.range->max;
}

const char * ScannerOptionView::UnitLabel(int *value)
{
	if (!value)
		return UnitLabel((float *) NULL);
		
	float f = *value;
	const char *label = UnitLabel(&f);
	// Round to nearest integer value...
	*value =  (int) ((f >= 0.0) ? (f + 0.5) : (f - 0.5));
	return label;
}


const char * ScannerOptionView::UnitLabel(float *value)
{
	bool plurial = false;
	if (value)
		plurial = (*value < -1.0 || *value > 1.0);
	
	switch (m_desc->unit) {
	case SANE_UNIT_DPI:			return "dpi";
	case SANE_UNIT_PERCENT:	    return "%";
	case SANE_UNIT_PIXEL:		return plurial ? "pixels" : "pixel";
	case SANE_UNIT_BIT:
		if (value && (((int) *value) % 8 == 0) ) {
			*value /= 8;	// convert into byte(s)
			if (*value >= 1024 * 1024 * 1024) {
				*value /= 1024 * 1024 * 1024;
				return "Gb";
			}
			if (*value >= 1024 * 1024) {
				*value /= 1024 * 1024;
				return "Mb";
			}
			if (*value >= 1024) {
				*value /= 1024;
				return "Kb";
			}
			return (*value < -1 || *value > 1) ? "bytes" : "byte";
		}
		return plurial ? "bits" : "bit";
		
	case SANE_UNIT_MM:
		// TODO: metric -> inch & co support
		if (value) {
			if (*value >= 1000) {
				*value /= 1000;
				return "m";
			}
			if (*value >= 10) {
				*value /= 10;
				return "cm";
			}
		}
		return "mm";

	case SANE_UNIT_MICROSECOND:
		if (value) {
			if (*value >= 1000000) {
				*value /= 1000000;
				return "s";
			}
			if (*value >= 1000) {
				*value /= 1000;
				return "ms";
			}
		}
		return "\xC2\xB5";	// UTF-8 codes for 'micro' char  (\302\265s)

	default:
		return NULL;
    }
}


// --------------------------------------------------------------
void ScannerOptionView::SetValue(BMessage *msg)
{
	SANE_Status	status;
	SANE_Word	word_value;
	SANE_Bool 	bool_value;
	SANE_Int 	info;
	SANE_Action action;
	void *		new_value;
	char *		str;
			
	// msg->PrintToStream();

	action = SANE_ACTION_SET_VALUE;
	new_value = NULL;
			
	if (msg->FindBool("auto", (bool *) &bool_value) == B_OK) {
		fprintf(stderr, "SET_VALUE_MSG %s: auto value asked!\n", m_desc->name);
		action = SANE_ACTION_SET_AUTO;
	} else {
		switch (m_type)	{
		case BOOLEAN: {
			BControl *control = dynamic_cast<BControl *>(FindView("value"));
			if ( ! control )
				break;
			bool_value = control->Value();
			new_value = &bool_value;
		 	printf("sane_control_option(%s, SET_VALUE, [%d])...\n",
				m_desc->name, bool_value);
			break;
		}

		case STRING: {
			BTextControl *tc = dynamic_cast<BTextControl *>(FindView("value"));
			if ( tc ) {
				str = (char *) tc->Text();
				new_value = str;
				if (strlen(str) > (unsigned int) m_desc->size)
					str[m_desc->size] = 0;	// truncate the string
							
			 	printf("sane_control_option(%s, SET_VALUE, [%s])...\n",
					m_desc->name, (char *) new_value);
			};
			break;
		}
					
		case STRING_LIST:
			if (msg->FindString("value", (const char **) &str) == B_OK) {
				new_value = str;
				if (strlen(str) > (unsigned int) m_desc->size)
					str[m_desc->size] = 0;	// truncate the string

			 	printf("sane_control_option(%s, SET_VALUE, [%s])...\n",
					m_desc->name, (char *) new_value);
			};
			break;

		case NUMERIC_LIST:
			if (msg->FindInt32("value", (int32 *) &word_value) == B_OK) {
				new_value = &word_value;
			 	printf("sane_control_option(%s, SET_VALUE, [%d])...\n",
					m_desc->name, word_value);
			};
			break;
			
		case NUMERIC:
		case NUMERIC_RANGE: {
			BControl *control = dynamic_cast<BControl *>(FindView("value"));
			if ( ! control )
				break;
			word_value = control->Value();
			new_value = &word_value;
			
		 	printf("sane_control_option(%s, SET_VALUE, [", m_desc->name);
			if (m_desc->type == SANE_TYPE_FIXED)
				printf("%g", SANE_UNFIX(word_value));			
			else
				printf("%d", word_value);			
			printf(" (0x%04x])...\n", word_value);
			break;
		}
				
		case COMMAND:
			bool_value = true;
			new_value = &bool_value;
		 	printf("sane_control_option(%s, SET_VALUE, [%d])...\n",
				m_desc->name, bool_value);
			printf("SANE_FIX(-42.17) = 0x%04x\n", SANE_FIX(-42.17));
			printf("SANE_FIX(2.00) = 0x%04x\n", SANE_FIX(2.00));
			break;
					
		default:
			break;
		};
					
		if ( new_value == NULL ) {
			fprintf(stderr, "SET_VALUE_MSG %s: no value to set found!\n", m_desc->name);
			return;
		};
	};

	status = sane_control_option(m_device, m_opt, action,
								new_value, &info);
	if ( status != SANE_STATUS_GOOD ) {
	 	fprintf (stderr, "sane_control_option(%s, SET_VALUE): %s\n",
				m_desc->name, sane_strstatus (status));
		return;
	};

	if (info & SANE_INFO_RELOAD_OPTIONS) {
		printf("sane_control_option(%s, SET_VALUE): SANE_INFO_RELOAD_OPTIONS\n",
				m_desc->name);
		// At least one option description fields have changed, reload it/them
		m_options_list_locker.Lock();
		m_options_list.DoForEach(reload_option, (void *) m_device);
		m_options_list_locker.Unlock();
	} else if (info & SANE_INFO_INEXACT) {
		printf("sane_control_option(%s, SET_VALUE): SANE_INFO_INEXACT\n",
				m_desc->name);
		UpdateValue();
	} else if (m_type == NUMERIC_RANGE)
		// Live refresh the slider label to report current value
		UpdateValue();

	if (info & SANE_INFO_RELOAD_PARAMS) {
		printf("sane_control_option(%s, SET_VALUE): SANE_INFO_RELOAD_PARAMS\n",
				m_desc->name);

		// Notify main window
		Window()->PostMessage(ScannerWindow::FORMAT_CHANGED_MSG);
	}		
	
}


// --------------------------------------------------------------
status_t ScannerOptionView::UpdateValue(void)
{
	bool		enabled;
	void 		*value;
	SANE_Int	info;
	SANE_Status	status;
	BString 	label;
	
	if ( ! m_desc )
		return B_ERROR;

	enabled = SANE_OPTION_IS_SETTABLE(m_desc->cap) && SANE_OPTION_IS_ACTIVE(m_desc->cap);

	value = malloc(m_desc->size + 16);
		
	status = sane_control_option(m_device, m_opt, SANE_ACTION_GET_VALUE, value, &info);
	if (status != SANE_STATUS_GOOD)
		goto exit;

	if (m_last_value && memcmp(m_last_value, value, m_desc->size) == 0)
		// Value didn't changed from last known one...
		goto exit;

	switch (m_type) {
	case BOOLEAN: {
		BControl *control = dynamic_cast<BControl *>(FindView("value"));
		if (control)
			control->SetValue(*(SANE_Bool *) value);
		break;
	}
			
	case NUMERIC:
	case NUMERIC_RANGE: {
		SANE_Word w = *(SANE_Word *) value;
	
		BControl *control = dynamic_cast<BControl *>(FindView("value"));
		if (control)
			control->SetValue(w);
			
		BSlider *slider = dynamic_cast<BSlider *>(control);
		if (slider){
			label = m_desc->title;
			label << ": ";
			if (m_desc->type == SANE_TYPE_FIXED) {
				float v = SANE_UNFIX(w);
				const char *l = UnitLabel(&v);
				label << v << " " << l;
			} else {
				int v = w;
				const char *l = UnitLabel(&v);
				label << v << " " << l;
			}
			BRegion region(BRect(0, 0, slider->Bounds().Width(), 20));
			slider->ConstrainClippingRegion(&region);
			slider->SetLabel(label.String());
			slider->ConstrainClippingRegion(NULL);
		}
	
		// PrintToStream();
		break;
	}
			
	case STRING: {
		BTextControl *tc = dynamic_cast<BTextControl *>(FindView("value"));
		if (tc) {
			tc->SetText((char *) value);
			tc->TextView()->SetMaxBytes(m_desc->size);
		}
		break;			
	}
			
	case STRING_LIST:
	case NUMERIC_LIST: {
		BMenuField *mf = dynamic_cast<BMenuField *>(FindView("value"));
		if (!mf)
			break;
				
		BMenu *menu = mf->Menu();
		BMenuItem *item;
		BMessage *msg;
				
		// clear menu content
		while ((item = menu->RemoveItem((int32) 0)))
			delete item;
	
		if (m_desc->cap & SANE_CAP_AUTOMATIC) {
			msg = new BMessage(SET_VALUE_MSG);
			msg->AddBool("auto", true);
			item = new BMenuItem(B_TRANSLATE("Auto"), msg);
			menu->AddItem(item);
			if (m_auto_mode)
				item->SetMarked(true);
		};
	
		if (m_type == STRING_LIST) {
			for (int i = 0; m_desc->constraint.string_list[i]; i++) {
				msg = new BMessage(SET_VALUE_MSG);
				msg->AddString("value", m_desc->constraint.string_list[i]);

				label = m_desc->constraint.string_list[i];
				label << " " << UnitLabel();
				item = new BMenuItem(label.String(), msg);
				menu->AddItem(item);
				if (!m_auto_mode && strcmp((char *) value, m_desc->constraint.string_list[i]) == 0)
					item->SetMarked(true);
			}			
		} else {
	  		for (int i = 1; i <= m_desc->constraint.word_list[0]; i++) {
				msg = new BMessage(SET_VALUE_MSG);
				msg->AddInt32("value", m_desc->constraint.word_list[i]);

				label = "";
				if (m_desc->type == SANE_TYPE_FIXED) {
					float v = SANE_UNFIX(m_desc->constraint.word_list[i]);
					const char *l = UnitLabel(&v);
					label << v << " " << l;
				} else {
					int v = m_desc->constraint.word_list[i];
					const char *l = UnitLabel(&v);
					label << v << " " << l;
				}

				item = new BMenuItem(label.String(), msg);
				menu->AddItem(item);
				if (!m_auto_mode && (*((SANE_Word *) value)) ==  m_desc->constraint.word_list[i])
					item->SetMarked(true);
			};
			
		};

		menu->SetTargetForItems(this);
		break;
	}
			
	default:
		break;
	};

	// Backup previous value... 
	if (!m_last_value)
		m_last_value = malloc(m_desc->size + 16);
	if (m_last_value)
		memcpy(m_last_value, value, m_desc->size);

exit:
	SetEnabled(enabled);
	
	free(value);
		
	return B_OK;
}


// --------------------------------------------------------------
status_t ScannerOptionView::SetEnabled
	(
	bool	on
	)
{
	BControl *control;
	BMenuField *menu_field;
	BStringView *string_view;
	BView *child;

	if (on == m_enabled)
		// Already set
		return B_OK;

	child = ChildAt(0);
	for (child = ChildAt(0); child; child = child->NextSibling()) {
		control = dynamic_cast<BControl *>(child);
		if ( control ) {
			control->SetEnabled(on);
			continue;
		}
			
		menu_field = dynamic_cast<BMenuField *>(child);
		if ( menu_field ) {
			menu_field->SetEnabled(on);
			continue;
		}
			
		string_view = dynamic_cast<BStringView *>(child);
		if ( string_view ) {
			rgb_color text_enabled = ui_color(B_MENU_ITEM_TEXT_COLOR);
			rgb_color text_disabled = {128, 128, 128, 255};	// hardcoded!!! :-(		
			string_view->SetHighColor(on ? text_enabled : text_disabled);
			string_view->Invalidate();
			continue;
		}
	};	// for each child view

	m_enabled = on;
	return B_OK;
}


// --------------------------------------------------------------
status_t ScannerOptionView::SetAutoMode
	(
	bool	on
	)
{
	// TODO
	return B_ERROR;
}

//------------------------------------------------------------------------------
void ScannerOptionView::InitIcon()
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
			const void *data = Resources.LoadResource('ICON', m_desc->name, &size);
			if (data) {
				// Now build the bitmap
				icon = new BBitmap(BRect(0, 0, 31, 31), B_CMAP8);
				icon->SetBits(data, size, 0, B_CMAP8);
			}
		}
	}
	m_icon = icon;
}


// --------------------------------------------------------------
void ScannerOptionView::PrintToStream()
{
	int opt = m_opt;
	const SANE_Option_Descriptor *od = m_desc;
	bool not_first = false;
	
	printf("[Option # %d]\n"
		   "      name:  %s\n"
	       "      title: %s\n"
	       "      desc:  %s\n"
	       "      type:  %s\n", 
	       opt, od->name, od->title, od->desc,
	   od->type == SANE_TYPE_BOOL ? "SANE_TYPE_BOOL" :
	   od->type == SANE_TYPE_INT ? "SANE_TYPE_INT" :
	   od->type == SANE_TYPE_FIXED ? "SANE_TYPE_FIXED" :
	   od->type == SANE_TYPE_STRING ? "SANE_TYPE_STRING" :
	   od->type == SANE_TYPE_BUTTON ? "SANE_TYPE_BUTTON" :
	   od->type == SANE_TYPE_GROUP ? "SANE_TYPE_GROUP" : "unknown");
	printf("      unit:  %s",
	   od->unit == SANE_UNIT_NONE ? "SANE_UNIT_NONE" :
	   od->unit == SANE_UNIT_PIXEL ? "SANE_UNIT_PIXEL" :
	   od->unit == SANE_UNIT_BIT ? "SANE_UNIT_BIT" :
	   od->unit == SANE_UNIT_MM ? "SANE_UNIT_MM" :
	   od->unit == SANE_UNIT_DPI ? "SANE_UNIT_DPI" :
	   od->unit == SANE_UNIT_PERCENT ? "SANE_UNIT_PERCENT" :
	   od->unit == SANE_UNIT_MICROSECOND ? "SANE_UNIT_MICROSECOND" :
	   "unknown");
	if (UnitLabel())
		printf("(%s)", UnitLabel());
	printf("\n");
	printf("      size: %d\n", od->size);
	printf("      caps: ");

      if (od->cap & SANE_CAP_SOFT_SELECT)
		printf("SANE_CAP_SOFT_SELECT ");
      if (od->cap & SANE_CAP_HARD_SELECT)
		printf("SANE_CAP_HARD_SELECT ");
      if (od->cap & SANE_CAP_SOFT_DETECT)
		printf("SANE_CAP_SOFT_DETECT ");
      if (od->cap & SANE_CAP_EMULATED)
		printf("SANE_CAP_EMULATED ");
      if (od->cap & SANE_CAP_AUTOMATIC)
		printf("SANE_CAP_AUTOMATIC ");
      if (od->cap & SANE_CAP_INACTIVE)
		printf("SANE_CAP_INACTIVE ");
      if (od->cap & SANE_CAP_ADVANCED)
		printf("SANE_CAP_ADVANCED ");
	printf("\n");
	printf("      constraint: %s\n",
	   od->constraint_type == SANE_CONSTRAINT_NONE ?
	   "SANE_CONSTRAINT_NONE" :
	   od->constraint_type == SANE_CONSTRAINT_RANGE ?
	   "SANE_CONSTRAINT_RANGE" :
	   od->constraint_type == SANE_CONSTRAINT_WORD_LIST ?
	   "SANE_CONSTRAINT_WORD_LIST" :
	   od->constraint_type == SANE_CONSTRAINT_STRING_LIST ?
	   "SANE_CONSTRAINT_STRING_LIST" : "unknown");

	printf("      ");
	if (od->type == SANE_TYPE_BOOL) {
		fputs ("[=(", stdout);
		if (od->cap & SANE_CAP_AUTOMATIC)
			fputs ("auto|", stdout);
		fputs ("yes|no)]", stdout);
	} else if (od->type != SANE_TYPE_BUTTON) {
		fputc (' ', stdout);
		if (od->cap & SANE_CAP_AUTOMATIC) {
			fputs ("auto|", stdout);
	  		not_first = SANE_TRUE;
		}
		switch (od->constraint_type) {
		case SANE_CONSTRAINT_NONE:
			switch (od->type) {
	    	case SANE_TYPE_INT:
	      		fputs ("<int>", stdout);
	      		break;
	    	case SANE_TYPE_FIXED:
	      		fputs ("<float>", stdout);
	      		break;
	    	case SANE_TYPE_STRING:
	      		fputs ("<string>", stdout);
	      		break;
	    	default:
	      		break;
	    	}
	    	if (od->type != SANE_TYPE_STRING && od->size > (SANE_Int) sizeof (SANE_Word))
	    		fputs (",...", stdout);
	  		break;

		case SANE_CONSTRAINT_RANGE:
	  		if (od->type == SANE_TYPE_INT) {
	      		printf ("%d..%d%s", RangeMin(), RangeMax(), UnitLabel() ? UnitLabel() : "");
	      		if (od->size > (SANE_Int) sizeof (SANE_Word))
					fputs (",...", stdout);
				if (od->constraint.range->quant)
					printf (" (in steps of %d)", od->constraint.range->quant);
	    	} else {
	      		printf ("%g..%g%s", FloatRangeMin(), FloatRangeMax(), UnitLabel() ? UnitLabel() : "");
	      		if (od->size > (SANE_Int) sizeof (SANE_Word))
				fputs (",...", stdout);
	      		if (od->constraint.range->quant)
					printf (" (in steps of %g)", SANE_UNFIX (od->constraint.range->quant));
	    	}
	  		break;

		case SANE_CONSTRAINT_WORD_LIST:
	  		for (int i = 1; i < od->constraint.word_list[0]; i++) {
	      		if (not_first)
					fputc ('|', stdout);

	    		  not_first = SANE_TRUE;

	    		if (od->type == SANE_TYPE_INT)
					printf ("%d", od->constraint.word_list[i]);
	      		else
					printf ("%g", SANE_UNFIX (od->constraint.word_list[i]));
	    	}
	  		printf("%s", UnitLabel() ? UnitLabel() : "");
	  		if (od->size > (SANE_Int) sizeof (SANE_Word))
	    		fputs (",...", stdout);
	  		break;

		case SANE_CONSTRAINT_STRING_LIST:
	  		for (int i = 0; od->constraint.string_list[i]; i++) {
	  			if (i > 0)
					fputc ('|', stdout);
				fputs (od->constraint.string_list[i], stdout);
	    	}
	  		break;
		}
    }
  
	if (od->type == SANE_TYPE_STRING || od->size == sizeof (SANE_Word)) {
    	/* print current option value */
    	fputs (" [", stdout);
    	if (SANE_OPTION_IS_ACTIVE (od->cap)) {
	  		void *val = malloc(od->size);
	  		sane_control_option (m_device, opt, SANE_ACTION_GET_VALUE, val, 0);
	  		switch (od->type) {
	  		case SANE_TYPE_BOOL:
	      		fputs (*(SANE_Bool *) val ? "yes" : "no", stdout);
	      		break;

	    	case SANE_TYPE_INT:
	    		printf ("%d", *(SANE_Int *) val);
	      		break;

	    	case SANE_TYPE_FIXED:
	      		printf ("%g", SANE_UNFIX (*(SANE_Fixed *) val));
	      		break;

	    	case SANE_TYPE_STRING:
	      		fputs ((char *) val, stdout);
	      		break;

	    	default:
	      		break;
	    	}
		} else
			fputs ("inactive", stdout);
		fputc (']', stdout);
    }
	fputc('\n', stdout);
}

// #pragma mark -


static bool reload_option(void *arg1, void *arg2)
{
	ScannerOptionView *ov;
	SANE_Handle device = (SANE_Handle) arg2;
	
	ov = (ScannerOptionView *) arg1;
	if (!ov)
		return false;
	
	if (ov->Device() != device)
		return false;
	
	BWindow *win = ov->Window();
	if (win) {
		win->DisableUpdates();
		win->BeginViewTransaction();
	}

	ov->UpdateValue();

	if (win) {
		win->EndViewTransaction();
		win->EnableUpdates();
		ov->Sync();
	}
	return false;
}




