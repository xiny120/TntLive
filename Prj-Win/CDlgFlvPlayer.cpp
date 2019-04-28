// CDlgFlvPlayer.cpp: 实现文件
//

#include "stdafx.h"
#include "include/cef_parser.h"
#include "LiveWin32.h"
#include "CDlgFlvPlayer.h"
#include "afxdialogex.h"


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
		m_pPlayer->StopRtmpPlay();
		RTMPGuester::Destory(m_pPlayer);
		m_pPlayer = NULL;
		CDialogEx::OnCancel();
		
	}
}


BOOL CDlgFlvPlayer::OnInitDialog(){
	CDialogEx::OnInitDialog();

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
		CefRefPtr<CefValue> jsonObject = CefParseJSON(minfo, JSON_PARSER_ALLOW_TRAILING_COMMAS);
		if (jsonObject->IsValid()){
			CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
			CefString token = dict->GetString("cmd");
			if (token == "pulldlghis") {
				CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
				CefString id = data->GetString("Id");
				CefString filepath = data->GetString("FilePath");
				data = dict->GetDictionary("ui");
				CefString sessionid = data->GetString("SessionId");
				CefString token = data->GetString("Token");
				m_pPlayer = RTMPGuester::Create(*this);
				std::string url = "http://gpk01.gwgz.com:8862/";
				url = url + std::string(filepath);
				m_pPlayer->StartRtmpPlay(url.c_str(), GetDlgItem(IDC_STATIC_VIDEO)->GetSafeHwnd(), "flv", "");
			}
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgFlvPlayer::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类
	

	__super::PostNcDestroy();
}

void CDlgFlvPlayer::OnRtmplayerPlayStart() {
	TRACE("CDlgFlvPlayer::OnRtmplayerPlayStart\r\n");
}
void CDlgFlvPlayer::OnRtmplayerPlayStop() {
	TRACE("CDlgFlvPlayer::OnRtmplayerPlayStop\r\n");
}
void CDlgFlvPlayer::OnRtmplayer1stVideo() {
	TRACE("CDlgFlvPlayer::OnRtmplayer1stVideo\r\n");
}
void CDlgFlvPlayer::OnRtmplayer1stAudio() {
	TRACE("CDlgFlvPlayer::OnRtmplayer1stAudio\r\n");
}
void CDlgFlvPlayer::OnRtmplayerConnectionFailed(int a) {

}