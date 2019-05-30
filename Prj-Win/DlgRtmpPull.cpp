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
#include "webapi.h"

// DlgRtmpPull �Ի���
HWND	m_hWndPullDlg;
extern int gUserId;
//extern int gLiving;
extern std::wstring m_baseurl;

IMPLEMENT_DYNAMIC(DlgRtmpPull, CDialog)

DlgRtmpPull::DlgRtmpPull(const char* rtmpurl,const char* medialisturl)
	: CDialog(DlgRtmpPull::IDD)
	, m_strUrl(_T("rtmp://www.pic98.com/live/livestream"))
	, m_pAVRtmplayer(NULL)
//	, m_pDlgVideoMain(NULL)
	//, m_pAudioMarker(NULL)
	, m_nVideoWidth(720)
	, m_nVideoHeight(576)
	, m_nChatroomWidth(420)
	, m_nListHeight(220)
	, mrtmpurl(rtmpurl)
	, mmedialisturl(medialisturl)
	{
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


// DlgRtmpPull ��Ϣ�������

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
	webapi::me();
	if (pSysMenu != NULL){
#ifdef _DEBUG
		pSysMenu->AppendMenu(MF_STRING, IDR_MENU_SYS_MORE_SHOWDEVTOOLS, L"�����߹���");
		pSysMenu->AppendMenu(MF_STRING, IDR_MENU_SYS_MORE_REFRESH, L"ˢ�½���");
#endif
	}
	m_myStatic.SubclassDlgItem(IDC_STATIC_CAPTRUE, this);
	m_hWndPullDlg = GetSafeHwnd();
	srand(time(NULL));
	brush.CreateSolidBrush(RGB(0, 0, 0));
	CRect rc1;
	GetClientRect(rc1);
	m_nVideoWidth = rc1.Width() - m_nChatroomWidth;

	CefBrowserSettings settings;
	
	std::wstring url;
	if (url.empty())
		url = m_baseurl + L"live/h5client/mainpage/#/pages/chatroom/chatroom";
	CRect rc;
	CefWindowInfo window_info;
	rc.left = 0;
	rc.top = 0;
	rc.bottom = 1000;
	rc.right = 500;
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC_SB_CHATROOM);
	window_info.SetAsChild(pWnd->GetSafeHwnd(), rc);
	//url = L"about:blank";
	CefBrowserHost::CreateBrowser(window_info, theApp.handler, url, settings, NULL);

	url = m_baseurl + L"live/h5client/mainpage/#/pages/medialist/medialist";
	pWnd = this->GetDlgItem(IDC_STATIC_SB_MEDIALIST);
	window_info.SetAsChild(pWnd->GetSafeHwnd(), rc);
	url = L"about:blank";
	CefBrowserHost::CreateBrowser(window_info, theApp.handler, url, settings, NULL);

	PostMessage(WM_SIZE, 0, 0);
	PostMessage(WM_PULLDLG, 0, 0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL DlgRtmpPull::DestroyWindow(){
	if (m_pAVRtmplayer) {
		m_pAVRtmplayer->StopRtmpPlay();
		RTMPGuester::Destory(m_pAVRtmplayer);
		m_pAVRtmplayer = NULL;
	}
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
		CStringA strId;
		int id = 0;
		strId.Format(("%d"), gUserId);
		TRACE(strId);
		TRACE("\r\n");
		std::string url;
		if (url.empty())
			url =  m_strUri;
		m_pAVRtmplayer = RTMPGuester::Create(*this);
		UpdateData(TRUE);
		m_pAVRtmplayer->StartRtmpPlay(url.c_str(),m_myStatic.GetSafeHwnd(), "rtmp","",1,0, strId.GetBuffer(), (const short**)theApp.m_soundMarker,theApp.m_iSoundMarker);
		m_btnRtmp.SetWindowTextW(L"����");
		this->ShowWindow(SW_SHOWNORMAL);
	}
}


