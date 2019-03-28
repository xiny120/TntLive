#include "CMyStatic.h"



CMyStatic::CMyStatic()
{
	brush.CreateSolidBrush(RGB(255, 255, 255));
}


CMyStatic::~CMyStatic()
{
}
BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CMyStatic::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}


BOOL CMyStatic::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	/*
	CBrush brush(RGB(255, 255, 255));
	CBrush *pOldBrush = pDC->SelectObject(&brush);

	CRect rect;
	GetClientRect(&rect);

	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	*/

	return TRUE;


	return CStatic::OnEraseBkgnd(pDC);
}


HBRUSH CMyStatic::CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/)
{
	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果不应调用父级的处理程序，则返回非 null 画笔
	
	return (HBRUSH)brush.GetSafeHandle();
	return NULL;
}


void CMyStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
	CRect rc;
	GetClientRect(rc);

	CPen p;
	p.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	dc.MoveTo(0, 0);
	dc.LineTo(100, 100);
	dc.FillSolidRect(rc, RGB(0, 0, 0));
}
