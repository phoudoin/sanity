#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <Application.h>
#include <Window.h>
#include <Alert.h>
#include <TextView.h>
#include <String.h>

#include "Sanity.h"
#include "SanityStrings.h"
#include "ScannerWindow.h"

#include "BeSANE.h"

const char * APPLICATION_SIGNATURE = "application/x-vnd." SOFTWARE_EDITOR "-" SOFTWARE_NAME;

const char * STR_ABOUT_TITLE =	"About " SOFTWARE_NAME;
const char * STR_ABOUT_BUTTON =	"Hum?";
const char * STR_ABOUT_TEXT = \
	SOFTWARE_NAME " " SOFTWARE_VERSION_LABEL "\n" \
	"A little SANE front-end for BeOS.\n" \
	B_UTF8_COPYRIGHT " 2000-2004 Philippe Houdoin\n" \
	"\n" \
	"Thanks to:\n" \
	"\tChrystelle\n" \
	"\tSANE project\n" \
	"\tCarlos Hasan's flotsam library\n" \
	"\tBe Inc.\n" \
	"\tInternet\n" \
	"\tLife" B_UTF8_ELLIPSIS "\n";

static void authenticate(
				SANE_String_Const resource,
				SANE_Char username[SANE_MAX_USERNAME_LEN],
	       		SANE_Char password[SANE_MAX_PASSWORD_LEN]);

// --------------------------------------------------------------
int main()
{
	// Create an application instance
	Application * app;

	app = new Application();
	sanity_locale_init();
	app->Run();
	delete app;
	
	return 0;
}



// --------------------------------------------------------------
Application::Application()
	: BApplication(APPLICATION_SIGNATURE)
{
	window_rect.Set(50, 50, 800, 480);
	windows_count = 0;
	
	sane_init(0, authenticate);
}



// -------------------------------------------------
Application::~Application()
{
	sane_exit();

}


// -------------------------------------------------
void Application::ReadyToRun( void )
{
	// Open at least one window!
	if ( windows_count < 1 )
		PostMessage(NEW_WINDOW_MSG);
}
	
	
// --------------------------------------------------------------
void Application::MessageReceived
	(
	BMessage * pMsg
	)
{
	switch( pMsg->what ) 
		{
		case NEW_WINDOW_MSG:
			{
			ScannerWindow * window = new ScannerWindow(window_rect);
			window->Show();

			window_rect.OffsetBy(40, 40);
			windows_count++;
			break;
			};
			
		case WINDOW_CLOSED_MSG:
			{
			windows_count--;
			if ( windows_count < 1 )
				PostMessage(B_QUIT_REQUESTED);
			break;
			};
			
		default:
			inherited::MessageReceived( pMsg );
			break;
		};
}


// --------------------------------------------------------------
void Application::RefsReceived
	(
	BMessage *	msg
	) 
{ 
	switch (msg->what) 
		{
		default:
			inherited::MessageReceived(msg);
			break;
		};
}


// -------------------------------------------------
void Application::AboutRequested( void )
{
	BAlert * about_box;
	
	about_box = new BAlert(STR_ABOUT_TITLE, STR_ABOUT_TEXT, STR_ABOUT_BUTTON);
	BTextView *tv = about_box->TextView();
	if (tv) {
		// Colors grab with Magnify on BeOS R5 deskbar logo
		rgb_color red = {203, 0, 51, 255};		
		rgb_color blue = {0, 51, 152, 255};

		tv->SetStylable(true);
		char *text = (char*) tv->Text();
		char *s = text;

		// set first line 20pt bold
		s = strchr(text, '\n');
		BFont font(be_bold_font);
		font.SetSize(16);
		tv->SetFontAndColor(0, s-text, &font);

		// set all Be in blue and red
		s = text;
		while ((s = strstr(s, "BeOS")) != NULL) {
			int32 i = s - text;
			//tv->SetFontAndColor(i, i+2, NULL, 0, &blue);
			//tv->SetFontAndColor(i+2, i+4, NULL, 0, &red);
			tv->SetFontAndColor(i, i+1, NULL, 0, &blue);
			tv->SetFontAndColor(i+1, i+2, NULL, 0, &red);
			s += 2;
		}
	}

	about_box->Go();
}


// #pragma mark -

