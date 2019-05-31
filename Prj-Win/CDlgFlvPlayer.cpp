﻿// CDlgFlvPlayer.cpp: 实现文件
//

#include "stdafx.h"
#include "include/cef_parser.h"
#include "LiveWin32.h"
#include "CDlgFlvPlayer.h"
#include "afxdialogex.h"
#include "xdefines.h"
#include <thread>
#include <wininet.h>
#include "webapi.h"

#pragma warning (disable:4996)
extern int gUserId;
extern std::wstring m_baseurl;
extern std::string m_apiurl;
// CDlgFlvPlayer 对话框

IMPLEMENT_DYNAMIC(CDlgFlvPlayer, CDialogEx)

CDlgFlvPlayer::CDlgFlvPlayer(CWnd* pParent /*=nullptr*/,const std::string& info)
	: CDialogEx(IDD_DLG_FLVPLAYER, pParent)
	, minfo(info)
	, mReady(FALSE)
	, m_pPlayer(NULL){
}

CDlgFlvPlayer::~CDlgFlvPlayer(){
}

void CDlgFlvPlayer::DoDataExchange(CDataExchange* pDX){
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFlvPlayer, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgFlvPlayer::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgFlvPlayer::OnBnClickedCancel)
	ON_MESSAGE(WM_PULLDLG_RESIZE, &OnPullDlgResize)
	ON_MESSAGE(WM_PULLDLG_SEEKTO, &CDlgFlvPlayer::OnPullDlgSeekto)
	ON_MESSAGE(WM_DLGFLVPLAYER_PLAY, OnDlgFlvPlayer_Play)
	ON_MESSAGE(WM_DLGFLVPLAYER_CLOSE,OnDlgFlvPlayer_Close)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_DWMWINDOWMAXIMIZEDCHANGE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOVING()
END_MESSAGE_MAP()

void CDlgFlvPlayer::OnBnClickedOk(){
	//CDialogEx::OnOK();
}


void CDlgFlvPlayer::OnBnClickedCancel(){
	if (AfxMessageBox(L"退出播放？", MB_YESNO) == IDYES) {
		if (m_pPlayer != nullptr) {
			m_pPlayer->StopRtmpPlay();
			RTMPGuester::Destory(m_pPlayer);
			m_pPlayer = NULL;
		}
		CDialogEx::OnCancel();
	}
}
LRESULT CDlgFlvPlayer::OnDlgFlvPlayer_Close(WPARAM, LPARAM) {
	CDialogEx::OnCancel();
	return TRUE;
}

LRESULT CDlgFlvPlayer::OnDlgFlvPlayer_Play(WPARAM wp,LPARAM lp) {
	char* pbuf1 = (char*)wp;
	char* pbuf2 = (char*)lp;
	CefRefPtr<CefValue> jsonObject = CefParseJSON(pbuf1, JSON_PARSER_ALLOW_TRAILING_COMMAS);
	if (jsonObject->IsValid()) {
		CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
		CefString token = dict->GetString("cmd");
		CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
		CefString createdate = data->GetString("createdate");
		CefString id = data->GetString("id");
		CefString filepath = data->GetString("filepath");
		CefString nickname = data->GetString("nickname");
		CefString filename = data->GetString("filename");
		std::wstring nn = nickname.ToWString();
		std::wstring fn = filename.ToWString();
		nn.erase(nn.find_last_not_of(' ') + 1);
		int32_t enc = data->GetInt("encrypted");
		std::string strid = id.ToString();
		data = dict->GetDictionary("ui");
		CefString sessionid = data->GetString("SessionID");
		//CefString token = data->GetString("Token");
		int userid = gUserId = data->GetInt("UserID");
		CStringA strId;
		strId.Format("%d", gUserId);

		char enckey = 0;

		CefRefPtr<CefValue> jsonObject = CefParseJSON(pbuf2, JSON_PARSER_ALLOW_TRAILING_COMMAS);
		if (jsonObject->IsValid()) {
			CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
			CefRefPtr<CefListValue> list = dict->GetList("data");
			CefRefPtr<CefDictionaryValue> resdata = list->GetDictionary(0);
			if (resdata != nullptr) {
				enckey = resdata->GetInt("enckey");
				enc = resdata->GetInt("encrypted");
			}

		}
		std::string title;
		if (fn == nn) {
			std::string fp(filepath);
			int lasti = fp.length();
			std::string parts[5];
			int idx = 0;
			for (int i = fp.length() - 1; i >= 0; i--) {
				if (fp[i] == '/' || fp[i] == '\\') {
					parts[idx] = fp.substr(i + 1, lasti - i - 1);
					lasti = i;
					idx++;
					if (idx > 4)
						break;
				}
			}
			title.append(parts[2]);
			title.append("年");
			title.append(parts[1]);
			title.append("月");
			title.append(parts[0].substr(0, 2));
			title.append("日");
			title.append(parts[0].substr(2, 2));
			title.append("时");
			title.append(parts[0].substr(4, 2));
			title.append("分");
			title.append(parts[0].substr(6, 2));
			title.append("秒 - 直播录像");
			this->SetWindowText(CA2W(title.c_str()));
		}
		else {
			SetWindowText(nickname.ToWString().c_str());
		}

		m_pPlayer = RTMPGuester::Create(*this);
		std::string url = "http://gpk01.gwgz.com:8862/";
		url = url + std::string(filepath);
		char path[1024] = { 0 };
		::GetModuleFileNameA(NULL, path, sizeof(path));
		for (int i = strlen(path); i >= 0; i--) {
			if (path[i] == '/' || path[i] == '\\') {
				path[i] = 0;
				break;
			}
		}
		m_pPlayer->StartRtmpPlay(url.c_str(), GetDlgItem(IDC_STATIC_VIDEO)->GetSafeHwnd(), "flv", path, enc, enckey, strId.GetBuffer(), (const short**)theApp.m_soundMarker, theApp.m_iSoundMarker);

	}
	delete[] pbuf1;
	delete[] pbuf2;
	return TRUE;
}

