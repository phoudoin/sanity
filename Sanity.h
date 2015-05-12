#ifndef SANITY_H
#define SANITY_H

#include <Application.h>
#include <Window.h>

#define SOFTWARE_EDITOR			"prds"
#define SOFTWARE_NAME			"Sanity"
#define SOFTWARE_VERSION_LABEL	"0.5a"

class Application : public BApplication 
{
	public:
		// Constructors, destructors, operators...
						Application();
						~Application();

		typedef BApplication inherited;
		
		// public constantes
		enum	{
			NEW_WINDOW_MSG		= 'neww',
			WINDOW_CLOSED_MSG	= 'wcls'
		};
				
	public:
		// Virtual function overrides
		virtual	void	MessageReceived(BMessage * msg);	
		virtual void	RefsReceived(BMessage * msg);
		virtual void	AboutRequested();
		// bool			QuitRequested(void);
		void			ReadyToRun(void);
		
		// From here, it's none of your business! ;-)
	private:
		BRect	window_rect;
		int		windows_count;
};

// open a BFile to the binary which code is pointed to by ptr 
extern status_t GetBinaryPath(char *path, void *ptr);

#endif // ifdef SANITY_H
