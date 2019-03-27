#pragma once
#include <afxwin.h>
class CMyStatic :
	public CStatic
{
public:
	CMyStatic();
	~CMyStatic();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

