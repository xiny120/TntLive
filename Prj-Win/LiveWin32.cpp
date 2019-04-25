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
#include "LiveWin32.h"
#include "LiveWin32Dlg.h"
#include "xdefines.h"

#include "simple_app.h"

// When generating projects with CMake the CEF_USE_SANDBOX value will be defined
// automatically if using the required compiler version. Pass -DUSE_SANDBOX=OFF
// to the CMake command-line to disable use of the sandbox.
// Uncomment this line to manually enable sandbox support.
// #define CEF_USE_SANDBOX 1

#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library may not link successfully with all VS
// versions.
#pragma comment(lib, "cef_sandbox.lib")
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLiveWin32App

BEGIN_MESSAGE_MAP(CLiveWin32App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

int gUserId = 0;

// CLiveWin32App 构造

CLiveWin32App::CLiveWin32App()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CLiveWin32App 对象

CLiveWin32App theApp;
std::string m_baseurl = "http://210.192.107.62:8080/";


CString GetModuleDir()
{
	HMODULE module = GetModuleHandle(0);
	TCHAR pFileName[MAX_PATH];
	GetModuleFileName(module, pFileName, MAX_PATH);

	CString csFullPath(pFileName);
	int nPos = csFullPath.ReverseFind(_T('\\'));
	if (nPos < 0)
		return CString("");
	else
		return csFullPath.Left(nPos);
}

CString GetWorkDir()
{
	TCHAR pFileName[MAX_PATH];
	int nPos = GetCurrentDirectory(MAX_PATH, pFileName);

	CString csFullPath(pFileName);
	if (nPos < 0)
		return CString("");
	else
		return csFullPath;
}


// CLiveWin32App 初始化

BOOL CLiveWin32App::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	if (CPullDlgData::me() == NULL)
		AfxMessageBox(L"错误0");


	CString work = GetModuleDir();

	CString work1 = GetWorkDir();


	int resid[] = { IDR_BIN1, IDR_BIN2, IDR_BIN3, IDR_BIN4, IDR_BIN5,
	IDR_BIN6, IDR_BIN7, IDR_BIN8, IDR_BIN9, IDR_BIN10 };

	for (int i = 0; i < 10; i++)
	{
		//定位我们的自定义资源，这里因为我们是从本模块定位资源，所以将句柄简单地置为NULL即可
		HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(resid[i]), TEXT("BIN"));
		if (NULL != hRsrc)
		{
			//获取资源的大小
			m_iSoundMarker[i] = SizeofResource(NULL, hRsrc) * 3;
			if (0 != m_iSoundMarker[i])
			{
				//加载资源
				HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
				if (NULL != hGlobal)
				{
					//锁定资源
					short* pRes = (short*)LockResource(hGlobal);
					m_soundMarker[i] = (short*)new char[m_iSoundMarker[i]];
					int z = 0;
					for (int j = 0; j < m_iSoundMarker[i] / 3 / 2; j++)
					{
						m_soundMarker[i][z] = pRes[j] * 0.39f;
						z++;
						m_soundMarker[i][z] = pRes[j] * 0.39f;
						z++;
						m_soundMarker[i][z] = pRes[j] * 0.39f;
						z++;
						//m_soundMarker[i][z] = pRes[j] / 20;
						//z++;
					}
				}
			}
		}
	}
	OutputDebugString(_T("m_iSoundMarker"));



	//AfxMessageBox(work + "\r\n" + work1);

	// Enable High-DPI support on Windows 7 or newer.
	CefEnableHighDPISupport();

	void* sandbox_info = NULL;
	/*
#if defined(CEF_USE_SANDBOX)
	// Manage the life span of the sandbox information object. This is necessary
	// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif
	*/
	//CefRefPtr<SimpleApp> app(new SimpleApp);
	app = (new SimpleApp);
	handler = (new SimpleHandler(false));
	// Provide CEF with command-line arguments.
	//CefMainArgs main_args(this->m_hInstance);
	main_args = CefMainArgs(this->m_hInstance);

	// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
	// that share the same executable. This function checks the command-line and,
	// if this is a sub-process, executes the appropriate logic.
	int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return exit_code;
	}
	

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));





	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CLiveWin32Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}



int CLiveWin32App::ExitInstance()
{
	// TODO: 在此添加专用代码和/或调用基类
	
	return CWinApp::ExitInstance();
}
