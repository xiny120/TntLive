#pragma once


// CDlgPlayerBar 对话框

class CDlgPlayerBar : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPlayerBar)

public:
	CDlgPlayerBar(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDlgPlayerBar();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_PLAYERBAR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	time_t	mLastActive;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
};