void DlgRtmpPull::Stop(){
	if (m_pAVRtmplayer != NULL) {
		CefRefPtr<CefBrowser> p = GetMedialist();
		if (p != NULL) {
			CefRefPtr<CefFrame> pf = p->GetMainFrame();
			if (pf != NULL) {
				pf->LoadURL(L"about:blank");
				Sleep(100);
			}
		}
		RTMPGuester::Destory(m_pAVRtmplayer);
		m_pAVRtmplayer = NULL;
	}
}
void DlgRtmpPull::OnGetMinMaxInfo(MINMAXINFO* lpMMI) {
	lpMMI->ptMinTrackSize.x = m_nVideoWidth + 40 + m_nChatroomWidth + GetSystemMetrics(SM_CXFRAME) * 2;
	lpMMI->ptMinTrackSize.y = m_nVideoHeight + 20 + m_nListHeight + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) * 2;
	__super::OnGetMinMaxInfo(lpMMI);
}

void DlgRtmpPull::OnSize(UINT nType, int cx, int cy) {
	__super::OnSize(nType, cx, cy);
	CRect rcClient, rcVideo, rcChatroom, rcStatic, rcList;
	GetClientRect(rcClient);
	rcList = rcStatic = rcChatroom = rcClient;
	rcChatroom.left = rcChatroom.right - m_nChatroomWidth;

	rcVideo.left = 0;
	rcVideo.top = 0;
	rcVideo.right = m_nVideoWidth + 40;
	rcVideo.bottom = m_nVideoHeight + 20;

	// ���촰��webview��С��
	if (rcChatroom.left < rcVideo.right) {
		rcChatroom.left = rcVideo.right;// +GetSystemMetrics(SM_CYFRAME);
	}
	if (IsWindow(this->GetSafeHwnd())) {
		CWnd* pWnd = GetDlgItem(IDC_STATIC_SB_CHATROOM);
		if (IsWindow(pWnd->GetSafeHwnd())) {
			pWnd->SetWindowPos(NULL, rcChatroom.left, rcChatroom.top, rcChatroom.Width(), rcChatroom.Height(), SWP_NOZORDER);
			CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
			if (pb != nullptr) {
				auto hwnd = pb->GetHost()->GetWindowHandle();
				::SetWindowPos(hwnd, HWND_TOP, rcChatroom.left, rcChatroom.top, rcChatroom.right - rcChatroom.left, rcChatroom.bottom - rcChatroom.top, SWP_NOZORDER | SWP_NOMOVE);
			}
		}
	}

	// ���������ں�ɫ������С��
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
	// ��ʷ¼���б�webviewλ�úʹ�С��
	CWnd* pList = this->GetDlgItem(IDC_STATIC_SB_MEDIALIST);
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
	return;
}

