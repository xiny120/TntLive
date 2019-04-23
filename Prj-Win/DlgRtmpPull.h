/*
*  Copyright (c) 2016 The AnyRTC project authors. All Rights Reserved.
*
*  Please visit https://www.anyrtc.io for detail.
*
* The GNU General Public License is a free, copyleft license for
* software and other kinds of works.
*
* The licenses for most software and other practical works are designed
* to take away your freedom to share and change the works.  By contrast,
* the GNU General Public License is intended to guarantee your freedom to
* share and change all versions of a program--to make sure it remains free
* software for all its users.  We, the Free Software Foundation, use the
* GNU General Public License for most of our software; it applies also to
* any other work released this way by its authors.  You can apply it to
* your programs, too.
* See the GNU LICENSE file for more info.
*/
#pragma once
#include "Resource.h"
#include "RtmpGuester.h"
#include "DlgVideo.h"
#include "CMyStatic.h"
#define WM_MY_PULL_MESSAGE (WM_USER + 102)

// DlgVideo 对话框
class CLiveWin32Dlg;
class DlgRtmpPull : public CDialog, public RTMPGuesterEvent
{
	DECLARE_DYNAMIC(DlgRtmpPull)

public:
	DlgRtmpPull();   // 标准构造函数
	virtual ~DlgRtmpPull();

// 对话框数据
	enum { IDD = IDD_DIALOG_PULL };
	CBrush brush;

public:
	//* For RTMPCGuesterEvent
	virtual void OnRtmplayerOK() {
		TRACE("OnRtmplayerOK");
	};
	virtual void OnRtmplayerStatus(int cacheTime, int curBitrate) {
	};
	virtual void OnRtmplayerCache(int time) {
	};
	virtual void OnRtmplayerClosed(int errcode) {
		TRACE("OnRtmplayerClosed");
	};

	virtual void OnGetPcmData(const void * pcm, const int len,const int rate, const int channels) {
		short* p = (short*)pcm;
		//return;
		
		if (m_pAudioMarketOut >= ((short*)m_pAudioMarker + m_iAudioMarker/2)) {
			m_pAudioMarketOut = (short*)m_pAudioMarker;
		}
		for (int i = 0; i < len/ channels; i++) {
			short val = *m_pAudioMarketOut++;
			int adj = p[i] * 0.61f + val;// ((p[i] * 0.7f) + val * 0.29f);
			if (adj > 32767) {
				//	adj = 32767;
				TRACE("adj > 32767\r\n");
			}	else if (adj < -32768) {
				TRACE("adj < -32768\r\n");
				//	adj = -32768;
			}
			p[i] = adj;
			i++;
			p[i] = adj;
		}
		//TRACE("len:%d samples:%d channels:%d\r\n",len, rate, channels);
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPullDlgResize(WPARAM, LPARAM);

	CMyStatic m_myStatic;
	CEdit	m_editUrl;
	CButton m_btnRtmp;
	CString m_strUrl;
	CefString m_strUri;
	//CStatic m_staticCaptrue;
	int	m_nVideoWidth;
	int m_nVideoHeight;
	int m_nChatroomWidth;
	int m_nListHeight;
	int m_iUserId;
	char*				m_pAudioMarker;
	short*				m_pAudioMarketOut;
	volatile int		m_iAudioMarker;
	volatile int		m_iAudioMarketStart[10];
	volatile int		m_iAudioMarketId;
	volatile int		m_iAudioMarketIdNew;
	volatile time_t		m_iAudioMarketLast;

	afx_msg void OnBnClickedBtnPull();

private:
	DlgVideo		*m_pDlgVideoMain;
	RTMPGuester		*m_pAVRtmplayer;
	
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	void Start();
	void Stop();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
