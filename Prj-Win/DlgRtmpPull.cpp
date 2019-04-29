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
#include "stdafx.h"
#include "include/cef_parser.h"
#include "LiveWin32.h"
#include "DlgRtmpPull.h"
#include "xdefines.h"
#include "CDlgFlvPlayer.h"

// DlgRtmpPull 对话框
HWND	m_hWndPullDlg;
extern int gUserId;
extern int gLiving;
extern std::string m_baseurl;

IMPLEMENT_DYNAMIC(DlgRtmpPull, CDialog)

DlgRtmpPull::DlgRtmpPull(const char* rtmpurl,const char* medialisturl)
	: CDialog(DlgRtmpPull::IDD)
	, m_strUrl(_T("rtmp://www.pic98.com/live/livestream"))
	, m_pAVRtmplayer(NULL)
//	, m_pDlgVideoMain(NULL)
	, m_nVideoWidth(720)
	, m_nVideoHeight(576)
	, m_nChatroomWidth(420)
	, m_nListHeight(220)
	, mrtmpurl(rtmpurl)
	, mmedialisturl(medialisturl)
	, m_pAudioMarker(NULL){
}

DlgRtmpPull::~DlgRtmpPull(){
}

void DlgRtmpPull::DoDataExchange(CDataExchange* pDX){
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_URL, m_editUrl);
	DDX_Control(pDX, IDC_BTN_PULL, m_btnRtmp);
	DDX_Text(pDX, IDC_EDIT_URL, m_strUrl);
	//DDX_Control(pDX, IDC_STATIC_CAPTRUE, m_staticCaptrue);
}


BEGIN_MESSAGE_MAP(DlgRtmpPull, CDialog)
	ON_WM_CLOSE()
	//ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_PULLDLG_RESIZE, OnPullDlgResize)
	ON_MESSAGE(WM_PULLDLG, OnPullDlg)
	ON_WM_GETMINMAXINFO()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	//ON_WM_CTLCOLOR()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// DlgRtmpPull 消息处理程序

LRESULT DlgRtmpPull::OnPullDlgResize(WPARAM wp, LPARAM lp) {
	m_nVideoWidth = wp;
	m_nVideoHeight = lp;
	PostMessage(WM_SIZE, 0, 0);
	return TRUE;
}

void DlgRtmpPull::OnOK() {
}

void DlgRtmpPull::OnCancel() {
	ShowWindow(SW_HIDE);
	Stop();
}

void DlgRtmpPull::OnClose() {
	ShowWindow(SW_HIDE);
	Stop();
}