HBRUSH DlgRtmpPull::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor){
	HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

void DlgRtmpPull::OnSysCommand(UINT nID, LPARAM lParam){
	CWnd* pWnd = this->GetDlgItem(IDC_STATIC_SB_MEDIALIST);
	if ((nID) == IDR_MENU_SYS_MORE_REFRESH) {
		CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
		if (pb != nullptr) {
			std::string url = pb->GetMainFrame()->GetURL().ToString();
			//GetMedialist()->GetMainFrame()->ExecuteJavaScript("console.log('hello')", GetMedialist()->GetMainFrame()->GetURL(), 0);

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

	
	std::wstring url = L"";
	CefString str = CPullDlgData::me()->pop();
	if (str.empty())
		return TRUE;
	CefRefPtr<CefValue> jsonObject = CefParseJSON(str, JSON_PARSER_ALLOW_TRAILING_COMMAS);
	if (!jsonObject->IsValid())
		return TRUE;
	CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
	CefString token = dict->GetString("cmd");
	if (token == "pulldlg") {
		std::wstringstream ss;
		CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
		CefString title = data->GetString("title");
		int roomid = data->GetInt("roomid");
		ss << L"rtmp://" << data->GetString("vhost").ToWString() << L":" << data->GetInt("vport")
			<< L"/" << data->GetString("vapp").ToWString() << L"/" << data->GetString("vstream").ToWString();
		ss >> url;
		CefString background = data->GetString("background");
		data = dict->GetDictionary("ui");
		std::wstring sid = data->GetString("SessionID").ToWString();
		std::wstring tkn = data->GetString("Token").ToWString();
		ss.clear();
		ss.str(L"");
		ss << m_baseurl << L"live/h5client/mainpage/#/pages/medialist/medialist?roomid=" << roomid;
		std::wstring urlmedialist;
		ss >> urlmedialist;
		CefString tempurl;
		tempurl.FromWString(urlmedialist);

		GetMedialist()->GetMainFrame()->LoadURL(tempurl);
		//GetMedialist()->Reload();
		m_strUri = url + L"?sessionid=" + sid + L"&token=" + tkn;
		/*
		CWnd* pWnd = this->GetDlgItem(IDC_STATIC_CEF3);
		if (pWnd != NULL && IsWindow(pWnd->GetSafeHwnd())) {
			CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
			if (pb != nullptr) {
				pb->ReloadIgnoreCache();
			}
		}

		pWnd = this->GetDlgItem(IDC_STATIC_LIST);
		if (pWnd != NULL && IsWindow(pWnd->GetSafeHwnd())) {
			CefRefPtr<CefBrowser> pb = theApp.handler->GetBrowser(pWnd->GetSafeHwnd());
			if (pb != nullptr) {
				pb->ReloadIgnoreCache();
			}
		}
		*/
		this->SetWindowText(title.c_str());
		Start();
	}else if (token == "pulldlghis") {
				
		if ( m_myStatic.GetLiving() == 99 ) {
			AfxMessageBox(L"ֱ�������У����ܲ鿴ֱ��¼����ֱ����Ϻ�鿴��");
			return TRUE;
		}
		Stop();
		CWnd* pWnd = AfxGetMainWnd();
		CDlgFlvPlayer dlg(NULL, str);
		dlg.DoModal();
		Start();
	}
	else {

		if (m_myStatic.GetLiving() == 99) {
			AfxMessageBox(L"ֱ�������У����ܲ鿴ֱ��¼����ֱ����Ϻ�鿴��");
			return TRUE;
		}
		CWnd* pWnd = AfxGetMainWnd();
		//Stop();
		CDlgFlvPlayer dlg(NULL, str);
		dlg.DoModal();
		//Start();
	}
	
	return TRUE;
}

BOOL DlgRtmpPull::PreTranslateMessage(MSG* pMsg){
	// TODO: �ڴ����ר�ô����/����û���
	return __super::PreTranslateMessage(pMsg);
}

void DlgRtmpPull::OnRtmplayerPlayStart() {
	TRACE("DlgRtmpPull::OnRtmplayerPlayStart\r\n");
	m_myStatic.SetLiving(1);
}
void DlgRtmpPull::OnRtmplayerPlayStop() {
	TRACE("DlgRtmpPull::OnRtmplayerPlayStop\r\n");
	m_myStatic.SetLiving(0);
}
void DlgRtmpPull::OnRtmplayer1stVideo() {
	m_myStatic.SetLiving(99);
	TRACE("DlgRtmpPull::OnRtmplayer1stVideo\r\n");
}
void DlgRtmpPull::OnRtmplayer1stAudio() {
	m_myStatic.SetLiving(99);
	TRACE("DlgRtmpPull::OnRtmplayer1stAudio\r\n");
}

void DlgRtmpPull::OnRtmplayerOK() {
	m_myStatic.Invalidate();
	//TRACE("OnRtmplayerOK\r\n");
};
void DlgRtmpPull::OnRtmplayerStatus(int cacheTime, int curBitrate, uint32_t curTime, double totalTime) {
	//TRACE("OnRtmplayerStatus cacheTime:%d curBitrate:%d\r\n",cacheTime,curBitrate);
};
void DlgRtmpPull::OnRtmplayerCache(int time) {
};
void DlgRtmpPull::OnRtmplayerClosed(int errcode) {
	m_myStatic.SetLiving(2);
	//TRACE("OnRtmplayerClosed\r\n");
};

void DlgRtmpPull::OnRtmplayerConnectionFailed(int a) {
	m_myStatic.SetLiving(4);
	//TRACE("DlgRtmpPull::OnRtmplayerConnectionFailed: %d\r\n",a);
}