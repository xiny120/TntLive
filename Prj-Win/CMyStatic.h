#pragma once
#include <afxwin.h>
class CMyStatic :
	public CStatic
{
public:
	CBrush brush;
	CMyStatic();
	~CMyStatic();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};