BOOL DlgRtmpPull::OnInitDialog() {
	CDialog::OnInitDialog();
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
#ifdef _DEBUG
		pSysMenu->AppendMenu(MF_STRING, IDR_MENU_SYS_MORE_SHOWDEVTOOLS, L"开发者工具");
		pSysMenu->AppendMenu(MF_STRING, IDR_MENU_SYS_MORE_REFRESH, L"刷新界面");
#endif
	}
	m_myStatic.SubclassDlgItem(IDC_STATIC_CAPTRUE, this);
	m_hWndPullDlg = GetSafeHwnd();
	srand(time(NULL));
	brush.CreateSolidBrush(RGB(0, 0, 0));

	//m_pDlgVideoMain = new DlgVideo(this);
	//m_pDlgVideoMain->Create(DlgVideo::IDD, this);
	//m_pDlgVideoMain->ShowWindow(SW_HIDE);
	//CRect rc;
	//m_myStatic.GetWindowRect(rc);
	//m_staticCaptrue.ShowWindow(SW_HIDE);
	//ScreenToClient(rc);
	//m_pDlgVideoMain->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW);

	CRect rc1;
	GetClientRect(rc1);
	m_nVideoWidth = rc1.Width() - m_nChatroomWidth;

	CefBrowserSettings settings;
	
	std::string url;
	if (url.empty())
		url = m_baseurl + "live/h5client/mainpage/#/pages/chatroom/chatroom";
	CRect rc;
	CefWindowInfo window_info;
	rc.left = 0;
	rc.top = 0;
	rc.bottom = 1000;
	rc.right = 500;
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC_CEF3);
	window_info.SetAsChild(pWnd->GetSafeHwnd(), rc);
	CefBrowserHost::CreateBrowser(window_info, theApp.handler, url, settings, NULL);

	url = mmedialisturl;// m_baseurl + "live/h5client/mainpage/#/pages/medialist/medialist";
	pWnd = this->GetDlgItem(IDC_STATIC_LIST);
	window_info.SetAsChild(pWnd->GetSafeHwnd(), rc);
	CefBrowserHost::CreateBrowser(window_info, theApp.handler, url, settings, NULL);

	PostMessage(WM_SIZE, 0, 0);
	PostMessage(WM_PULLDLG, 0, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL DlgRtmpPull::DestroyWindow(){
	if (m_pAVRtmplayer) {
		m_pAVRtmplayer->StopRtmpPlay();
		RTMPGuester::Destory(m_pAVRtmplayer);
		m_pAVRtmplayer = NULL;
	}
	//if (m_pDlgVideoMain) {
	//	m_pDlgVideoMain->DestroyWindow();
	//	delete m_pDlgVideoMain;
	//	m_pDlgVideoMain = NULL;
	//}

	return CDialog::DestroyWindow();
}

BOOL DlgRtmpPull::OnEraseBkgnd(CDC* pDC){
	return TRUE;
}

void DlgRtmpPull::OnLButtonDblClk(UINT nFlags, CPoint point){
	CDialog::OnLButtonDblClk(nFlags, point);
}



void DlgRtmpPull::Start(){
	Stop();
	ShowWindow(SW_SHOW);
	if (m_pAVRtmplayer == NULL) {
		delete[] m_pAudioMarker;
		CString strId;
		int id = 0;
		m_iAudioMarker = 0;
		strId.Format(_T("%d"), gUserId);
		TRACE(strId);
		TRACE("\r\n");
		for (int i = 0; i < strId.GetLength(); i++)	{
			id = _tstoi(strId.Mid(i, 1));
			m_iAudioMarker += theApp.m_iSoundMarker[id] + 12000;
		}

		int RANGE_MIN = 200;
		int RANGE_MAX = 240;
		srand(time(NULL));
		m_iAudioMarketId = 0;
		m_iAudioMarketIdNew = 0;
		m_iAudioMarketLast = time(NULL);
		m_iAudioMarketStart[0] = (((double)rand() / (double)(RAND_MAX + 1)) * (RANGE_MAX - RANGE_MIN) + RANGE_MIN);
		m_iAudioMarketStart[0] -= 200;
		if (m_iAudioMarketStart[0] <= 20)
			m_iAudioMarketStart[0] = 20;
		for (int i = 1; i < _countof(m_iAudioMarketStart); i++)	{
			m_iAudioMarketStart[i] = m_iAudioMarketStart[i - 1] + m_iAudioMarketStart[i - 1] + (((double)rand() / (double)RAND_MAX) * RANGE_MAX + RANGE_MIN);
		}

		m_pAudioMarker = new char[m_iAudioMarker];
		m_pAudioMarketOut = (short*)m_pAudioMarker;
		memset(m_pAudioMarker, 0, m_iAudioMarker);
		char* pCur = m_pAudioMarker;
		for (int i = 0; i < strId.GetLength(); i++)	{
			id = _tstoi(strId.Mid(i, 1));
			memcpy(pCur, theApp.m_soundMarker[id], theApp.m_iSoundMarker[id]);
			pCur += theApp.m_iSoundMarker[id] + 12000;
		}
		std::string url;
		if (url.empty())
			url =  m_strUri;//mrtmpurl;//
		m_pAVRtmplayer = RTMPGuester::Create(*this);
		UpdateData(TRUE);
		m_pAVRtmplayer->StartRtmpPlay(url.c_str(),m_myStatic.GetSafeHwnd() /*m_pDlgVideoMain->m_hWnd*/, "rtmp","");
		m_btnRtmp.SetWindowTextW(L"结束");
	}
}


void DlgRtmpPull::Stop(){
	if (m_pAVRtmplayer != NULL) {
		//m_btnRtmp.SetWindowTextW(L"拉流");
		RTMPGuester::Destory(m_pAVRtmplayer);
		m_pAVRtmplayer = NULL;
	}
}
void DlgRtmpPull::OnGetMinMaxInfo(MINMAXINFO* lpMMI) {
	//设置对话框最小宽度与高度
	//RECT rc;
	//if (m_pDlgVideoMain != NULL) {
		//if (IsWindow(m_pDlgVideoMain->GetSafeHwnd())) {
			//m_pDlgVideoMain->GetWindowRect(&rc);
			//ScreenToClient(&rc);
			lpMMI->ptMinTrackSize.x = m_nVideoWidth + 40 + m_nChatroomWidth + GetSystemMetrics(SM_CXFRAME) * 2;
			lpMMI->ptMinTrackSize.y = m_nVideoHeight + 20 + m_nListHeight + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) * 2;
		//}
	//}
	__super::OnGetMinMaxInfo(lpMMI);
}

