#ifndef SCANNERWINDOW_H
#define SCANNERWINDOW_H

#include <Window.h>

#include <sane/sane.h>
#include <sane/saneopts.h>

// Forward declarations
class PreviewView;
class BitmapView;
class StackView;
class TranslatorSavePanel;
class ScannerOptionView;
class BBox;
class BMenuField;
class BPopUpMenu;
class BStatusBar;
class BButton;
class BScrollView;

#define SCAN_LABEL		"Scan"
#define PRESET_LABEL	"Preset:"
#define DEVICE_LABEL	"Device:"

// THE window class
class ScannerWindow : public BWindow  
{
public:
		// Constructors, destructors, operators...
		ScannerWindow(BRect Frame, BBitmap **outBitmap=NULL);
		~ScannerWindow();

		typedef BWindow 		inherited;


		// public constantes
		enum	{
			SAVE_AS_MSG			= 'sava',
			SAVE_FILE_PANEL_MSG	= 'sfpm',
			SET_DEVICE_MSG		= 'sdev',
			SET_DEVICE_BY_NAME_MSG		= 'sdvn',
			DEVICE_LIST_MSG		= 'devl',
			SCAN_MSG 			= 'scan',
			UPDATED_IMAGE_MSG	= 'uimg',
			FORMAT_CHANGED_MSG	= 'parc',
			ACCEPT_MSG			= 'accp',
			PARAM_CHANGED_MSG	= 'prch'
		};
				
		// Virtual function overrides
virtual void	MessageReceived(BMessage * msg);
virtual	bool	QuitRequested(void);

		thread_id				DevicesRosterThreadID() { return m_devices_roster_thread_id; };
		
		// From here, it's none of your business! ;-)
	private:
		status_t				SetDevice(BMessage *msg);
		status_t				SetImage(BBitmap * bitmap);
		status_t	 			BuildControls();
		status_t				AddDeviceInfoBox();

		int32					DevicesRosterThread();
		void					RescanDevices();
		int32					ScanThread();

		static	int32			_DevicesRosterThread(void * p) { return ((ScannerWindow *) p)->DevicesRosterThread(); }
		thread_id				m_devices_roster_thread_id;

		static	int32			_ScanThread(void * p) { return ((ScannerWindow *) p)->ScanThread(); }
		thread_id				m_scan_thread_id;
		volatile bool			m_cancel_scan;

		SANE_Handle				m_device;
		const SANE_Device *		m_device_info;

		BBox *					m_panel;
		BRect 					m_options_rect;
		PreviewView *			m_preview_view;
		BitmapView *			m_bitmap_view;
		BBitmap *				m_image;
		BPopUpMenu * 			m_preset_menu;
		BPopUpMenu * 			m_devices_menu;
		BStatusBar *			m_status_bar;
		BButton *				m_scan_button;
		BButton *				m_save_as_button;
		BButton *				m_close_button;
		BButton *				m_accept_button;
		BScrollView *			m_options_scroller;
		StackView *				m_options_stack;

		SANE_Int				m_preview_resolution;

		ScannerOptionView *		m_tl_x;
		ScannerOptionView *		m_tl_y;
		ScannerOptionView *		m_br_x;
		ScannerOptionView *		m_br_y;

		TranslatorSavePanel *	m_save_panel;

		// SANETranslator specific
		bool					m_standalone;
		BBitmap **				m_translator_out_bitmap;
};

#endif // ifdef SCANNERWINDOW_H

