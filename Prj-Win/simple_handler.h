// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <list>

class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
	public CefJSDialogHandler,
                      public CefLoadHandler {
 public:
	 HWND m_hWndDlg;
  explicit SimpleHandler(bool use_views);
  ~SimpleHandler();

  // Provide access to the single global instance of this object.
  static SimpleHandler* GetInstance();

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) OVERRIDE;

  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;


  //CefJSDialogHandler
  virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
	  const CefString& origin_url,
	  JSDialogType dialog_type,
	  const CefString& message_text,
	  const CefString& default_prompt_text,
	  CefRefPtr<CefJSDialogCallback> callback,
	  bool& suppress_message) OVERRIDE;

  // CefClient methods
  // Return the handler for JavaScript dialogs. If no handler is provided the
  // default implementation will be used.

  virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() {
	  return this;
  }


  // Request that all existing browser windows close.
  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

public:
	CefRefPtr<CefBrowser> GetBrowser() {
		if (!browser_list_.empty())
			return *browser_list_.begin();
		else
			return nullptr;
	}

	CefRefPtr<CefBrowser> GetBrowser(HWND hWnd) {
		if (!browser_list_.empty()) {
			BrowserList::iterator iter;
			for (iter = browser_list_.begin(); iter != browser_list_.end(); iter++) {
				HWND hWnd1 = (*iter)->GetHost()->GetWindowHandle();
				HWND hWnd2 = GetParent(hWnd1);
				if (hWnd == hWnd2) {
					return *iter;
				}
			}
			//return *browser_list_.begin();
			return nullptr;
		}
		else
			return nullptr;
	}

 private:
  // Platform-specific implementation.
  void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                           const CefString& title);

  // True if the application is using the Views framework.
  const bool use_views_;

  // List of existing browser windows. Only accessed on the CEF UI thread.
  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
