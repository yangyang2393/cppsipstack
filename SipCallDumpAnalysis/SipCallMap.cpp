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

#include "SipCallMap.h"
#include "Log.h"
#include "RtpMap.h"
#include "SipCallDumpSetup.h"
#include "TimeString.h"
#include "MemoryDebug.h"

CSipCallMap gclsCallMap;

extern std::string gstrPhone;

CSipRtpInfo::CSipRtpInfo() : m_iPort(0)
{
}

CSipCallInfo::CSipCallInfo() : m_psttPcap(NULL), m_iInviteTime(0), m_iCancelTime(0), m_iStartTime(0), m_iByeTime(0)
{
}

/**
 * @ingroup SipCallDump
 * @brief SDP �޽����� RTP ������ �����Ѵ�.
 * @param pclsSdp			SDP �޽���
 * @param clsRtpInfo	RTP ����
 */
static void SetRtpInfo( CSdpMessage * pclsSdp, CSipRtpInfo & clsRtpInfo )
{
	bool bFound = false;
	CSdpMedia * pclsMedia;

	clsRtpInfo.m_strIp = pclsSdp->m_clsConnection.m_strAddr;

	if( pclsSdp->SelectMedia( "audio", &pclsMedia ) == false )
	{
		CLog::Print( LOG_ERROR, "%s audio media is not found in SDP", __FUNCTION__ );
		return;
	}

	clsRtpInfo.m_iPort = pclsMedia->m_iPort;
	
	// media �� connection ������ �����ϸ� �̸� RTP ������ IP �ּҷ� �����Ѵ�.
	if( pclsMedia->m_clsConnection.Empty() == false )
	{
		clsRtpInfo.m_strIp = pclsMedia->m_clsConnection.m_strAddr;
	}
}

/**
 * @ingroup SipCallDump
 * @brief SIP �޽������� SDP �޽����� �����´�.
 * @param clsMessage	SIP �޽���
 * @param clsSdp			[out] SDP �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
static bool GetSdp( CSipMessage & clsMessage, CSdpMessage & clsSdp )
{
	if( clsMessage.m_iContentLength > 0 )
	{
		if( clsMessage.m_clsContentType.IsEqual( "application", "sdp" ) )
		{
			if( clsSdp.Parse( clsMessage.m_strBody.c_str(), clsMessage.m_strBody.length() ) > 0 )
			{
				return true;
			}
		}
		else
		{
			CLog::Print( LOG_ERROR, "%s Content-Type is not application/sdp", __FUNCTION__ );
		}
	}

	return false;
}

CSipCallMap::CSipCallMap()
{
}

CSipCallMap::~CSipCallMap()
{
}

/**
 * @ingroup SipCallDump
 * @brief SIP ��Ŷ�� �����Ѵ�.
 * @param psttPcap		��Ŷ ĸó ���� pcap ����ü�� ������
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @param pszUdpBody	UDP body
 * @param iUdpBodyLen UDP body ����
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::Insert( pcap_t * psttPcap, struct pcap_pkthdr * psttHeader, const	u_char * pszData, const char * pszUdpBody, int iUdpBodyLen )
{
	CSipMessage clsMessage;

	CLog::Print( LOG_NETWORK, "Sip[%.*s]", iUdpBodyLen, pszUdpBody );

	if( clsMessage.Parse( pszUdpBody, iUdpBodyLen ) == false )
	{
		CLog::Print( LOG_ERROR, "sip parse error" );
		return false;
	}

	return Insert( psttPcap, psttHeader, pszData, &clsMessage );
}

/**
 * @ingroup SipCallDump
 * @brief SIP ��Ŷ�� �����Ѵ�.
 * @param psttPcap		��Ŷ ĸó ���� pcap ����ü�� ������
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @param pclsMessage	SIP �޽��� ��ü�� ������
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::Insert( pcap_t * psttPcap, struct pcap_pkthdr * psttHeader, const u_char * pszData, CSipMessage * pclsMessage )
{
	if( pclsMessage->IsRequest() )
	{
		if( pclsMessage->IsMethod( "INVITE" ) )
		{
			CSdpMessage clsSdp;

			if( GetSdp( *pclsMessage, clsSdp ) == false )
			{
				//CLog::Print( LOG_ERROR, "%s INVITE no SDP", __FUNCTION__ );
				return false;
			}

			InsertInvite( psttPcap, psttHeader, pszData, pclsMessage, &clsSdp );
		}
		else
		{
			Insert( psttHeader, pszData, pclsMessage );
		}
	}
	else
	{
		if( pclsMessage->IsMethod( "INVITE" ) )
		{
			CSdpMessage clsSdp;

			if( GetSdp( *pclsMessage, clsSdp ) == false )
			{
				Insert( psttHeader, pszData, pclsMessage );
			}
			else
			{
				InsertInviteResponse( psttHeader, pszData, pclsMessage, &clsSdp );
			}
		}
		else
		{
			Insert( psttHeader, pszData, pclsMessage );
		}
	}

	return true;
}

/**
 * @ingroup SipCallDump
 * @brief RTP ��Ŷ�� �����Ѵ�.
 * @param pszCallId		SIP Call-ID
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::Insert( const char * pszCallId, struct pcap_pkthdr * psttHeader, const u_char * pszData )
{
	SIP_CALL_MAP::iterator itMap;
	bool bRes = false;

	m_clsMutex.acquire();
	itMap = m_clsMap.find( pszCallId );
	if( itMap != m_clsMap.end() )
	{
		bRes = true;
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup SipCallDump
 * @brief ��ȭ ������ �����Ѵ�.
 * @param pszCallId SIP Call-ID
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::Delete( const char * pszCallId )
{
	SIP_CALL_MAP::iterator itMap;
	bool bRes = false;

	m_clsMutex.acquire();
	itMap = m_clsMap.find( pszCallId );
	if( itMap != m_clsMap.end() )
	{
		Erase( itMap );
		bRes = true;
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup SipCallDump
 * @brief ��� ��ȭ ������ �����Ѵ�.
 */
