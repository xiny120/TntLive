#include "CMyStatic.h"

extern int gLiving;

CMyStatic::CMyStatic():mLiving(0){
	brush.CreateSolidBrush(RGB(255, 255, 255));
}


CMyStatic::~CMyStatic(){
}
BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CMyStatic::OnSize(UINT nType, int cx, int cy){
	CStatic::OnSize(nType, cx, cy);
	Invalidate();
}


BOOL CMyStatic::OnEraseBkgnd(CDC* pDC){
	return FALSE;
	return CStatic::OnEraseBkgnd(pDC);
}

void CMyStatic::OnPaint(){
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
	CRect rc;
	GetClientRect(rc);
	dc.FillSolidRect(rc, RGB(0, 0, 0));
	CString strMsg = "";
	
	if (mLiving == 0) {
		strMsg = "正在连接直播服务器,请稍后...";
	}else if(mLiving == 1) {
		strMsg = "正在查询频道是否开始直播，请稍后...";
	}else if (mLiving == 2) {
		strMsg = "直播结束，您还可以收看往期精彩直播录像！";
	}else if (mLiving == 3) {
		strMsg = "目前暂无直播，您可以收看往期精彩直播录像！";
	}else if (mLiving == 4) {
		strMsg = "目前暂无直播，您可以收看往期精彩直播录像！";
	}else if (mLiving == 99) {
		strMsg = "直播缓冲中，精彩马上呈现...";
	}
	COLORREF clr = dc.SetTextColor(RGB(255, 0, 0));
	dc.DrawText(strMsg, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	dc.SetTextColor(clr);

}


void CMyStatic::SetLiving(int living){
	mLiving = living;
	Invalidate();
}


int CMyStatic::GetLiving(){
	return mLiving;
}
