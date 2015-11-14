#include "PreviewView.h"
#include "CheckerBitmap.h"


PreviewView::PreviewView(BRect frame, BRect geometry)
	: BView(frame, "PreviewView", B_FOLLOW_ALL_SIDES,
	B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	m_geometry 		= geometry;
	
	m_image 		= NULL;
	m_background 	= new CheckerBitmap(12);
	
	SetViewColor(B_TRANSPARENT_COLOR);
	m_background_color = ui_color(B_PANEL_BACKGROUND_COLOR);
}


PreviewView::~PreviewView()
{
	delete m_background;
}


void PreviewView::AttachedToWindow(void)
{
	if (Parent())
		m_background_color = Parent()->ViewColor();
}


void PreviewView::FrameResized(float width, float height)
{
}


BRect PreviewView::CenterImage()
{
	BRect rect(m_geometry);  // m_bitmap->Bounds());
	float width, height;
	width = Bounds().Width();
	height = Bounds().Height();

	if (width == 0 || height == 0)
		return rect;

	float ratio = width / (rect.Width()+1.0);
	if (ratio * (rect.Height()+1.0) <= height) {
		rect.right = width-1;
		rect.bottom = (int) (ratio * (rect.Height()+1.0))-1;
		// center vertically
		rect.OffsetBy(0, (int) ((height - rect.Height()) / 2));
	} else {
		ratio = height / (rect.Height()+1.0);
		rect.right = (int)  (ratio * (rect.Width()+1.0))-1;
		rect.bottom = height-1;
		// center horizontally
		rect.OffsetBy((int) ((width - rect.Width()) / 2), 0);
	}

	return rect;  // Bounds().InsetByCopy(2, 2);
}

void PreviewView::Draw(BRect invalid)
{
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	BRect r = CenterImage();
	
	SetLowColor(m_background_color);

	DrawBorders(r);
	
	rgb_color darker1 = tint_color(m_background_color, B_DARKEN_1_TINT);
	rgb_color darker2 = tint_color(m_background_color, B_DARKEN_3_TINT);
	
	SetHighColor(darker2);
	StrokeLine(BPoint(r.left + 1, r.bottom + 1), BPoint(r.right + 1, r.bottom + 1));
	StrokeLine(BPoint(r.right + 1, r.bottom + 1), BPoint(r.right + 1, r.top + 1));
	SetHighColor(darker1);
	StrokeLine(BPoint(r.left + 2, r.bottom + 2), BPoint(r.right + 2, r.bottom + 2));
	StrokeLine(BPoint(r.right + 2, r.bottom + 2), BPoint(r.right + 2, r.top + 2));

	SetHighColor(0, 0, 0, 255);
	StrokeRect(r);
	r.InsetBy(1, 1);

	if (m_image)
		DrawBitmap(m_image, m_image->Bounds(), r, B_FILTER_BITMAP_BILINEAR);
	else {
		SetHighColor(255, 255, 255, 255);
		FillRect(r);
	}
	

#if 0	
	if ( Window()->Lock() )
		{
		if ( m_image )
			{
			switch ( m_image->ColorSpace() )
				{
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
			};

		Window()->Unlock();
		};
#endif	
}


void PreviewView::SetBitmap(BBitmap * bm)
{
	m_image = bm;
	Invalidate();
}


inline BBitmap *	PreviewView::GetBitmap()
{
	return m_image;
}


void PreviewView::SetGeometry(BRect geometry)
{
	if (geometry == m_geometry)
		return;
	
	m_geometry = geometry;
	Invalidate();
}


void PreviewView::DrawBorders(BRect border)
{
	BRect bounds(Bounds());
	// top
	FillRect(BRect(0, 0, bounds.right, border.top-1), B_SOLID_LOW);
	// left
	FillRect(BRect(0, border.top, border.left-1, border.bottom), B_SOLID_LOW);
	// right
	FillRect(BRect(border.right+1, border.top, bounds.right, border.bottom), B_SOLID_LOW);
	// bottom
	FillRect(BRect(0, border.bottom+1, bounds.right, bounds.bottom), B_SOLID_LOW);
}