void CSipCallMap::DeleteAll( )
{
	SIP_CALL_MAP::iterator itMap, itNext;

	m_clsMutex.acquire();
	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		gclsRtpMap.Delete( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort );
		gclsRtpMap.Delete( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );
	}
	m_clsMap.clear();
	m_clsMutex.release();
}

/**
 * @ingroup SipCallDump
 * @brief SIP INVITE ��û �޽��� ������ �����Ѵ�.
 * @param psttPcap		��Ŷ ĸó ���� pcap ����ü�� ������
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @param pclsMessage SIP �޽���
 * @param pclsSdp			SDP �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::InsertInvite( pcap_t * psttPcap, struct pcap_pkthdr * psttHeader, const u_char * pszData, CSipMessage * pclsMessage, CSdpMessage * pclsSdp )
{
	SIP_CALL_MAP::iterator itMap;
	std::string strCallId;

	if( pclsMessage->GetCallId( strCallId ) == false )
	{
		CLog::Print( LOG_ERROR, "%s pclsMessage->GetCallId() error", __FUNCTION__ );
		return false;
	}

	if( gstrPhone.empty() == false )
	{
		if( strcmp( pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str(), gstrPhone.c_str() ) && strcmp( pclsMessage->m_clsTo.m_clsUri.m_strUser.c_str(), gstrPhone.c_str() ) )
		{
			return false;
		}
	}

	m_clsMutex.acquire();
	itMap = m_clsMap.find( strCallId );
	if( itMap == m_clsMap.end() )
	{
		CSipCallInfo clsCallInfo;

		clsCallInfo.m_strFromId = pclsMessage->m_clsFrom.m_clsUri.m_strUser;
		clsCallInfo.m_strToId = pclsMessage->m_clsTo.m_clsUri.m_strUser;

		SetRtpInfo( pclsSdp, clsCallInfo.m_clsFromRtp );
		time( &clsCallInfo.m_iInviteTime );

		m_clsMap.insert( SIP_CALL_MAP::value_type( strCallId, clsCallInfo ) );
	}
	else
	{
		CSipRtpInfo clsRtpInfo;

		SetRtpInfo( pclsSdp, clsRtpInfo );

		if( !strcmp( itMap->second.m_strFromId.c_str(), pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str() ) )
		{
			if( itMap->second.m_clsFromRtp.Equal( clsRtpInfo ) == false )
			{
				char szLog[1024];

				snprintf( szLog, sizeof(szLog), "caller rtp change (%s:%d) => (%s:%d)\n", itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, clsRtpInfo.m_strIp.c_str(), clsRtpInfo.m_iPort );
				itMap->second.m_strLog.append( szLog );

				gclsRtpMap.Delete( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort );
				itMap->second.m_clsFromRtp = clsRtpInfo;
				gclsRtpMap.Insert( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, strCallId.c_str() );
			}
		}
		else
		{
			if( itMap->second.m_clsToRtp.Equal( clsRtpInfo ) == false )
			{
				char szLog[1024];

				snprintf( szLog, sizeof(szLog), "callee rtp change (%s:%d) => (%s:%d)\n", itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, clsRtpInfo.m_strIp.c_str(), clsRtpInfo.m_iPort );
				itMap->second.m_strLog.append( szLog );

				gclsRtpMap.Delete( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );
				itMap->second.m_clsToRtp = clsRtpInfo;
				gclsRtpMap.Insert( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, strCallId.c_str() );
			}
		}
	}
	m_clsMutex.release();

	return true;
}

/**
 * @ingroup SipCallDump
 * @brief SIP INVITE ���� �޽��� ������ �����Ѵ�.
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @param pclsMessage SIP �޽���
 * @param pclsSdp			SDP �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::InsertInviteResponse( struct pcap_pkthdr * psttHeader, const u_char * pszData, CSipMessage * pclsMessage, CSdpMessage * pclsSdp )
{
	SIP_CALL_MAP::iterator itMap;
	std::string strCallId;

	if( pclsMessage->GetCallId( strCallId ) == false )
	{
		CLog::Print( LOG_ERROR, "%s pclsMessage->GetCallId() error", __FUNCTION__ );
		return false;
	}

	m_clsMutex.acquire();
	itMap = m_clsMap.find( strCallId );
	if( itMap != m_clsMap.end() )
	{
		if( itMap->second.m_iStartTime == 0 )
		{
			// ReINVITE ������ ó������ �ʴ´�.
			time( &itMap->second.m_iStartTime );

			if( itMap->second.m_clsToRtp.m_iPort )
			{
				gclsRtpMap.Delete( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );
			}

			SetRtpInfo( pclsSdp, itMap->second.m_clsToRtp );

			gclsRtpMap.Insert( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, strCallId.c_str() );
			gclsRtpMap.Insert( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, strCallId.c_str() );
		}
		else
		{
			CSipRtpInfo clsRtpInfo;

			SetRtpInfo( pclsSdp, clsRtpInfo );

			if( !strcmp( itMap->second.m_strFromId.c_str(), pclsMessage->m_clsFrom.m_clsUri.m_strUser.c_str() ) )
			{
				if( itMap->second.m_clsToRtp.Equal( clsRtpInfo ) == false )
				{
					char szLog[1024];

					snprintf( szLog, sizeof(szLog), "callee rtp change (%s:%d) => (%s:%d)\n", itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, clsRtpInfo.m_strIp.c_str(), clsRtpInfo.m_iPort );
					itMap->second.m_strLog.append( szLog );

					gclsRtpMap.Delete( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );
					itMap->second.m_clsToRtp = clsRtpInfo;
					gclsRtpMap.Insert( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, strCallId.c_str() );
				}
			}
			else
			{
				if( itMap->second.m_clsFromRtp.Equal( clsRtpInfo ) == false )
				{
					char szLog[1024];

					snprintf( szLog, sizeof(szLog), "caller rtp change (%s:%d) => (%s:%d)\n", itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, clsRtpInfo.m_strIp.c_str(), clsRtpInfo.m_iPort );
					itMap->second.m_strLog.append( szLog );

					gclsRtpMap.Delete( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort );
					itMap->second.m_clsFromRtp = clsRtpInfo;
					gclsRtpMap.Insert( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, strCallId.c_str() );
				}
			}
		}
	}
	m_clsMutex.release();

	return true;
}

/**
 * @ingroup SipCallDump
 * @brief INVITE ��û/������ ������ SIP ��Ŷ�� �����Ѵ�.
 * @param psttHeader	��Ŷ ĸó�� ���
 * @param pszData			��Ŷ ĸó�� ��Ŷ
 * @param pclsMessage SIP �޽���
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CSipCallMap::Insert( struct pcap_pkthdr * psttHeader, const u_char * pszData, CSipMessage * pclsMessage )
{
	SIP_CALL_MAP::iterator itMap;
	std::string strCallId;
	bool bRes = false;

	if( pclsMessage->GetCallId( strCallId ) == false )
	{
		CLog::Print( LOG_ERROR, "%s pclsMessage->GetCallId() error", __FUNCTION__ );
		return false;
	}

	m_clsMutex.acquire();
	itMap = m_clsMap.find( strCallId );
	if( itMap != m_clsMap.end() )
	{
		bool bDelete = false;

		if( pclsMessage->IsRequest() )
		{
			if( pclsMessage->IsMethod( "CANCEL" ) )
			{
				time( &itMap->second.m_iCancelTime );
			}
			else if( pclsMessage->IsMethod( "ACK" ) )
			{
				if( itMap->second.m_iCancelTime )
				{
					bDelete = true;
				}
			}
			else if( pclsMessage->IsMethod( "BYE" ) )
			{
				time( &itMap->second.m_iByeTime );
			}
		}
		else
		{
			if( pclsMessage->IsMethod( "BYE" ) )
			{
				bDelete = true;
			}
		}
		
		if( bDelete )
		{
			Erase( itMap );
		}

		bRes = true;
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup SipCallDump
 * @brief �ڷᱸ������ �����Ѵ�.
 * @param itMap ���� ��� iterator
 */