void DlgRtmpPull::OnSize(UINT nType, int cx, int cy){
	__super::OnSize(nType, cx, cy);
	CRect rcClient, rcVideo, rcChatroom, rcStatic, rcList;
	GetClientRect(rcClient);
	rcList = rcStatic = rcChatroom = rcClient;
	rcChatroom.left = rcChatroom.right - m_nChatroomWidth;

	rcVideo.left = 0;
	rcVideo.top = 0;
	rcVideo.right = m_nVideoWidth + 40;
	rcVideo.bottom = m_nVideoHeight + 20;

	// 聊天窗口webview大小。
	if (rcChatroom.left < rcVideo.right) {
		rcChatroom.left = rcVideo.right;// +GetSystemMetrics(SM_CYFRAME);
	}
	if (IsWindow(this->GetSafeHwnd())) {
		CWnd* pWnd = GetDlgItem(IDC_STATIC_CEF3);
		if (IsWindow(pWnd->GetSafeHwnd())) {
			pWnd->SetWindowPos(NULL, rcChatroom.left, rcChatroom.top, rcChatroom.Width(), rcChatroom.Height(), SWP_NOZORDER);
			CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
			if (pb != nullptr) {
				auto hwnd = pb->GetHost()->GetWindowHandle();
				::SetWindowPos(hwnd, HWND_TOP, rcChatroom.left, rcChatroom.top, rcChatroom.right - rcChatroom.left, rcChatroom.bottom - rcChatroom.top, SWP_NOZORDER | SWP_NOMOVE);
			}
		}
	}

	// 播放主窗口黑色背景大小。
	rcStatic.right = rcChatroom.left - GetSystemMetrics(SM_CYFRAME);
	if (rcStatic.right < rcVideo.right) {
		rcStatic.right = rcVideo.right;
	}
	int yAdj = (rcStatic.Height() - rcVideo.Height());
	rcList = rcStatic;
	rcStatic.bottom = rcStatic.bottom - yAdj;
	if (rcStatic.bottom < rcVideo.bottom) {
		rcStatic.bottom = rcVideo.bottom;
	}
	rcList.top = rcStatic.bottom;// +GetSystemMetrics(SM_CXFRAME);
	if (IsWindow(this->m_myStatic.GetSafeHwnd())) {
		m_myStatic.SetWindowPos(NULL, 0, 0, rcStatic.Width(), rcStatic.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
	// 历史录像列表webview位置和大小。
	CWnd* pList = this->GetDlgItem(IDC_STATIC_LIST);
	if (pList != NULL) {
		pList->SetWindowPos(NULL, rcList.left, rcList.top, rcList.Width(), rcList.Height(), SWP_NOZORDER);
		if (IsWindow(pList->GetSafeHwnd())) {
			CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pList->GetSafeHwnd());
			if (pb != nullptr) {
				auto hwnd = pb->GetHost()->GetWindowHandle();
				::SetWindowPos(hwnd, HWND_TOP, rcList.left, rcList.top, rcList.right - rcList.left, rcList.bottom - rcList.top, SWP_NOZORDER | SWP_NOMOVE);
			}
		}
	}
	// 播放主窗口大小。
	/*
	if (m_pDlgVideoMain != NULL) {
		int x = 0;
		int y = 0;
		if (rcStatic.Height() > rcVideo.Height()) {
			y = (rcStatic.Height() - rcVideo.Height()) / 2;
		}
		if (rcStatic.Width() > rcVideo.Width()) {
			x = (rcStatic.Width() - rcVideo.Width()) / 2;
		}
		if (IsWindow(m_pDlgVideoMain->GetSafeHwnd())) {
			m_pDlgVideoMain->SetWindowPos(NULL, x, y, rcVideo.Width(), rcVideo.Height(), SWP_NOZORDER);
		}
	}
	*/
	return;
}

HBRUSH DlgRtmpPull::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	/*
	if (pWnd->GetDlgCtrlID() == IDC_STATIC_CAPTRUE) {
		return (HBRUSH)brush.GetSafeHandle();
	}
	*/
	return hbr;
}

void DlgRtmpPull::OnSysCommand(UINT nID, LPARAM lParam){
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC_LIST);
	if ((nID) == IDR_MENU_SYS_MORE_REFRESH) {
		CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
		if (pb != nullptr) {
			std::string url = pb->GetMainFrame()->GetURL().ToString();
			pb->ReloadIgnoreCache();
		}
	}
	else if ((nID) == IDR_MENU_SYS_MORE_SHOWDEVTOOLS) {
		CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
		if (pb != nullptr) {
			CefWindowInfo win_info;
			CefRefPtr<CefClient> client;
			CefBrowserSettings settings;
			win_info.SetAsPopup(GetSafeHwnd(), CefString("DevTools"));
			pb->GetHost()->ShowDevTools(win_info, theApp.handler, settings, CefPoint());
		}
	}
	__super::OnSysCommand(nID, lParam);
}

