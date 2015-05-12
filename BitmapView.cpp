#include "BitmapView.h"
#include "CheckerBitmap.h"



BitmapView::BitmapView(BRect frame, BBitmap * bm)
	: BView(frame, "BitmapView",
			B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_FRAME_EVENTS)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	m_image 		= bm;
	m_background 	= NULL;
}


BitmapView::~BitmapView()
{
	delete m_background;
}


void BitmapView::AttachedToWindow(void)
{
	m_background = new CheckerBitmap(12);
	if ( m_background )
		SetViewBitmap(m_background);

	SetupScrollbars();
}


void BitmapView::FrameResized(float width, float height)
{
	SetupScrollbars();
}


void BitmapView::Draw(BRect invalid)
{
	if (! m_image)
		return;
	
	if (LockLooper()) {
		switch ( m_image->ColorSpace() ) {
		case B_RGBA32:
		case B_RGBA32_BIG:
		case B_RGBA15:
		case B_RGBA15_BIG:
			SetDrawingMode(B_OP_ALPHA);
			SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
			break;
					
		default:
			SetDrawingMode(B_OP_COPY);
			break;
		};
			
		DrawBitmap(m_image, invalid, invalid);
	
		UnlockLooper();	
	}
}


void BitmapView::SetBitmap(BBitmap * bm)
{
	m_image = bm;
	SetupScrollbars();
	Invalidate();
}


inline BBitmap *	BitmapView::GetBitmap()
{
	return m_image;
}


void BitmapView::SetupScrollbars(void)
{
	BRect 			bounds;
	BScrollBar *	sb;

	bounds = Bounds();
	float myPixelWidth = bounds.Width();
	float myPixelHeight = bounds.Height();
	float maxWidth = 1, maxHeight = 1;

	if( m_image != NULL  && Window()->Lock()) {
		// get max size of image
		maxWidth 	= m_image->Bounds().right;
		maxHeight 	= m_image->Bounds().bottom;
		Window()->Unlock();
	};
		
	float propW = myPixelWidth/maxWidth;
	float propH = myPixelHeight/maxHeight;
	
	float rangeW = maxWidth - myPixelWidth;
	float rangeH = maxHeight - myPixelHeight;

	if(rangeW < 0) rangeW = 0;
	if(rangeH < 0) rangeH = 0;

	if ((sb=ScrollBar(B_HORIZONTAL))!=NULL) {
		sb->SetProportion(propW);
		sb->SetRange(0,rangeW);
		// Steps are 1/8 visible window for small steps
		//   and 1/2 visible window for large steps
		sb->SetSteps(myPixelWidth / 8.0, myPixelWidth / 2.0);
	} 

	if ((sb=ScrollBar(B_VERTICAL))!=NULL) {
		sb->SetProportion(propH);
		sb->SetRange(0,rangeH);
		// Steps are 1/8 visible window for small steps
		//   and 1/2 visible window for large steps
		sb->SetSteps(myPixelHeight / 8.0, myPixelHeight / 2.0);
	}
}

