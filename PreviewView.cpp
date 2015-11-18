#include "PreviewView.h"
#include "CheckerBitmap.h"
#include "ScannerWindow.h"


PreviewView::PreviewView(BRect geometry)
	: BView("PreviewView", B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	m_geometry 		= geometry;
	m_frame			= geometry;
	m_image_frame	= geometry;
	m_mouse_button	= false;
	
	m_image 		= NULL;
	m_background 	= new CheckerBitmap(12);
	
	m_move_cursor = new BCursor(B_CURSOR_ID_CROSS_HAIR);

	SetViewColor(B_TRANSPARENT_COLOR);
	m_background_color = ui_color(B_PANEL_BACKGROUND_COLOR);
}


PreviewView::~PreviewView()
{
	delete m_background;
	delete m_move_cursor;
}


void PreviewView::AttachedToWindow(void)
{
	if (Parent())
		m_background_color = Parent()->ViewColor();
}


void PreviewView::FrameResized(float width, float height)
{
}


void PreviewView::MouseDown(BPoint p)
{
	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);
	if ( buttons & B_PRIMARY_MOUSE_BUTTON ) {
		m_mouse_point_1 = p;
		m_mouse_button = true;
		be_app->SetCursor(m_move_cursor, true);
		SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY);
	}
}


void PreviewView::MouseUp(BPoint p)
{
	BPoint point;
	uint32 buttons;
	GetMouse(&point, &buttons);
	if ( !buttons & B_PRIMARY_MOUSE_BUTTON ) {
		m_mouse_button = false;
		be_app->SetCursor(B_CURSOR_SYSTEM_DEFAULT, true);
		if ( m_mouse_point_1 == p ) {
			BMessage *msg = new BMessage(ScannerWindow::PARAM_CHANGED_MSG);
			msg->AddRect("rect", m_geometry);
			Window()->PostMessage(msg);
		}
	}
}


void PreviewView::MouseMoved(BPoint p, uint32 transit,const BMessage *message)
{
	if (m_mouse_button) {
		BRect r = CenterImage();

		float kx = r.Width() / m_geometry.Width();
		float ky = r.Height() / m_geometry.Height();

		BRect new_frame;
		new_frame.left = (m_mouse_point_1.x - r.left) / kx;
		new_frame.top = (m_mouse_point_1.y - r.top) / ky;
		new_frame.right = (p.x - r.left) / kx;
		new_frame.bottom = (p.y - r.top) / ky;

		BMessage *msg = new BMessage(ScannerWindow::PARAM_CHANGED_MSG);
		msg->AddRect("rect", new_frame);
		Window()->PostMessage(msg);
	}
}


BRect PreviewView::CenterImage()
{
	BRect rect(m_geometry);
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

	return rect;
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

	float kx = r.Width() / m_geometry.Width();
	float ky = r.Height() / m_geometry.Height();

	float eps = 1.0 * (m_geometry.Width() / 100.0);
	if ( !(fabs(m_frame.left - m_geometry.left) >= eps ||
			fabs(m_frame.top - m_geometry.top) >= eps ||
			fabs(m_frame.right - m_geometry.right) >= eps ||
			fabs(m_frame.bottom - m_geometry.bottom) >= eps) ) {
		m_frame = m_geometry;
	}

	SetHighColor(210, 210, 210, 255);
	FillRect(r);

	BRect rf(m_frame.left * kx, m_frame.top * ky, m_frame.right * kx, m_frame.bottom * ky);
	rf.OffsetBy(r.left, r.top);

	SetHighColor(255, 255, 255, 255);
	FillRect(rf);

	if (m_image) {
		BRect rif(m_image_frame.left * kx, m_image_frame.top * ky, m_image_frame.right * kx, m_image_frame.bottom * ky);
		rif.OffsetBy(r.left, r.top);
		DrawBitmap(m_image, m_image->Bounds(), rif, B_FILTER_BITMAP_BILINEAR);
	}

	if ( m_frame != m_geometry ) {
		SetHighColor(200, 200, 200, 176);
		FillRect(BRect(r.left, r.top, rf.left - 1, r.bottom));
		FillRect(BRect(rf.right + 1, r.top, r.right, r.bottom));
		FillRect(BRect(rf.left, r.top, rf.right, rf.top - 1));
		FillRect(BRect(rf.left, rf.bottom + 1, rf.right, r.bottom));

		SetHighColor(10, 40, 205, 196);
		StrokeRect(rf);

		float dotR = 3;
		FillEllipse(BPoint(rf.left, rf.top), dotR, dotR);
		FillEllipse(BPoint(rf.right - 1, rf.top), dotR, dotR);
		FillEllipse(BPoint(rf.left, rf.bottom - 1), dotR, dotR);
		FillEllipse(BPoint(rf.right - 1, rf.bottom - 1), dotR, dotR);
	}
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


void PreviewView::SetFrame(BRect frame)
{
	if (frame == m_frame)
		return;

	m_frame = frame;
	Invalidate();
}


void PreviewView::SetImageFrame(void)
{
	m_image_frame = m_frame;
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



