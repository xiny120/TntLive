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
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "tests/cefclient/browser/main_context_impl.h"
#include "tests/cefclient/browser/main_message_loop_multithreaded_win.h"
#include "tests/cefclient/browser/root_window_manager.h"
#include "tests/cefclient/browser/test_runner.h"
#include "tests/shared/browser/client_app_browser.h"
#include "tests/shared/browser/main_message_loop_external_pump.h"
#include "tests/shared/browser/main_message_loop_std.h"
#include "tests/shared/common/client_app_other.h"
#include "tests/shared/common/client_switches.h"
#include "tests/shared/renderer/client_app_renderer.h"

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


// CLiveWin32App 构造

CLiveWin32App::CLiveWin32App()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CLiveWin32App 对象



namespace client {
	namespace {

		int RunMain(HINSTANCE hInstance, int nCmdShow) {
			// Enable High-DPI support on Windows 7 or newer.
			CefEnableHighDPISupport();

			CefMainArgs main_args(hInstance);

			void* sandbox_info = NULL;

#if defined(CEF_USE_SANDBOX)
			// Manage the life span of the sandbox information object. This is necessary
			// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
			CefScopedSandboxInfo scoped_sandbox;
			sandbox_info = scoped_sandbox.sandbox_info();
#endif

			// Parse command-line arguments.
			CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
			command_line->InitFromString(::GetCommandLineW());

			// Create a ClientApp of the correct type.
			CefRefPtr<CefApp> app;
			ClientApp::ProcessType process_type = ClientApp::GetProcessType(command_line);
			if (process_type == ClientApp::BrowserProcess)
				app = new ClientAppBrowser();
			else if (process_type == ClientApp::RendererProcess)
				app = new ClientAppRenderer();
			else if (process_type == ClientApp::OtherProcess)
				app = new ClientAppOther();

			// Execute the secondary process, if any.
			int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
			if (exit_code >= 0)
				return exit_code;

			// Create the main context object.
			scoped_ptr<MainContextImpl> context(new MainContextImpl(command_line, true));

			CefSettings settings;

#if !defined(CEF_USE_SANDBOX)
			settings.no_sandbox = true;
#endif

			// Populate the settings based on command line arguments.
			context->PopulateSettings(&settings);

			// Create the main message loop object.
			scoped_ptr<MainMessageLoop> message_loop;
			if (settings.multi_threaded_message_loop)
				message_loop.reset(new MainMessageLoopMultithreadedWin);
			else if (settings.external_message_pump)
				message_loop = MainMessageLoopExternalPump::Create();
			else
				message_loop.reset(new MainMessageLoopStd);

			// Initialize CEF.
			context->Initialize(main_args, settings, app, sandbox_info);

			// Register scheme handlers.
			test_runner::RegisterSchemeHandlers();

			RootWindowConfig window_config;
			window_config.always_on_top = command_line->HasSwitch(switches::kAlwaysOnTop);
			window_config.with_controls =
				!command_line->HasSwitch(switches::kHideControls);
			window_config.with_osr = settings.windowless_rendering_enabled ? true : false;

			// Create the first window.
			context->GetRootWindowManager()->CreateRootWindow(window_config);

			// Run the message loop. This will block until Quit() is called by the
			// RootWindowManager after all windows have been destroyed.
			int result = message_loop->Run();

			// Shut down CEF.
			context->Shutdown();

			// Release objects in reverse order of creation.
			message_loop.reset();
			context.reset();

			return result;
		}

	}  // namespace
}  // namespace client


CLiveWin32App theApp;


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



	void* sandbox_info = NULL;
	CefMainArgs main_args(theApp.m_hInstance);
	CefRefPtr<client::ClientApp> app(new client::ClientApp);

	int exit_code = CefExecuteProcess(main_args, app.get(), sandbox_info);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return exit_code;
	}


	CefSettings settings;
	//CefSettingsTraits::init(&settings);
	//settings.single_process = true;
	settings.no_sandbox = true;
	settings.multi_threaded_message_loop = true;
	CefInitialize(main_args, settings, app.get(), sandbox_info);






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

