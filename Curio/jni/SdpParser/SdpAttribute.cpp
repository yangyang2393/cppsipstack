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

#ifndef ANDROID
#include "SipParserDefine.h"
#else
#include "../SipParser/SipParserDefine.h"
#endif

#include "SdpAttribute.h"

CSdpAttribute::CSdpAttribute()
{
}

CSdpAttribute::~CSdpAttribute()
{
}

int CSdpAttribute::Parse( const char * pszText, int iTextLen )
{
	Clear();
	if( pszText == NULL || iTextLen <= 0 ) return -1;

	int		iPos;

	for( iPos = 0; iPos < iTextLen; ++iPos )
	{
		if( pszText[iPos] == ':' )
		{
			m_strName.append( pszText, iPos );
			m_strValue.append( pszText + iPos + 1, iTextLen - ( iPos + 1 ) );
			break;
		}
	}

	if( m_strName.empty() )
	{
		m_strName.append( pszText, iTextLen );
	}

	return iTextLen;
}

int CSdpAttribute::ToString( char * pszText, int iTextSize )
{
	if( pszText == NULL || iTextSize <= 0 ) return -1;
	if( Empty() ) return -1;

	if( m_strValue.empty() )
	{
		return snprintf( pszText, iTextSize, "%s", m_strName.c_str() );
	}
	else
	{
		return snprintf( pszText, iTextSize, "%s:%s", m_strName.c_str(), m_strValue.c_str() );
	}
}

void CSdpAttribute::Clear()
{
	m_strName.clear();
	m_strValue.clear();
}

bool CSdpAttribute::Empty()
{
	if( m_strName.empty() ) return true;

	return false;
}