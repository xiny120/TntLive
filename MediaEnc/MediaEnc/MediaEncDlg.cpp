
// MediaEncDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MediaEnc.h"
#include "MediaEncDlg.h"
#include "afxdialogex.h"
#include <versionhelpers.h>


#ifdef __cplusplus  
extern "C" {
#endif  

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>

#ifdef __cplusplus  
}
#endif  

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMediaEncDlg 对话框



CMediaEncDlg::CMediaEncDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MEDIAENC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMediaEncDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_curTime);
}

BEGIN_MESSAGE_MAP(CMediaEncDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMediaEncDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMediaEncDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CMediaEncDlg 消息处理程序

BOOL CMediaEncDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	CTime tNow = CTime::GetCurrentTime();
	m_curTime.SetFormat(_T("yyyy-MM-dd HH:mm:ss"));
	m_curTime.SetTime(&tNow);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMediaEncDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMediaEncDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMediaEncDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void RunExe(CString in,CString opt,CString out){
	SECURITY_ATTRIBUTES sa = { sizeof(sa),NULL,TRUE };
	//SECURITY_ATTRIBUTES *psa = NULL;
	DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

	if(IsWindows7OrGreater()){
			dwShareMode |= FILE_SHARE_DELETE;
	}

	HANDLE hConsoleRedirect = CreateFile(L"c://NetStatus.txt",
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		&sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//
	ASSERT(hConsoleRedirect != INVALID_HANDLE_VALUE);
	STARTUPINFO stStartUpInfo;
	::memset(&stStartUpInfo, 0, sizeof(stStartUpInfo));
	stStartUpInfo.cb = sizeof(stStartUpInfo);
	stStartUpInfo.wShowWindow = SW_HIDE;
	stStartUpInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES | STARTF_FORCEONFEEDBACK;
	stStartUpInfo.hStdOutput = hConsoleRedirect;//将文件作为标准输出句柄  
	stStartUpInfo.hStdError = hConsoleRedirect;

	PROCESS_INFORMATION stProcessInfo;
	::memset(&stProcessInfo, 0, sizeof(stProcessInfo));
	//TCHAR szCmd[] = _T("\"ffmpeg -i \" http://community.csdn.net/");
	CString cmd;
	cmd.AppendFormat(L"ffmpeg -i \"%s\" -y %s  \"%s\"",in,opt,out);
	//cmd = L"ping 8.8.8.8";
	try	{
		bool bRet = ::CreateProcess(NULL,cmd.GetBuffer(),	NULL,NULL,	TRUE,NULL,
			NULL,NULL,&stStartUpInfo,&stProcessInfo);
		if (bRet){
			WaitForSingleObject(stProcessInfo.hProcess, 1000 * 60 * 5L);
			FlushFileBuffers(hConsoleRedirect);
			::CloseHandle(stProcessInfo.hProcess);
			::CloseHandle(stProcessInfo.hThread);
			stProcessInfo.hProcess = NULL;
			stProcessInfo.hThread = NULL;
			stProcessInfo.dwProcessId = 0;
			stProcessInfo.dwThreadId = 0;
		}else{
			DWORD dwErrCode = GetLastError();
			printf_s("ErrCode : %d\n", dwErrCode);
		}
	}catch (...){
	}

	CloseHandle(hConsoleRedirect);
}

void CMediaEncDlg::OnBnClickedOk(){
	CFileDialog fd(TRUE, 0, 0, OFN_NOCHANGEDIR);
	if (fd.DoModal() == IDOK) {
		CreateDirectory(L"./temp",NULL);
		CString strTemp0 = L"./temp/out0.mp4";
		RunExe(fd.GetPathName(), L" -pix_fmt yuv420p -c:v libx264 ", strTemp0);
		CString strTemp1 = L"./temp/out1.flv";
		RunExe(strTemp0, L" -vcodec copy ", strTemp1);
		::DeleteFile(strTemp0);
		CTime tNow;
		m_curTime.GetTime(tNow);
		CString strNow = tNow.Format(L"%d%H%M%S");
		
		CString strFileName = tNow.Format(L"%d%H%M%S000.flv");
		CString strFilePath = tNow.Format(L"live/livestream/%Y/%m/") + strFileName;
		strTemp0 = L"./temp/"+ strFileName;

		RunExe(strTemp1, L" -vcodec copy -c:a aac -ar 12000 ", strTemp0);
		::DeleteFile(strTemp1);
		::CopyFile(strTemp0, strTemp0 + ".bak", FALSE);
		CStringA a(strNow);
		char s = a.GetAt(5);
		if (s == 0)
			s = 1;
		CFile f;
		if (f.Open(strTemp0, CFile::modeReadWrite, NULL)) {
			char buf[1024] = { 0 };
			int readed = 0;
			while (readed < 1024) {
				int toread = 1024 - readed;
				int red = f.Read(&buf[readed],toread);
				if (red <= 0) {
					break;
				}
				readed += red;
			}
			if (readed == 1024) {
				for (int i = 0; i < 1024; i++) {
					buf[i] = buf[i] ^ s;
				}
				f.SeekToBegin();
				f.Write(&buf[512], 512);
				f.Write(&buf[0], 512);
			}
			f.Close();
		}
		CString strConn = L"Provider=SQLOLEDB.1;Password=tnt516516A;Persist Security Info=True;User ID=hds12204021;Initial Catalog=hds12204021_db;Data Source=hds12204021.my3w.com";
		_ConnectionPtr conn;
		HRESULT hr;
		try {
			hr = conn.CreateInstance("ADODB.Connection");
			if (SUCCEEDED(hr)) {
				hr = conn->Open(strConn.GetBuffer(), "", "", adModeUnknown);
				if (SUCCEEDED(hr)) {
					_variant_t RecordsAffected;
					CString strCmd;
					strCmd.AppendFormat(L"INSERT INTO [hds12204021_db].[dbo].[Web2019_historylist]"
						L"([RoomId], [CreateDate], [App], [Stream], [PublisherId], [FilePath], [FileName], [NickName]"
						L", [FileSize], [Encryptioned], [EncKey]) VALUES ("
						L"'%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,%d)",
						L"{2b7e7bfc-2730-49fe-ba43-a3e1043fcc13}", tNow.Format(L"%Y-%m-%d %H:%M:%S"), L"live", L"livestream",
						L"{36fe6a03-5572-420f-a45a-aa18f680db6d}", strFilePath, strFileName, strFileName,
						0, 1, s
					);
					hr = conn->Execute(strCmd.GetBuffer(), &RecordsAffected, adCmdText);
				}
			}
		}
		catch (_com_error& e1) {
			TRACE(e1.Description().operator LPCSTR());
			TRACE(e1.ErrorMessage());
		}
		

		AfxMessageBox(L"ok!");
	}
}


void CMediaEncDlg::OnBnClickedCancel(){
	if(AfxMessageBox(L"退出码？",MB_YESNO) == IDYES)
		CDialogEx::OnCancel();
}
