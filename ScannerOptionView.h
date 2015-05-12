#ifndef SCANNEROPTIONVIEW_H
#define SCANNEROPTIONVIEW_H

#include <InterfaceKit.h>
#include <SupportKit.h>

#include "BeSANE.h"

class ScannerOptionView : public BView  
{
public:
		// Constructors, destructors, operators...
		ScannerOptionView(BRect frame, const char * name, uint32 resizeMask, uint32 flags, SANE_Handle device, int opt_number);
		~ScannerOptionView();
							
		typedef BView 			inherited;

		typedef enum {
			UNKNOWN = 0,
			GROUP,
			STRING,
			NUMERIC,
			NUMERIC_RANGE,
			NUMERIC_ARRAY,
			STRING_LIST,
			NUMERIC_LIST,
			COMMAND,
			BOOLEAN
		} field_type; 

		// public constantes
		enum	{
			BROWSE_FILE_MSG		= 'brwf',
			BROWSE_DIR_MSG		= 'brwd',
			SET_VALUE_MSG		= 'setv',
			RELOAD_MSG			= 'rloa'
		};

	// Virtual function overrides
	virtual void 				AttachedToWindow(void);
	virtual void				MessageReceived(BMessage * msg);
	virtual void 				MouseDown(BPoint where);
	virtual void 				Draw(BRect updateRect);



	virtual void				SetValue(BMessage *msg);
	virtual status_t			Build(void);
	virtual status_t			UpdateValue(void);

	virtual field_type			Type(void) 						{ return m_type; };
	virtual status_t			SetEnabled(bool on);
	virtual bool				IsEnabled(void) const			{ return m_enabled; };
	virtual status_t			SetAutoMode(bool on);
	virtual bool				IsAutoMode(void) const			{ return m_auto_mode; };
	SANE_Handle					Device(void) 					{ return m_device; };
	int							Option(void)					{ return m_opt; };
	
	float						FloatRangeMin();
	float						FloatRangeMax();
	int							RangeMin();
	int							RangeMax();
		
	const char *				UnitLabel(int *value);
	const char *				UnitLabel(float *value = 0);

	// From here, it's none of your business! ;-)
private:
		void						PrintToStream();
		void 						InitIcon();

		uint32						m_flags;
		
		SANE_Handle					m_device;
		int							m_opt;
		const SANE_Option_Descriptor *m_desc;
		bool						m_enabled;
		bool						m_auto_mode;
		field_type					m_type;
		int							m_nb_values;
		void *						m_last_value;
		BBitmap *					m_icon;
		

public:	
		static BList				m_options_list;
		static BLocker				m_options_list_locker;
};

#endif // ifdef SCANNEROPTIONVIEW_H

