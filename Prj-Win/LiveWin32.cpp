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
int gLiving = 0;

// CLiveWin32App ����

CLiveWin32App::CLiveWin32App():CWinApp(L"����ֱ��"){
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	//CWinApp::CWinApp(L"helloabc");
}


// Ψһ��һ�� CLiveWin32App ����

CLiveWin32App theApp;
std::string m_baseurl = "http://gpk01.gwgz.com/";
std::string m_apiurl = "http://gpk01.gwgz.com:8092/";


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


// CLiveWin32App ��ʼ��

BOOL CLiveWin32App::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	if (CPullDlgData::me() == NULL)
		AfxMessageBox(L"����0");
	CString work = GetModuleDir();
	CString work1 = GetWorkDir();

	//First free the string allocated by MFC at CWinApp startup.
	//The string is allocated before InitInstance is called.
	free((void*)m_pszAppName);
	//Change the name of the application file.
	//The CWinApp destructor will free the memory.
	m_pszAppName = _tcsdup(_T("����ͨ��������"));


	int resid[] = { IDR_BIN1, IDR_BIN2, IDR_BIN3, IDR_BIN4, IDR_BIN5,
	IDR_BIN6, IDR_BIN7, IDR_BIN8, IDR_BIN9, IDR_BIN10 };

	for (int i = 0; i < 10; i++){
		//��λ���ǵ��Զ�����Դ��������Ϊ�����Ǵӱ�ģ�鶨λ��Դ�����Խ�����򵥵���ΪNULL����
		HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(resid[i]), TEXT("BIN"));
		if (NULL != hRsrc){
			//��ȡ��Դ�Ĵ�С
			m_iSoundMarker[i] = SizeofResource(NULL, hRsrc) * 4;
			if (0 != m_iSoundMarker[i]){
				//������Դ
				HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
				if (NULL != hGlobal){
					//������Դ
					short* pRes = (short*)LockResource(hGlobal);
					m_soundMarker[i] = (short*)new char[m_iSoundMarker[i]];
					int z = 0;
					for (int j = 0; j < m_iSoundMarker[i] / 4 / 2; j++){
						m_soundMarker[i][z] = pRes[j] * 0.03f;
						z++;
						m_soundMarker[i][z] = pRes[j] * 0.03f;
						z++;
						m_soundMarker[i][z] = pRes[j] * 0.03f;
						z++;
						m_soundMarker[i][z] = pRes[j] * 0.03f;
						z++;
						//m_soundMarker[i][z] = pRes[j] / 20;
						//z++;
					}
				}
			}
		}
	}
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
	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	TCHAR buf[1024];
	::GetCurrentDirectory(1024, buf);
	OutputDebugString(buf);

	CLiveWin32Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}



int CLiveWin32App::ExitInstance()
{
	// TODO: �ڴ����ר�ô����/����û���
	
	return CWinApp::ExitInstance();
}
