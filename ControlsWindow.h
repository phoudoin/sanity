#ifndef CONTROLSWINDOW_H
#define CONTROLSWINDOW_H

#include <InterfaceKit.h>

#include "ScannerWindow.h"

class ControlsWindow : public BWindow  
{
	public:
		// Constructors, destructors, operators...
		ControlsWindow(ScannerWindow * parent_window);
		~ControlsWindow();
							
		typedef BWindow 		inherited;

		// public constantes
		enum	{
			SET_DEVICE_MSG		= 'sdev',
			DEVICE_LIST_MSG		= 'devl',
			SCAN_MSG 			= 'scan'
		};

		// Virtual function overrides
	public:	
		virtual void			MessageReceived(BMessage * msg);
		virtual	bool			QuitRequested(void);

		virtual int32			DevicesRosterThread();
		virtual int32			ScanThread();

		SANE_Handle				Device() 						{ return m_device; };
		
		// From here, it's none of your business! ;-)
	private:
		static	int32			_DevicesRosterThread(void * p) { return ((ControlsWindow *) p)->DevicesRosterThread(); }
		thread_id				m_devices_roster_thread_id;

		static	int32			_ScanThread(void * p) { return ((ControlsWindow *) p)->ScanThread(); }
		thread_id				m_scan_thread_id;
		volatile bool			m_cancel_scan;

		SANE_Handle				m_device;
		const SANE_Device *		m_device_info;

		ScannerWindow *			m_parent_window;
		
		BBox *					m_panel;
		BMenuField * 			m_devices_field;
		BPopUpMenu * 			m_devices_menu;
		BStatusBar *			m_status_bar;
		BButton *				m_scan_button;
		BScrollView *			m_options_sv;
		BListView *				m_options_lv;
};

#endif // ifdef CONTROLSWINDOW_H

