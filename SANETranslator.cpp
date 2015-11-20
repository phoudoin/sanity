/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	Copyright 2005, yellowTAB GmbH.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

/*	Implements a wrapper translation kit addon to let any app	*/
/*	that can read images scan using Sanity.						*/

#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <ByteOrder.h>
#include <Message.h>
#include <Screen.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
#include <File.h>
#include <NodeInfo.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Bitmap.h>
#include <String.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "Sanity.h"
#include "ScannerWindow.h"

#if DEBUG
 #define dprintf(x) printf x
#else
 #define dprintf(x)
#endif


/* These three data items are exported by every translator. */
char translatorName[] = "SanityTranslator";
char translatorInfo[] = "Allows applications to use scanner devices through the translation kit.";
int32 translatorVersion = 100; /* format is revision+minor*10+major*100 */

extern const char * STR_ABOUT_TEXT;

/*	yT reserves all codes with non-lowecase letters in them.	*/
/*	Sanity is a frontend for SANE...	*/
#define SANE_TYPE 'SANE'
#define SANE_PRETTY "Scanner Device"


/* These two data arrays are a really good idea to export from Translators, but not required. */
translation_format inputFormats[] = {
/*	{	B_TRANSLATOR_BITMAP, B_TRANSLATOR_BITMAP, 0.4, 0.8, "image/x-be-bitmap", "Be Bitmap Format (SANEHandler)" },*/
/*	{	PPM_TYPE, B_TRANSLATOR_BITMAP, 0.3, 0.8, "image/x-ppm", "PPM portable pixmap format" },*/
	{	SANE_TYPE, B_TRANSLATOR_BITMAP, 0.9, 0.9, SANE_MIMETYPE, SANE_PRETTY },
	{	0, 0, 0, 0, "\0", "\0" }
};		/*	optional (else Identify is always called)	*/

translation_format outputFormats[] = {
	{	B_TRANSLATOR_BITMAP, B_TRANSLATOR_BITMAP, 0.4, 0.8, "image/x-be-bitmap", "Be Bitmap Format (SANEHandler)" },
/*	{	PPM_TYPE, B_TRANSLATOR_BITMAP, 0.3, 0.8, "image/x-ppm", "PPM portable pixmap format" },*/
	{	0, 0, 0, 0, "\0", "\0" }
};	/*	optional (else Translate is called anyway)	*/

/*	Translators that don't export outputFormats 	*/
/*	will not be considered by files looking for 	*/
/*	specific output formats.	*/


static status_t check_posio(BPositionIO *inSource, BString *device=NULL)
{
	BFile *file;
	/* make sure we are handed a file with the correct type*/
	file = dynamic_cast<BFile *>(inSource);
	if (file == NULL)
		return EINVAL;
	if (file->InitCheck())
		return file->InitCheck();
	BNodeInfo finfo(file);
	if (finfo.InitCheck())
		return finfo.InitCheck();
	char fmime[256];
	if (finfo.GetType(fmime) < B_OK)
		return EINVAL;
	if (strcmp(fmime, SANE_MIMETYPE))
		return EINVAL;
	if (device) {
		file->ReadAttrString("sane-device", device);
	}
	return B_OK;
}

	/*	Return B_NO_TRANSLATOR if not handling this data.	*/
	/*	Even if inputFormats exists, may be called for data without hints	*/
	/*	If outType is not 0, must be able to export in wanted format	*/

status_t
Identify(	/*	required	*/
	BPositionIO * inSource,
	const translation_format * inFormat,	/*	can beNULL	*/
	BMessage * ioExtension,	/*	can be NULL	*/
	translator_info * outInfo,
	uint32 outType)
{
	status_t err;
	dprintf(("SANETranslator: Identify()\n"));
	
	//sanity_locale_init();
	
	/* Silence compiler warnings. */
	inFormat = inFormat;
	ioExtension = ioExtension;

	/* Check that requested format is something we can deal with. */
	if (outType == 0) {
		outType = B_TRANSLATOR_BITMAP;
	}
	if (outType != B_TRANSLATOR_BITMAP) {
		return B_NO_TRANSLATOR;
	}
	
	err = check_posio(inSource);
	if (err)
		return err;
	dprintf(("SANETranslator: Identify: mime accepted\n"));
	if (ioExtension)
		ioExtension->PrintToStream();
	
	if (inFormat) {
		dprintf(("SANETranslator: Identify: infmt: %08lx, %08lx, %f, %f, %s, %s\n", inFormat->type, inFormat->group, inFormat->quality, inFormat->capability, inFormat->MIME, inFormat->name));
	}
	
	/* Stuff info into info struct -- Translation Kit will do "translator" for us. */
	outInfo->group = B_TRANSLATOR_BITMAP;
	outInfo->type = SANE_TYPE;
	outInfo->quality = 0.3;		/* no alpha, etc */
	outInfo->capability = 0.8;	/* we're pretty good at SANE reading, though */
	strcpy(outInfo->name, SANE_PRETTY);
	strcpy(outInfo->MIME, SANE_MIMETYPE);

	dprintf(("SANETranslator: Identify: B_OK\n"));
	return B_OK;
}


	/*	Return B_NO_TRANSLATOR if not handling the output format	*/
	/*	If outputFormats exists, will only be called for those formats	*/