static void authenticate(SANE_String_Const resource,
	       SANE_Char username[SANE_MAX_USERNAME_LEN],
	       SANE_Char password[SANE_MAX_PASSWORD_LEN])
{
	BString text;
	
	text = "Authentification request:\n\n";
	text << "Resource: " << resource << "\n\n";
	text << "Feature not yet supported, sorry" B_UTF8_ELLIPSIS;

	BAlert *alert = new BAlert("Authentification request",
		text.String(), "Bummer!", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();


/*

  char tmp[3 + 128 + SANE_MAX_USERNAME_LEN + SANE_MAX_PASSWORD_LEN], *wipe;
  unsigned char md5digest[16];
  int md5mode = 0, len, query_user = 1;
  FILE *pass_file;
  struct stat stat_buf;

  *tmp = 0;

  if (getenv ("HOME") != NULL)
    {
      if (strlen (getenv ("HOME")) < 500)
	{
	  sprintf (tmp, "%s/.sane/pass", getenv ("HOME"));
	}
    }

  if ((strlen (tmp) > 0) && (stat (tmp, &stat_buf) == 0))
    {

      if ((stat_buf.st_mode & 63) != 0)
	{
	  fprintf (stderr, "%s has wrong permissions (use at least 0600)\n",
		   tmp);
	}
      else
	{

	  if ((pass_file = fopen (tmp, "r")) != NULL)
	    {

	      if (strstr (resource, "$MD5$") != NULL)
		len = (strstr (resource, "$MD5$") - resource);
	      else
		len = strlen (resource);

	      while (fgets (tmp, 512, pass_file))
		{

		  if ((strlen (tmp) > 0) && (tmp[strlen (tmp) - 1] == '\n'))
		    tmp[strlen (tmp) - 1] = 0;
		  if ((strlen (tmp) > 0) && (tmp[strlen (tmp) - 1] == '\r'))
		    tmp[strlen (tmp) - 1] = 0;

		  if (strchr (tmp, ':') != NULL)
		    {

		      if (strchr (strchr (tmp, ':') + 1, ':') != NULL)
			{

			  if (
			      (strncmp
			       (strchr (strchr (tmp, ':') + 1, ':') + 1,
				resource, len) == 0)
			      &&
			      ((int) strlen
			       (strchr (strchr (tmp, ':') + 1, ':') + 1) ==
			       len))
			    {

			      if ((strchr (tmp, ':') - tmp) <
				  SANE_MAX_USERNAME_LEN)
				{

				  if (
				      (strchr (strchr (tmp, ':') + 1, ':') -
				       (strchr (tmp, ':') + 1)) <
				      SANE_MAX_PASSWORD_LEN)
				    {

				      strncpy (username, tmp,
					       strchr (tmp, ':') - tmp);

				      username[strchr (tmp, ':') - tmp] = 0;

				      strncpy (password,
					       strchr (tmp, ':') + 1,
					       strchr (strchr (tmp, ':') + 1,
						       ':') -
					       (strchr (tmp, ':') + 1));
				      password[strchr
					       (strchr (tmp, ':') + 1,
						':') - (strchr (tmp,
								':') + 1)] =
					0;

				      query_user = 0;
				      break;
				    }
				}

			    }
			}
		    }
		}

	      fclose (pass_file);
	    }
	}
    }

  if (strstr (resource, "$MD5$") != NULL)
    {
      md5mode = 1;
      len = (strstr (resource, "$MD5$") - resource);
      if (query_user == 1)
	fprintf (stderr, "Authentification required for resource %*.*s. "
		 "Enter username: ", len, len, resource);
    }
  else
    {

      if (accept_only_md5_auth != 0)
	{
	  fprintf (stderr, "ERROR: backend requested plain-text password\n");
	  return;
	}
      else
	{
	  fprintf (stderr,
		   "WARNING: backend requested plain-text password\n");
	  query_user = 1;
	}

      if (query_user == 1)
	fprintf (stderr,
		 "Authentification required for resource %s. Enter username: ",
		 resource);
    }

  if (query_user == 1)
    fgets (username, SANE_MAX_USERNAME_LEN, stdin);

  if ((strlen (username)) && (username[strlen (username) - 1] == '\n'))
    username[strlen (username) - 1] = 0;

  if (query_user == 1)
    {
#ifndef __BEOS__
      strcpy (password, (wipe = getpass ("Enter password: ")));
#endif
      memset (wipe, 0, strlen (password));
    }

  if (md5mode)
    {

      sprintf (tmp, "%.128s%.*s", (strstr (resource, "$MD5$")) + 5,
	       SANE_MAX_PASSWORD_LEN - 1, password);

      md5_buffer (tmp, strlen (tmp), md5digest);

      memset (password, 0, SANE_MAX_PASSWORD_LEN);

      sprintf (password, "$MD5$%02x%02x%02x%02x%02x%02x%02x%02x"
	       "%02x%02x%02x%02x%02x%02x%02x%02x",
	       md5digest[0], md5digest[1],
	       md5digest[2], md5digest[3],
	       md5digest[4], md5digest[5],
	       md5digest[6], md5digest[7],
	       md5digest[8], md5digest[9],
	       md5digest[10], md5digest[11],
	       md5digest[12], md5digest[13], md5digest[14], md5digest[15]);
    }
*/
}

// get the path to the binary ptr points in to
// (can't use GetAppInfo from addons)

status_t GetBinaryPath(char *path, void *ptr)
{
	status_t status = B_ERROR;
	
	// we must find the path to the binary this code is from...
	// iterate through the the image list to find ourselves
	// asking the roster for the app mime might get the wrong path
	// if we are used as an addon
	image_info ai;
	int32 cookie = 0;
	while ( B_OK == get_next_image_info(0, &cookie, &ai) )
	{
		if (((ptr >= (char *)ai.text) && (ptr < (char *)ai.text + ai.text_size))
		 || ((ptr >= (char *)ai.data) && (ptr < (char *)ai.data + ai.data_size)))
		{
			strncpy(path, ai.name, B_PATH_NAME_LENGTH);
			path[B_PATH_NAME_LENGTH] = '\0';
			return B_OK;
		}
	}
	return B_ERROR;
}

