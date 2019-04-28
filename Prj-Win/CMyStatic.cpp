#include "CMyStatic.h"

extern int gLiving;

CMyStatic::CMyStatic(){
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
}


BOOL CMyStatic::OnEraseBkgnd(CDC* pDC){
	return TRUE;
	return CStatic::OnEraseBkgnd(pDC);
}

void CMyStatic::OnPaint(){
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
	CRect rc;
	GetClientRect(rc);
	dc.FillSolidRect(rc, RGB(0, 0, 0));
	
	if (gLiving == 0) {
		COLORREF clr = dc.SetTextColor(RGB(255, 0, 0));
		
		dc.DrawText("暂无直播,您可以点播下列直播录像。", rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		dc.SetTextColor(clr);
	}else {

	}
	
}
