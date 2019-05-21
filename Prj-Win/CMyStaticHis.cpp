#include "CMyStaticHis.h"



extern int gLiving;

CMyStaticHis::CMyStaticHis() :mLiving(0) {
	brush.CreateSolidBrush(RGB(255, 255, 255));
}


CMyStaticHis::~CMyStaticHis() {
}
BEGIN_MESSAGE_MAP(CMyStaticHis, CStatic)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CMyStaticHis::OnSize(UINT nType, int cx, int cy) {
	CStatic::OnSize(nType, cx, cy);
	Invalidate();
}


BOOL CMyStaticHis::OnEraseBkgnd(CDC* pDC) {
	return FALSE;
	return CStatic::OnEraseBkgnd(pDC);
}

void CMyStaticHis::OnPaint() {
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
	CRect rc;
	GetClientRect(rc);
	dc.FillSolidRect(rc, RGB(0, 0, 0));
	CString strMsg = "";

	if (mLiving == 0) {
		strMsg = "视频正在缓冲中...请稍后...";
	}
	else if (mLiving == 1) {
		strMsg = "视频正在缓冲中...请稍后...";
	}
	else if (mLiving == 2) {
		strMsg = "视频播放结束！";
	}
	else if (mLiving == 3) {
		strMsg = "视频正在缓冲中...请稍后...";
	}
	else if (mLiving == 4) {
		strMsg = "视频正在缓冲中...请稍后...";
	}
	else if (mLiving == 99) {
		strMsg = "直播缓冲中，精彩马上呈现...";
	}
	COLORREF clr = dc.SetTextColor(RGB(255, 0, 0));
	dc.DrawText(strMsg, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	dc.SetTextColor(clr);

}


void CMyStaticHis::SetLiving(int living) {
	mLiving = living;
	Invalidate();
}


int CMyStaticHis::GetLiving() {
	return mLiving;
}