extern std::string UTF8_To_string(const std::string & str);
BOOL CDlgFlvPlayer::OnInitDialog(){
	CDialogEx::OnInitDialog();
	//ShowWindow(SW_HIDE);
	m_myStatic.SubclassDlgItem(IDC_STATIC_VIDEO, this);
	m_playerBar = new CDlgPlayerBar(this);
	m_playerBar->Create(IDD_DLG_PLAYERBAR, this);
	m_playerBar->ShowWindow(SW_HIDE);
	std::string str1 = minfo;
	webapi::me();
	if (!minfo.empty()) {
		TRACE(minfo.c_str());
		CefRefPtr<CefValue> jsonObject = CefParseJSON(minfo, JSON_PARSER_ALLOW_TRAILING_COMMAS);
		if (jsonObject->IsValid()){
			CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
			CefString cmd = dict->GetString("cmd");
			
			if (cmd == "pulldlghis117" || cmd == "pulldlghisgp" || cmd == "pulldlghis" ) {
				CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
				int id = data->GetInt("id");
				data = dict->GetDictionary("ui");
				CefString sessionid = data->GetString("SessionID");
				int userid = gUserId = data->GetInt("UserID");
				HWND hWndThis = GetSafeHwnd();
				std::thread t1([hWndThis,str1,id,sessionid,userid, cmd]() {
					std::string posturl(m_apiurl + "api/1.00/private");
					if (cmd == "pulldlghis117") {
						posturl = m_apiurl + "api/1.00/public";
					}
					std::string par;
					std::stringstream ss;
					ss << "{\"action\":\"caniplay\",\"id\":\"" << id << "\"}";
					std::string ret = webapi::me()->post((char*)posturl.c_str(),(char*)sessionid.ToString().c_str() ,(char*)ss.str().c_str());
					if (ret.length() < 1) {
						MessageBoxA(hWndThis, "查询播放文件失败！", "注意", MB_OK);
						::PostMessage(hWndThis, WM_DLGFLVPLAYER_CLOSE, 0, 0);
					}
					CefRefPtr<CefValue> jsonObject = CefParseJSON(ret, JSON_PARSER_ALLOW_TRAILING_COMMAS);
					if (jsonObject->IsValid()) {
						CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
						if (dict->GetInt("status") != 0) {
							CefString msg = dict->GetString("msg");
							
							MessageBoxA(hWndThis, UTF8_To_string(msg.ToString()).c_str(),"注意",MB_OK);
							::PostMessage(hWndThis, WM_DLGFLVPLAYER_CLOSE, 0, 0);
							return;
						}
					}
					char* pbuf1 = new char[str1.length() + 1];
					strcpy(pbuf1, str1.c_str());
					char* pbuf2 = new char[ret.length() + 1];
					strcpy(pbuf2, ret.c_str());
					::PostMessage(hWndThis, WM_DLGFLVPLAYER_PLAY,(WPARAM)pbuf1,(LPARAM)pbuf2);
				});
				t1.detach();
			}
		}
	}
	//ShowWindow(SW_SHOW);
	return TRUE;  // return TRUE unless you set the focus to a control
}


void CDlgFlvPlayer::PostNcDestroy(){
	__super::PostNcDestroy();
}

