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
	: CDialogEx(IDD_DLG_PLAYERBAR, pParent), mmousecap(0)
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
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
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
	Invalidate();
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
	this->mfont.CreatePointFont(120, L"微软雅黑");
	CDC* pDC = GetDC();
	CFont *of = pDC->SelectObject(&mfont);
	pDC->DrawText(L"00:00:00", mtimerc, DT_SINGLELINE | DT_CALCRECT);
	pDC->SelectObject(of);
	ReleaseDC(pDC);

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
			bi->mtype = BarInfo::TwoType::ToSoundMute;
		}
		else {
			bi->mtype = BarInfo::TwoType::ToSound;
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
	CString strTime;
	strTime.Format(L"%02d:%02d:%02d",mcurTime/3600, mcurTime%3600/60,mcurTime%3600%60);
	CFont *of = dc.SelectObject(&mfont);
	dc.DrawText(strTime, mcurtimerc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	strTime.Format(L"%02d:%02d:%02d", (uint32_t)mtotalTime / 3600, (uint32_t)mtotalTime % 3600 / 60, (uint32_t)mtotalTime % 3600 % 60);
	dc.DrawText(strTime, mtotaltimerc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	dc.SelectObject(of);
	CPoint pt(3, 3);
	dc.RoundRect(mseekbarrc, pt);

	mseekbarbtnrc.top = mseekbarrc.top - 6;
	mseekbarbtnrc.bottom = mseekbarrc.bottom + 6;
	int sh = mseekbarbtnrc.Height();
	double pos = (double)mseekbarrc.Width()*(double)mcurTime / mtotalTime;
	mseekbarbtnrc.left = mseekbarrc.left + pos - 6;
	mseekbarbtnrc.right = mseekbarbtnrc.left + sh;
	pt.x = 10;
	pt.y = 10;
	if (mmousecap == 1) {
		mseekbarbtnrc.left = mmousepos  - 6;
		mseekbarbtnrc.right = mseekbarbtnrc.left + sh;
	}
	dc.RoundRect(mseekbarbtnrc, pt);
	rc.top = mseekbarrc.top + 3;
	rc.bottom = mseekbarrc.bottom - 3;
	rc.left = mseekbarrc.left + 2;
	rc.right = mseekbarbtnrc.left - 1;

	if(rc.right > rc.left)
		dc.FillSolidRect(rc, RGB(0,0,255));
	
}


void CDlgPlayerBar::OnSize(UINT nType, int cx, int cy){
	CDialogEx::OnSize(nType, cx, cy);
	CRect rc;
	GetClientRect(rc);
	int barsize = 30;
	int span = 16;
	int leftspan = 22;
	int rightspan = 22;
	int left=0, right=0;
	std::list<BarInfo*>::iterator iter;
	for (iter = mbarlist.begin(); iter != mbarlist.end(); iter++) {
		switch ((*iter)->mbartype) {
		case BarInfo::BarType::BTWindow:
			(*iter)->mpos.left = leftspan;
			(*iter)->mpos.right = (*iter)->mpos.left + barsize;
			(*iter)->mpos.top = rc.top + (rc.Height() - barsize) / 2;
			(*iter)->mpos.bottom = (*iter)->mpos.top + barsize;
			left = (*iter)->mpos.right;
			break;
		case BarInfo::BarType::BTSound:
			(*iter)->mpos.right = rc.Width() - rightspan;
			(*iter)->mpos.left = (*iter)->mpos.right - barsize;
			(*iter)->mpos.top = rc.top + (rc.Height() - barsize) / 2;
			(*iter)->mpos.bottom = (*iter)->mpos.top + barsize;
			right = (*iter)->mpos.left;
			break;
		default:
			break;
		}
	}

	mcurtimerc.left = left + span;
	mcurtimerc.right = mcurtimerc.left + mtimerc.Width();
	mcurtimerc.top = rc.top + (rc.Height() - mtimerc.Height()) / 2;
	mcurtimerc.bottom = mcurtimerc.top + mtimerc.Height();
	left = mcurtimerc.right;
	mtotaltimerc.right = right - span;
	mtotaltimerc.left = mtotaltimerc.right - mtimerc.Width();
	mtotaltimerc.top = rc.top + (rc.Height() - mtimerc.Height()) / 2;
	mtotaltimerc.bottom = mtotaltimerc.top + mtimerc.Height();
	right = mtotaltimerc.left;

	int seekbarh = 12;
	mseekbarrc.left = left + span;
	mseekbarrc.right = right - span;
	mseekbarrc.top = rc.top + (rc.Height() - seekbarh) / 2;
	mseekbarrc.bottom = mseekbarrc.top + seekbarh;
	if (mtotalTime < 1) {
		mtotalTime = 1;
	}
	mseekbarbtnrc.top = mseekbarrc.top - 6;
	mseekbarbtnrc.bottom = mseekbarrc.bottom + 6;
	int sh = mseekbarbtnrc.Height();
	double pos = (double)mseekbarrc.Width()*(double)mcurTime / mtotalTime;
	mseekbarbtnrc.left = mseekbarrc.left + pos - 6;
	mseekbarbtnrc.right = mseekbarbtnrc.left + sh;
	
	Invalidate();
}


void CDlgPlayerBar::OnLButtonUp(UINT nFlags, CPoint point){
	CDialogEx::OnLButtonUp(nFlags, point);
	if (mmousecap == 1) {
		mmousecap = 0;
		::ReleaseCapture();
		return;
	}
	std::list<BarInfo*>::iterator iter;
	for (iter = mbarlist.begin(); iter != mbarlist.end(); iter++) {
		(*iter)->LButtonDown(point);
	}
}


void CDlgPlayerBar::OnLButtonDown(UINT nFlags, CPoint point){
	CDialogEx::OnLButtonDown(nFlags, point);
	mmousecap = 1;
	::SetCapture(GetSafeHwnd());
}


void CDlgPlayerBar::OnMouseMove(UINT nFlags, CPoint point){
	if (mmousecap == 1) {
		mmousepos = point.x;
		if (mmousepos > mseekbarrc.right)
			mmousepos = mseekbarrc.right;
		if (mmousepos < mseekbarrc.left)
			mmousepos = mseekbarrc.left;

		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}
