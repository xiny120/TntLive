#pragma once
#include <list>
#include <string>
#include <queue>
#include <mutex>
#include "include/cef_client.h"

#define WM_PULLDLG WM_USER + 0x100
#define WM_PULLDLG_RESIZE WM_USER + 0x101
#define WM_PULLDLG_SEEKTO	WM_USER + 0x102
#define WM_DLGFLVPLAYER_PLAY WM_USER + 0x103

class CPullDlgData {
private:
	CPullDlgData() {

	}

	BOOL Init() {
		return TRUE;
	}

	std::queue<CefString> m_queue;

	std::recursive_mutex m_mutex;

public:
	static CPullDlgData* me() {
		static CPullDlgData * pData = NULL;
		if (pData == NULL) {
			pData = new CPullDlgData();
			if (!pData->Init()) {
				delete pData;
				pData = NULL;
			}		
		}
		return pData;
	}

	void push(CefString str) {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_queue.push(str);
	}

	CefString pop() {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		CefString str;
		while (!m_queue.empty()) {
			str = m_queue.front();
			m_queue.pop();

		}
		return str;
	}
};