void CSipCallMap::Erase( SIP_CALL_MAP::iterator & itMap )
{
	CRtpInfo clsFromRtp, clsToRtp;

	if( gclsRtpMap.Select( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, clsFromRtp ) == false )
	{
		CLog::Print( LOG_ERROR, "from rtp(%s:%d) is not found", itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort );
	}

	if( gclsRtpMap.Select( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, clsToRtp ) == false )
	{
		CLog::Print( LOG_ERROR, "to rtp(%s:%d) is not found", itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );
	}

	printf( "callid[%s] from[%s] to[%s]\n", itMap->first.c_str(), itMap->second.m_strFromId.c_str(), itMap->second.m_strToId.c_str() );
	printf( " - rtp(%s:%d)=>(%s:%d) count(%d)\n", itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort
		, itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort, clsFromRtp.m_iRtpCount );
	printf( " - rtp(%s:%d)=>(%s:%d) count(%d)\n", itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort
		, itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort, clsToRtp.m_iRtpCount );

	if( itMap->second.m_strLog.empty() == false )
	{
		printf( " - log[%s]\n", itMap->second.m_strLog.c_str() );
	}

	gclsRtpMap.Delete( itMap->second.m_clsFromRtp.m_strIp.c_str(), itMap->second.m_clsFromRtp.m_iPort );
	gclsRtpMap.Delete( itMap->second.m_clsToRtp.m_strIp.c_str(), itMap->second.m_clsToRtp.m_iPort );

	m_clsMap.erase( itMap );
}