status_t
Translate(	/*	required	*/
	BPositionIO * inSource,
	const translator_info * /* inInfo*/ ,	/* silence compiler warning */
	BMessage * ioExtension,	/*	can be NULL	*/
	uint32 outType,
	BPositionIO * outDestination)
{
	ScannerWindow * window;
	BString sane_device;
	status_t err;
	dprintf(("SANETranslator: Translate()\n"));

	err = check_posio(inSource, &sane_device);
	if (err)
		return err;
	
	inSource->Seek(0, SEEK_SET);	/* paranoia */
//	inInfo = inInfo;	/* silence compiler warning */
	/* Check what we're being asked to produce. */

	if (!outType) {
		outType = B_TRANSLATOR_BITMAP;
	}
	dprintf(("SANETranslator: outType is '%c%c%c%c'\n", char(outType>>24), char(outType>>16), char(outType>>8), char(outType)));
	if (outType != B_TRANSLATOR_BITMAP) {
		return B_NO_TRANSLATOR;
	}

	/* XXX: make that threadsafe */
	dprintf(("SANETranslator: sane_init\n"));
	sane_init(0, NULL);

	dprintf(("SANETranslator: new Window\n"));
	BRect window_rect(50, 50, 780, 580);
	BBitmap *outBitmap = NULL;
	window = new ScannerWindow(window_rect, &outBitmap);
	if (sane_device.Length()) {
		status_t ret;
		/* wait for the scanner list to be built before selecting one of them */
		wait_for_thread(window->DevicesRosterThreadID(), &ret);
		dprintf(("SANETranslator: using device '%s'\n", sane_device.String()));
		BMessenger msgr(window);
		BMessage setDevMsg(ScannerWindow::SET_DEVICE_BY_NAME_MSG);
		setDevMsg.AddString("device", sane_device.String());
		msgr.SendMessage(&setDevMsg);
	}
	window->Show();
	
	status_t wret = -1;
	thread_id wth = window->Thread();
	wait_for_thread(wth, &wret);

	dprintf(("SANETranslator: BBitmap @ %p\n", outBitmap));
	if (outBitmap) {
		BBitmapStream stream(outBitmap);
		
		
		char buffer[4096*32];
		ssize_t got;
		while ((got = stream.Read(buffer, 4096*32)) > 0) {
			dprintf(("SANETranslator: got %d\n", got));
			outDestination->Write(buffer, got);
		}
		return B_OK;
	}
	

	/* XXX: make that threadsafe */
	//sane_exit();
	
	return -1;
}


class SANEView :
	public BView
{
public:
		SANEView(
				const BRect & frame,
				const char * name,
				uint32 resize,
				uint32 flags) :
			BView(frame, name, resize, flags)
			{
				//SetViewColor(220,220,220,0);
			}
		~SANEView()
			{
				/* nothing here */
			}

virtual	void Draw(
				BRect area)
			{
				area = area; /* silence compiler */
				PushState();
				SetFont(be_bold_font);
				font_height fh;
				GetFontHeight(&fh);
				char str[100];
				sprintf(str, "SANETranslator %.2f", (float)translatorVersion/100.0);
				DrawString(str, BPoint(10+fh.descent+1, 10+fh.ascent+fh.descent*2+fh.leading));
				PopState();
				//DrawString(STR_ABOUT_TEXT, BPoint(10+fh.descent+1, 30+fh.ascent+fh.descent*2+fh.leading));
			}

private:
};


	/*	The view will get resized to what the parent thinks is 	*/
	/*	reasonable. However, it will still receive MouseDowns etc.	*/
	/*	Your view should change settings in the translator immediately, 	*/
	/*	taking care not to change parameters for a translation that is 	*/
	/*	currently running. Typically, you'll have a global struct for 	*/
	/*	settings that is atomically copied into the translator function 	*/
	/*	as a local when translation starts.	*/
	/*	Store your settings wherever you feel like it.	*/

status_t 
MakeConfig(	/*	optional	*/
	BMessage * ioExtension,	/*	can be NULL	*/
	BView * * outView,
	BRect * outExtent)
{
	SANEView * v = new SANEView(BRect(0,0,200,100), "SANETranslator Settings", B_FOLLOW_ALL, B_WILL_DRAW);
	*outView = v;
	*outExtent = v->Bounds();
	return B_OK;
}


	/*	Copy your current settings to a BMessage that may be passed 	*/
	/*	to BTranslators::Translate at some later time when the user wants to 	*/
	/*	use whatever settings you're using right now.	*/

status_t
GetConfigMessage(	/*	optional	*/
	BMessage * ioExtension)
{
	status_t err = B_OK;
	return err;
}



