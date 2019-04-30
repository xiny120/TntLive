// DlgPlayerBar.cpp: 实现文件
//

#include "stdafx.h"
#include "LiveWin32.h"
#include "DlgPlayerBar.h"
#include "afxdialogex.h"

#define IDT_HIDEME	1

// CDlgPlayerBar 对话框

void BarInfo::MouseMove(const CPoint& pt) {
	if (callback != NULL && mpos.PtInRect(pt)) {
		callback->BarEventHover(this);
	}
}

void BarInfo::LButtonDown(const CPoint& pt) {
	if (callback != NULL && mpos.PtInRect(pt)) {
		callback->BarEventPress(this);
	}
}

IMPLEMENT_DYNAMIC(CDlgPlayerBar, CDialogEx)

CDlgPlayerBar::CDlgPlayerBar(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_PLAYERBAR, pParent)
{

}

CDlgPlayerBar::~CDlgPlayerBar()
{
}

void CDlgPlayerBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPlayerBar, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPlayerBar::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPlayerBar::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CDlgPlayerBar 消息处理程序


void CDlgPlayerBar::OnBnClickedOk(){
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CDlgPlayerBar::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
}


void CDlgPlayerBar::OnTimer(UINT_PTR nIDEvent){
	CPoint pt;
	CRect rc;
	switch (nIDEvent)
	{
	case IDT_HIDEME:
		GetClientRect(rc);
		ClientToScreen(rc);
		GetCursorPos(&pt);
		if (IsWindowVisible()) {
			if (rc.PtInRect(pt)) {
				mLastActive = time(NULL);
			}
			else {
				if ((time(NULL) - mLastActive) > 5) {
					ShowWindow(SW_HIDE);
				}
			}
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgPlayerBar::OnShowWindow(BOOL bShow, UINT nStatus){
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow) {
		mLastActive = time(NULL);
	}
}


BOOL CDlgPlayerBar::OnInitDialog(){
	CDialogEx::OnInitDialog();
	mLastActive = time(NULL);
	SetTimer(IDT_HIDEME, 500, NULL);
	BarInfo *bi = new BarInfo();
	bi->mbmp[BarInfo::TwoType::ToNormal][BarInfo::Status::Normal].LoadBitmap(IDB_BMP_TONORMAL);
	bi->mbmp[BarInfo::TwoType::ToNormal][BarInfo::Status::Hover].LoadBitmap(IDB_BMP_TONORMAL);
	bi->mbmp[BarInfo::TwoType::ToNormal][BarInfo::Status::Press].LoadBitmap(IDB_BMP_TONORMAL);
	bi->mbmp[BarInfo::TwoType::ToFullscreen][BarInfo::Status::Normal].LoadBitmap(IDB_BMP_TOFULLSCREEN);
	bi->mbmp[BarInfo::TwoType::ToFullscreen][BarInfo::Status::Hover].LoadBitmap(IDB_BMP_TOFULLSCREEN);
	bi->mbmp[BarInfo::TwoType::ToFullscreen][BarInfo::Status::Press].LoadBitmap(IDB_BMP_TOFULLSCREEN);
	bi->callback = this;
	bi->mtype = BarInfo::TwoType::ToFullscreen;
	bi->mstatus = BarInfo::Status::Normal;
	bi->mvisable = TRUE;
	bi->mbartype = BarInfo::BarType::BTWindow;
	mbarlist.push_back(bi);
	bi = new BarInfo();
	bi->mbmp[BarInfo::TwoType::ToSoundMute][BarInfo::Status::Normal].LoadBitmap(IDB_BMP_SOUNDMUTE);
	bi->mbmp[BarInfo::TwoType::ToSoundMute][BarInfo::Status::Hover].LoadBitmap(IDB_BMP_SOUNDMUTE);
	bi->mbmp[BarInfo::TwoType::ToSoundMute][BarInfo::Status::Press].LoadBitmap(IDB_BMP_SOUNDMUTE);
	bi->mbmp[BarInfo::TwoType::ToSound][BarInfo::Status::Normal].LoadBitmap(IDB_BMP_SOUND);
	bi->mbmp[BarInfo::TwoType::ToSound][BarInfo::Status::Hover].LoadBitmap(IDB_BMP_SOUND);
	bi->mbmp[BarInfo::TwoType::ToSound][BarInfo::Status::Press].LoadBitmap(IDB_BMP_SOUND);
	bi->callback = this;
	bi->mtype = BarInfo::TwoType::ToSound;
	bi->mstatus = BarInfo::Status::Normal;
	bi->mvisable = TRUE;
	bi->mbartype = BarInfo::BarType::BTSound;
	mbarlist.push_back(bi);


	PostMessage(WM_SIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgPlayerBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return FALSE;
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDlgPlayerBar::BarEventHover(BarInfo*) {

}
void CDlgPlayerBar::BarEventPress(BarInfo* bi) {
	switch (bi->mbartype){
	case BarInfo::BarType::BTWindow: {
		if (bi->mtype == BarInfo::TwoType::ToNormal) {
			::ShowWindow(GetParent()->GetSafeHwnd(), SW_SHOWNORMAL);
			bi->mtype = BarInfo::TwoType::ToFullscreen;
		}
		else {
			::ShowWindow(GetParent()->GetSafeHwnd(), SW_SHOWMAXIMIZED);
			bi->mtype = BarInfo::TwoType::ToNormal;
		}
	}
	break;
	case BarInfo::BarType::BTSound:	{
		if (bi->mtype == BarInfo::TwoType::ToSound) {
			bi->mtype == BarInfo::TwoType::ToSoundMute;
		}
		else {
			bi->mtype == BarInfo::TwoType::ToSound;
		}
	}
	break;
	default:
		break;
	}
	Invalidate();
}

void DrawShadow(CPaintDC&dc,CRect rc) {
	CPoint pt(3, 3);
	COLORREF clr[3] = { RGB(160, 160, 160),RGB(180, 180, 180), RGB(200, 200, 200)};
	for (int i = 0; i < 1; i++) {
		CPen p0(PS_SOLID, 2, clr[i]);
		CBrush b0(clr[i]);
		CPen* op = dc.SelectObject(&p0);
		CBrush* ob = dc.SelectObject(&b0);
		dc.RoundRect(rc, pt);
		dc.SelectObject(ob);
		dc.SelectObject(op);
		rc.DeflateRect(2, 2, 2, 2);
	}

}

void CDlgPlayerBar::OnPaint(){
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	CDC bmp;
	bmp.CreateCompatibleDC(&dc);
	CRect rc;
	GetClientRect(rc);
	dc.DrawEdge(rc, EDGE_ETCHED, BF_RECT);
	rc.DeflateRect(2, 2, 2, 2);
	dc.FillSolidRect(rc, RGB(255, 255, 255));
	std::list<BarInfo*>::iterator iter;
	for (iter = mbarlist.begin(); iter != mbarlist.end(); iter++) {
		BarInfo* bi = (*iter);
		CBitmap* old = bmp.SelectObject(bi->GetBitmap());
		BITMAP bm;
		bi->GetBitmap()->GetBitmap(&bm);
		int mode = dc.SetStretchBltMode(HALFTONE);
		::SetBrushOrgEx(dc.GetSafeHdc(), 0, 0, NULL);
		dc.StretchBlt(bi->mpos.left, bi->mpos.top, bi->mpos.Width(), bi->mpos.Height(), &bmp, 0, 0,bm.bmWidth,bm.bmHeight, SRCCOPY);
		dc.SetStretchBltMode( mode);
		::SetBrushOrgEx(dc.GetSafeHdc(), 0, 0, NULL);
		bmp.SelectObject(old);
	}


	COLORREF clr = dc.SetTextColor(RGB(255, 0, 0));
	dc.DrawText(L"HELLO", rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	dc.SetTextColor(clr);
	
}


void CDlgPlayerBar::OnSize(UINT nType, int cx, int cy){
	CDialogEx::OnSize(nType, cx, cy);
	CRect rc;
	GetClientRect(rc);
	int barsize = 30;
	std::list<BarInfo*>::iterator iter;
	for (iter = mbarlist.begin(); iter != mbarlist.end(); iter++) {
		switch ((*iter)->mbartype) {
		case BarInfo::BarType::BTWindow:
			(*iter)->mpos.left = 60;
			(*iter)->mpos.right = (*iter)->mpos.left + barsize;
			(*iter)->mpos.top = rc.top + (rc.Height() - barsize) / 2;
			(*iter)->mpos.bottom = (*iter)->mpos.top + barsize;
			break;
		case BarInfo::BarType::BTSound:
			(*iter)->mpos.right = rc.Width() - 60;
			(*iter)->mpos.left = (*iter)->mpos.right - barsize;
			(*iter)->mpos.top = rc.top + (rc.Height() - barsize) / 2;
			(*iter)->mpos.bottom = (*iter)->mpos.top + barsize;
			break;
		default:
			break;
		}
	}
	Invalidate();
}


void CDlgPlayerBar::OnLButtonUp(UINT nFlags, CPoint point){
	CDialogEx::OnLButtonUp(nFlags, point);
	std::list<BarInfo*>::iterator iter;
	for (iter = mbarlist.begin(); iter != mbarlist.end(); iter++) {
		(*iter)->LButtonDown(point);
	}
}