void CDlgFlvPlayer::OnRtmplayerPlayStart() {
	TRACE("CDlgFlvPlayer::OnRtmplayerPlayStart\r\n");
}
void CDlgFlvPlayer::OnRtmplayerPlayStop() {
	TRACE("CDlgFlvPlayer::OnRtmplayerPlayStop\r\n");
}
void CDlgFlvPlayer::OnRtmplayer1stVideo() {
	mReady = TRUE;
	m_myStatic.SetLiving(99);
	TRACE("CDlgFlvPlayer::OnRtmplayer1stVideo\r\n");
}
void CDlgFlvPlayer::OnRtmplayer1stAudio() {
	m_myStatic.SetLiving(99);
	TRACE("CDlgFlvPlayer::OnRtmplayer1stAudio\r\n");
}
void CDlgFlvPlayer::OnRtmplayerConnectionFailed(int a) {

}
void CDlgFlvPlayer::OnRtmplayerOK() {
	//TRACE("OnRtmplayerOK");
};
void CDlgFlvPlayer::OnRtmplayerStatus(int cacheTime_, int curBitrate_, uint32_t curTime_, double totalTime_) {
	cacheTime = cacheTime_, curBitrate = curBitrate_, curTime = curTime_, totalTime = totalTime_;
	m_playerBar->mcurTime = curTime / 1000;
	m_playerBar->mtotalTime = totalTime;
};
void CDlgFlvPlayer::OnRtmplayerCache(int time) {
};
void CDlgFlvPlayer::OnRtmplayerClosed(int errcode) {
	TRACE("OnRtmplayerClosed");
};


void CDlgFlvPlayer::OnGetMinMaxInfo(MINMAXINFO* lpMMI) {
	lpMMI->ptMinTrackSize.x = m_nVideoWidth + 40 +  GetSystemMetrics(SM_CXFRAME) * 2;
	lpMMI->ptMinTrackSize.y = m_nVideoHeight + 20 + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) * 2;
	__super::OnGetMinMaxInfo(lpMMI);
}

LRESULT CDlgFlvPlayer::OnPullDlgSeekto(WPARAM wp, LPARAM lp) {
	m_pPlayer->SeekTo(wp, lp);
	return TRUE;
}

LRESULT CDlgFlvPlayer::OnPullDlgResize(WPARAM wp, LPARAM lp) {
	m_nVideoWidth = wp;
	m_nVideoHeight = lp;
	CRect rc;
	GetClientRect(rc);
	if (rc.Width() < m_nVideoWidth + 40 + GetSystemMetrics(SM_CXFRAME) * 2) {
		rc.right = rc.left + m_nVideoWidth + 40 + GetSystemMetrics(SM_CXFRAME) * 2;
	}
	if (rc.Height() < m_nVideoHeight + 20 + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) * 2) {
		rc.bottom = rc.top + m_nVideoHeight + 20 + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) * 2;;
	}
	MoveWindow(rc);
	return TRUE;
}


void CDlgFlvPlayer::OnWindowMaximizedChange(BOOL bIsMaximized){
	__super::OnWindowMaximizedChange(bIsMaximized);
}


void CDlgFlvPlayer::OnTimer(UINT_PTR nIDEvent){
	__super::OnTimer(nIDEvent);
}


void CDlgFlvPlayer::OnMouseMove(UINT nFlags, CPoint point){
	static CPoint lastPoint;
	if (mReady) {
		if (abs(point.x - lastPoint.x) > 5 || abs(point.y - lastPoint.y) > 5) {
			m_playerBar->ShowWindow(SW_SHOW);
		}
		lastPoint = point;
	}
	__super::OnMouseMove(nFlags, point);
}


void CDlgFlvPlayer::OnLButtonUp(UINT nFlags, CPoint point){
	if(m_playerBar->IsWindowVisible())
		m_playerBar->ShowWindow(SW_HIDE);
	else
		m_playerBar->ShowWindow(SW_SHOW);
	__super::OnLButtonUp(nFlags, point);
}
void CDlgFlvPlayer::OnSize(UINT nType, int cx, int cy){
	__super::OnSize(nType, cx, cy);
	CRect rc;
	GetClientRect(rc);
	if (IsWindow(m_myStatic.GetSafeHwnd()))
		m_myStatic.MoveWindow(rc);
	GetWindowRect(rc);
	if(IsWindow(m_playerBar->GetSafeHwnd()))
		AdjPlayBarPos(rc);
}


void CDlgFlvPlayer::OnMoving(UINT fwSide, LPRECT pRect){
	__super::OnMoving(fwSide, pRect);
	AdjPlayBarPos(pRect);
}

void CDlgFlvPlayer::AdjPlayBarPos(LPRECT pRect) {
	CRect rc;
	m_playerBar->GetWindowRect(rc);
	int x = (pRect->right - pRect->left - rc.Width()) / 2 + pRect->left;
	int y = pRect->bottom - rc.Height() - 30;
	m_playerBar->SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

}