LRESULT DlgRtmpPull::OnPullDlg(WPARAM, LPARAM) {
	//Start();
	//return TRUE;
	std::string url = "";
	CefString str = CPullDlgData::me()->pop();
	if (!str.empty()) {
		CefRefPtr<CefValue> jsonObject = CefParseJSON(str, JSON_PARSER_ALLOW_TRAILING_COMMAS);
		if (jsonObject->IsValid()){
			CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
			CefString token = dict->GetString("cmd");
			if (token == "pulldlg") {
				CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
				CefString title = data->GetString("title");
				CefString roomid = data->GetString("id");
				CefString pulluri = data->GetString("pulluri");
				CefString background = data->GetString("background");
				data = dict->GetDictionary("ui");
				CefString sessionid = data->GetString("SessionId");
				CefString token = data->GetString("Token");
				url = std::string(pulluri);
				std::string sid = std::string(sessionid);
				std::string tkn = std::string(token);
				std::string urlmedialist = m_baseurl + "live/h5client/mainpage/#/pages/medialist/medialist?roomid=";// +;
				urlmedialist += roomid;
				GetMedialist()->GetMainFrame()->LoadURL(urlmedialist);
				m_strUri = url + "?sessionid=" + sid + "&token=" + tkn;
				CWnd* pWnd = this->GetDlgItem(IDC_STATIC_CEF3);
				if (pWnd != NULL && IsWindow(pWnd->GetSafeHwnd())) {
					CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
					if (pb != nullptr) {
						pb->ReloadIgnoreCache();
					}
				}

				this->SetWindowText(title.c_str());
				Start();
			}else if (token == "pulldlghis") {
				
				if (gLiving == 99 ) {
					AfxMessageBox(L"直播进行中，不能查看直播录像！请直播完毕后查看！");
					return TRUE;
				}
				Stop();
				CDlgFlvPlayer dlg(this, str);
				dlg.DoModal();
				Start();
			}
		}
	}
	return TRUE;
}

BOOL DlgRtmpPull::PreTranslateMessage(MSG* pMsg){
	// TODO: 在此添加专用代码和/或调用基类
	return __super::PreTranslateMessage(pMsg);
}

void DlgRtmpPull::OnRtmplayerPlayStart() {
	TRACE("DlgRtmpPull::OnRtmplayerPlayStart\r\n");
	gLiving = 1;
}
void DlgRtmpPull::OnRtmplayerPlayStop() {
	TRACE("DlgRtmpPull::OnRtmplayerPlayStop\r\n");
	gLiving = 0;
	m_myStatic.Invalidate();
	//m_pDlgVideoMain->ShowWindow(SW_HIDE);
}
void DlgRtmpPull::OnRtmplayer1stVideo() {
	gLiving = 99;
	m_myStatic.Invalidate();
	TRACE("DlgRtmpPull::OnRtmplayer1stVideo\r\n");
	//m_pDlgVideoMain->ShowWindow(SW_SHOW);
}
void DlgRtmpPull::OnRtmplayer1stAudio() {
	gLiving = 99;
	m_myStatic.Invalidate();
	TRACE("DlgRtmpPull::OnRtmplayer1stAudio\r\n");
}

void DlgRtmpPull::OnRtmplayerOK() {
	//gLiving = 1;
	m_myStatic.Invalidate();
	TRACE("OnRtmplayerOK\r\n");
};
void DlgRtmpPull::OnRtmplayerStatus(int cacheTime, int curBitrate) {
	//TRACE("OnRtmplayerStatus cacheTime:%d curBitrate:%d\r\n",cacheTime,curBitrate);
};
void DlgRtmpPull::OnRtmplayerCache(int time) {
};
void DlgRtmpPull::OnRtmplayerClosed(int errcode) {
	gLiving = 2;
	TRACE("OnRtmplayerClosed\r\n");
	m_myStatic.Invalidate();
	//m_pDlgVideoMain->ShowWindow(SW_HIDE);
};

void DlgRtmpPull::OnRtmplayerConnectionFailed(int a) {
	gLiving = 4;
	TRACE("DlgRtmpPull::OnRtmplayerConnectionFailed: %d\r\n",a);
}