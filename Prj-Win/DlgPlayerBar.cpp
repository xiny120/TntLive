// DlgPlayerBar.cpp: 实现文件
//

#include "stdafx.h"
#include "LiveWin32.h"
#include "DlgPlayerBar.h"
#include "afxdialogex.h"

#define IDT_HIDEME	1

// CDlgPlayerBar 对话框

IMPLEMENT_DYNAMIC(CDlgPlayerBar, CDialogEx)

CDlgPlayerBar::CDlgPlayerBar(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_PLAYERBAR, pParent)
{

}

CDlgPlayerBar::~CDlgPlayerBar()
{
}

void CDlgPlayerBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPlayerBar, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPlayerBar::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPlayerBar::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CDlgPlayerBar 消息处理程序


void CDlgPlayerBar::OnBnClickedOk(){
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CDlgPlayerBar::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
}


void CDlgPlayerBar::OnTimer(UINT_PTR nIDEvent){
	switch (nIDEvent)
	{
	case IDT_HIDEME:
		if (IsWindowVisible() && ((time(NULL) - mLastActive) > 5)) {
			ShowWindow(SW_HIDE);
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CDlgPlayerBar::OnShowWindow(BOOL bShow, UINT nStatus){
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow) {
		mLastActive = time(NULL);
	}
}


BOOL CDlgPlayerBar::OnInitDialog(){
	CDialogEx::OnInitDialog();
	SetClassLong(this->m_hWnd, GCL_STYLE, GetClassLong(this->m_hWnd, GCL_STYLE) | CS_DROPSHADOW);
	mLastActive = time(NULL);
	SetTimer(IDT_HIDEME, 500, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
