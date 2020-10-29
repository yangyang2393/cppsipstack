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

#ifndef _SIP_PARAMETER_LIST_H_
#define _SIP_PARAMETER_LIST_H_

#include "SipParameter.h"

/** parameter 리스트 */
typedef std::list< CSipParameter > SIP_PARAMETER_LIST;

/**
 * @ingroup SipParser
 * @brief SIP parameter 리스트 저장 클래스
 */
class CSipParameterList
{
public:
	CSipParameterList();
	~CSipParameterList();

	int HeaderListParamParse( const char * pszText, int iTextLen );
	int ParamParse( const char * pszText, int iTextLen );
	int ParamToString( char * pszText, int iTextSize, char cSep = ';' );

	bool InsertParam( const char * pszName, const char * pszValue );
	bool UpdateParam( const char * pszName, const char * pszValue );
	bool SelectParam( const char * pszName, std::string & strValue );
	bool SelectParam( const char * pszName );
	const char * SelectParamValue( const char * pszName );

	void ClearParam();

	SIP_PARAMETER_LIST m_clsParamList;
};

#endif
