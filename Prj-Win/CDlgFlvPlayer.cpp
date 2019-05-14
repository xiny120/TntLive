// CDlgFlvPlayer.cpp: 实现文件
//

#include "stdafx.h"
#include "include/cef_parser.h"
#include "LiveWin32.h"
#include "CDlgFlvPlayer.h"
#include "afxdialogex.h"
#include "xdefines.h"


extern int gUserId;
extern std::string m_baseurl;
// CDlgFlvPlayer 对话框

IMPLEMENT_DYNAMIC(CDlgFlvPlayer, CDialogEx)

CDlgFlvPlayer::CDlgFlvPlayer(CWnd* pParent /*=nullptr*/,const std::string& info)
	: CDialogEx(IDD_DLG_FLVPLAYER, pParent)
	, minfo(info)
	, m_pPlayer(NULL)
{

}

CDlgFlvPlayer::~CDlgFlvPlayer()
{
}

void CDlgFlvPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFlvPlayer, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgFlvPlayer::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgFlvPlayer::OnBnClickedCancel)
	ON_MESSAGE(WM_PULLDLG_RESIZE, &OnPullDlgResize)
	ON_MESSAGE(WM_PULLDLG_SEEKTO, &CDlgFlvPlayer::OnPullDlgSeekto)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_DWMWINDOWMAXIMIZEDCHANGE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOVING()
END_MESSAGE_MAP()


// CDlgFlvPlayer 消息处理程序


void CDlgFlvPlayer::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CDlgFlvPlayer::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (AfxMessageBox(L"退出播放？", MB_YESNO) == IDYES) {
		if (m_pPlayer != nullptr) {
			m_pPlayer->StopRtmpPlay();
			RTMPGuester::Destory(m_pPlayer);
			m_pPlayer = NULL;
		}
		CDialogEx::OnCancel();
		
	}
}


BOOL CDlgFlvPlayer::OnInitDialog(){
	CDialogEx::OnInitDialog();
	m_myStatic.SubclassDlgItem(IDC_STATIC_VIDEO, this);
	m_playerBar = new CDlgPlayerBar(this);
	m_playerBar->Create(IDD_DLG_PLAYERBAR, this);
	m_playerBar->ShowWindow(SW_HIDE);

	delete[] m_pAudioMarker;
	CString strId;
	int id = 0;
	m_iAudioMarker = 0;
	strId.Format(_T("%d"), gUserId);
	TRACE(strId);
	TRACE("\r\n");
	for (int i = 0; i < strId.GetLength(); i++)
	{
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
	for (int i = 1; i < _countof(m_iAudioMarketStart); i++)
	{
		m_iAudioMarketStart[i] = m_iAudioMarketStart[i - 1] + m_iAudioMarketStart[i - 1] + (((double)rand() / (double)RAND_MAX) * RANGE_MAX + RANGE_MIN);
	}

	m_pAudioMarker = new char[m_iAudioMarker];
	m_pAudioMarketOut = (short*)m_pAudioMarker;
	memset(m_pAudioMarker, 0, m_iAudioMarker);
	char* pCur = m_pAudioMarker;
	for (int i = 0; i < strId.GetLength(); i++)
	{
		id = _tstoi(strId.Mid(i, 1));
		memcpy(pCur, theApp.m_soundMarker[id], theApp.m_iSoundMarker[id]);
		pCur += theApp.m_iSoundMarker[id] + 12000;
	}



	// TODO:  在此添加额外的初始化
	if (!minfo.empty()) {
		TRACE(minfo.c_str());
		CefRefPtr<CefValue> jsonObject = CefParseJSON(minfo, JSON_PARSER_ALLOW_TRAILING_COMMAS);
		if (jsonObject->IsValid()){
			CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
			CefString token = dict->GetString("cmd");
			if (token == "pulldlghis117" || token == "pulldlghisgp") {
				CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
				CefString createdate = data->GetString("CreateDate");
				CefString id = data->GetString("Id");
				CefString filepath = data->GetString("FilePath");
				int32_t enc = data->GetInt("Encryptioned");
				std::string fp(filepath);
				int lasti = fp.length();
				std::string parts[5];
				int idx = 0;
				//size_t pos = fp.rfind('\\');
				//pos = fp.rfind('/');
				for (int i = fp.length()-1; i>=0; i--) {
					if (fp[i] == '/' || fp[i] == '\\') {
						parts[idx] = fp.substr(i+1, lasti - i-1);
						lasti = i;
						idx++;
						if (idx > 4)
							break;
					}
				}

				std::string title;
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


				data = dict->GetDictionary("ui");
				CefString sessionid = data->GetString("SessionId");
				CefString token = data->GetString("Token");
				m_pPlayer = RTMPGuester::Create(*this);
				std::string url = "http://gpk01.gwgz.com:8862/";
				url = url + std::string(filepath);
				m_pPlayer->StartRtmpPlay(url.c_str(), GetDlgItem(IDC_STATIC_VIDEO)->GetSafeHwnd(), "flv", "", enc);
			}
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
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
	//设置对话框最小宽度与高度
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
	//PostMessage(WM_SIZE, 0, 0);
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
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	__super::OnWindowMaximizedChange(bIsMaximized);
}


void CDlgFlvPlayer::OnTimer(UINT_PTR nIDEvent){
	__super::OnTimer(nIDEvent);
}


void CDlgFlvPlayer::OnMouseMove(UINT nFlags, CPoint point){
	static CPoint lastPoint;
	if (abs(point.x - lastPoint.x) > 5 || abs(point.y - lastPoint.y) > 5) {
		m_playerBar->ShowWindow(SW_SHOW);
	}
	lastPoint = point;
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