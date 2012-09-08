/* 
 * Copyright (C) 2012 Yee Young Han <websearch@naver.com> (http://blog.naver.com/websearch)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "stdafx.h"
#include "KSipServerMonitor.h"
#include "LogInDlg.h"
#include "Setup.h"

// CLogInDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CLogInDlg, CDialog)

CLogInDlg::CLogInDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogInDlg::IDD, pParent)
	, m_iPort(6000)
	, m_iPeriod(1)
{

}

CLogInDlg::~CLogInDlg()
{
}

void CLogInDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_LIST, m_clsIpList);
	DDX_Text(pDX, IDC_PORT, m_iPort);
	DDX_Text(pDX, IDC_PERIOD, m_iPeriod);
}


BEGIN_MESSAGE_MAP(CLogInDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CLogInDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLogInDlg �޽��� ó�����Դϴ�.

BOOL CLogInDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	USES_CONVERSION;

	char	szKey[255];
	std::string	strValue;

	for( int i = 0; ; ++i )
	{
		_snprintf( szKey, sizeof(szKey), "server_%d", i );

		if( gclsSetup.GetString( szKey, strValue ) == false ) break;

		m_clsIpList.AddString( A2T( strValue.c_str() ) );
	}

	m_clsIpList.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CLogInDlg::OnBnClickedOk()
{
	int iCurSel = m_clsIpList.GetCurSel();
	if( iCurSel == LB_ERR )
	{
		CString	strIp;

		m_clsIpList.GetWindowText( strIp );
		m_clsIpList.AddString( strIp );

		m_strIp = strIp;
	}
	else
	{
		m_clsIpList.GetLBText( iCurSel, m_strIp );
	}

	SaveFile();

	OnOK();
}

void CLogInDlg::SaveFile()
{
	int i, iCount = m_clsIpList.GetCount();
	char	szKey[255];
	CString			strText;

	USES_CONVERSION;

	for( i = 0; i < iCount; ++i )
	{
		_snprintf( szKey, sizeof(szKey), "server_%d", i );
		m_clsIpList.GetLBText( i, strText );
		gclsSetup.PutString( szKey, T2A( strText ) );
	}
}