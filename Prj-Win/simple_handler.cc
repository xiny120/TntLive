// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"


#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_parser.h"

#include "xdefines.h"

extern int gUserId;
namespace {

SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler(bool use_views)
    : use_views_(use_views), is_closing_(false) {
  DCHECK(!g_instance);
  g_instance = this;
}

SimpleHandler::~SimpleHandler() {
  g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
  return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  CEF_REQUIRE_UI_THREAD();

  if (use_views_) {
    // Set the title of the window using the Views framework.
    CefRefPtr<CefBrowserView> browser_view =
        CefBrowserView::GetForBrowser(browser);
    if (browser_view) {
      CefRefPtr<CefWindow> window = browser_view->GetWindow();
      if (window)
        window->SetTitle(title);
    }
  } else {
    // Set the title of the window using platform APIs.
    PlatformTitleChange(browser, title);
  }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Add to the list of existing browsers.
  browser_list_.push_back(browser);
  HWND hWnd = browser->GetHost()->GetWindowHandle();
  ::PostMessage(hWnd,WM_SIZE, 0, 0);
  hWnd = GetParent(hWnd);
  ::PostMessage(hWnd, WM_SIZE, 0, 0);
  
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Closing the main window requires special handling. See the DoClose()
  // documentation in the CEF header for a detailed destription of this
  // process.
  if (browser_list_.size() == 1) {
    // Set a flag to indicate that the window close should be allowed.https://img-cdn-qiniu.dcloud.net.cn/tuku/img/yundong10.jpg
    is_closing_ = true;
  }

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();

  // Remove from the list of existing browsers.
  BrowserList::iterator bit = browser_list_.begin();
  for (; bit != browser_list_.end(); ++bit) {
    if ((*bit)->IsSame(browser)) {
      browser_list_.erase(bit);
      break;
    }
  }

  if (browser_list_.empty()) {
    // All browser windows have closed. Quit the application message loop.
    CefQuitMessageLoop();
  }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) {
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files.
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message.
  std::stringstream ss;
  ss << "<!DOCTYPE html><html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
     << std::string(failedUrl) << " with error " << std::string(errorText)
     << " (" << errorCode << ").</h2></body></html>";
  CefRefPtr<CefFrame> framem = browser->GetMainFrame();
  if (failedUrl == "http://localhost:8081/live/h5client/mainpage/#/")
	  frame->LoadURL("about:blank");
  else
	  framem->LoadURL("http://localhost:8081/live/h5client/mainpage/#/");
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
  if (!CefCurrentlyOn(TID_UI)) {
    // Execute on the UI thread.
    CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this,
                                   force_close));
    return;
  }

  if (browser_list_.empty())
    return;

  BrowserList::const_iterator it = browser_list_.begin();
  for (; it != browser_list_.end(); ++it)
    (*it)->GetHost()->CloseBrowser(force_close);
}


bool SimpleHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
	const CefString& origin_url,
	JSDialogType dialog_type,
	const CefString& message_text,
	const CefString& default_prompt_text,
	CefRefPtr<CefJSDialogCallback> callback,
	bool& suppress_message) {
	std::string str0 = message_text.ToString();
	

	CefRefPtr<CefValue> jsonObject = CefParseJSON(message_text, JSON_PARSER_ALLOW_TRAILING_COMMAS);
	if (jsonObject->IsValid())
	{
		CefRefPtr<CefDictionaryValue> dict = jsonObject->GetDictionary();
		CefString token = dict->GetString("cmd");
		if (token == "pulldlg") {
			CefRefPtr<CefDictionaryValue> data = dict->GetDictionary("data");
			CefString roomid = data->GetString("id");
			CefString pulluri = data->GetString("pulluri");
			CPullDlgData::me()->push(str0);

			data = dict->GetDictionary("ui");
			CefString sessionid = data->GetString("SessionId");
			CefString token = data->GetString("Token");
			gUserId = data->GetInt("UserId");

			
			
			::PostMessage(m_hWndDlg, WM_PULLDLG, 0, 0);
		}
	}


	return true;
}