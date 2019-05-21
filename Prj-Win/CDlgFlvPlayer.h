#pragma once
#include "RtmpGuester.h"
#include "CMyStaticHis.h"
#include "DlgPlayerBar.h"

// CDlgFlvPlayer 对话框

class CDlgFlvPlayer : public CDialogEx, public RTMPGuesterEvent
{
	DECLARE_DYNAMIC(CDlgFlvPlayer)

public:
	CDlgFlvPlayer(CWnd* pParent,const std::string& info);   // 标准构造函数
	virtual ~CDlgFlvPlayer();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_FLVPLAYER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	void AdjPlayBarPos(LPRECT pRect);
private:
	BOOL mReady;
	std::string minfo;
	RTMPGuester		*m_pPlayer;
	int m_iUserId;
	CMyStaticHis	m_myStatic;
	CDlgPlayerBar	*m_playerBar;
	//char*				m_pAudioMarker;
	//short*				m_pAudioMarketOut;
	//volatile int		m_iAudioMarker;
	//volatile int		m_iAudioMarketStart[10];
	//volatile int		m_iAudioMarketId;
	//volatile int		m_iAudioMarketIdNew;
	//volatile time_t		m_iAudioMarketLast;
	int	m_nVideoWidth;
	int m_nVideoHeight;
	int cacheTime;
	int curBitrate;
	uint32_t curTime;
	double totalTime;
public:
	virtual BOOL OnInitDialog();

	//* For RTMPCGuesterEvent
	virtual void OnRtmplayerOK();
	virtual void OnRtmplayerStatus(int cacheTime_, int curBitrate_, uint32_t curTime_, double totalTime_);
	virtual void OnRtmplayerCache(int time);
	virtual void OnRtmplayerClosed(int errcode);
	virtual void OnRtmplayerPlayStart();
	virtual void OnRtmplayerPlayStop();
	virtual void OnRtmplayer1stVideo();
	virtual void OnRtmplayer1stAudio();
	virtual void OnRtmplayerConnectionFailed(int a);

	virtual void OnGetPcmData(const void * pcm, const int len, const int rate, const int channels) {

		short* p = (short*)pcm;
		return;
	}
	virtual void PostNcDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnPullDlgResize(WPARAM, LPARAM);
	afx_msg LRESULT OnPullDlgSeekto(WPARAM, LPARAM);
	afx_msg LRESULT OnDlgFlvPlayer_Play(WPARAM, LPARAM);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnWindowMaximizedChange(BOOL bIsMaximized);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
};
