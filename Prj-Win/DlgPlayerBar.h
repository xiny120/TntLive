#pragma once

class CDlgPlayerBar;

class BarInfo {
public:
	enum BarType {
		BTWindow,
		BTSound,
	};
	enum TwoType {
		ToFullscreen = 1,
		ToNormal = 0,
		ToSoundMute = 1,
		ToSound = 0,
		ToEnd = 2,
	};
	enum Status {
		Normal = 0,
		Press,
		Hover,
		End
	};
	BarInfo():callback(NULL){

	}
	CBitmap* GetBitmap() {
		return &mbmp[mtype][mstatus];
	}
	void MouseMove(const CPoint& pt);
	void LButtonDown(const CPoint& pt);
	CRect mpos;
	CString mname;
	CBitmap mbmp[TwoType::ToEnd][Status::End];
	CDlgPlayerBar* callback;
	BOOL mvisable;
	TwoType mtype;
	BarType mbartype;
	Status mstatus;
};

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
public:
	uint32_t mcurTime;
	double mtotalTime;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	std::list<BarInfo*> mbarlist;
	CRect mcurtimerc,mtotaltimerc,mtimerc,mseekbarrc,mseekbarbtnrc;
	CFont mfont;
	int mmousepos;
	int mmousecap;
	HCURSOR mcursor;
	DECLARE_MESSAGE_MAP()
public:
	virtual void BarEventHover(BarInfo*);
	virtual void BarEventPress(BarInfo*);
	time_t	mLastActive;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
