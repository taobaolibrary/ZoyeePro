#include "stdafx.h"
#include "SocketServer.h"
#include "SocketBase.h"

CContext* CSocketServer::Init( const char* pDesc, int nProtocol /*= TCP*/ )
{
	if (bIsInit)
	{
		m_pContext->pszBuff = "�Ѿ���ʼ����, �޷��ٴγ�ʼ��";
		m_pContext->CalcBuffStrLen();
		m_pCallback->OnMSG(m_pContext);
		return m_pContext;
	}
	sockSrv = sockCli = INVALID_SOCKET;
	CContext* pContext = new CContext;
	pContext->_s = CSocketBase::Instance()->CreateConnection(pDesc, true, nProtocol);
	if (pContext->_s != INVALID_SOCKET)
	{
		pContext->emAction = ON_SYSTEM;
		pContext->pszBuff = "�˿��Ѿ�����!";
		pContext->CalcBuffStrLen();
		sockSrv = pContext->_s;
		m_pCallback->OnMSG(pContext);
		m_pContext = pContext;

		m_phThread = new HANDLE[1];
		m_nThreadSize = 1;
		m_phThread[0] = CreateThread(0, 0, workThread, this, 0, 0);
		return pContext;
	}
	return nullptr;
}

int CSocketServer::UnInit()
{
	if (sockCli != INVALID_SOCKET)
	{
		closesocket(sockCli);
	}
	if (sockSrv != INVALID_SOCKET)
	{
		closesocket(sockSrv);
	}
	
	if (m_phThread[0] != 0)
	{
	}
	//TerminateThread()

	return 0;
}

int CSocketServer::Send( const char* pszbuff, const int nLen, CContext* pContext )
{
	return send(pContext->_c, pszbuff, nLen, 0);	
}

CContext* CSocketServer::Connect( CContext* pDesc )
{
	printNoSurport("CSocketServer::Connect");
	return nullptr;
}

int CSocketServer::DisConnect()
{
	return 0;
}

int CSocketServer::DisConnect( CContext* pContext )
{
	printNoSurport("CSocketServer::DisConnect");
	return 0;
}

DWORD WINAPI CSocketServer::workThread( void *pParam )
{
	CSocketServer* pThis = (CSocketServer*)pParam;
	sockaddr remoteSockaddr;
	int nSize = sizeof(remoteSockaddr);
	int nLen = 0;
	char szBuff[1024] = {0};
	while (true)
	{
		pThis->sockCli = accept(pThis->sockSrv, &remoteSockaddr,  &nSize);
		if (pThis->sockCli == INVALID_SOCKET)
		{
			DWORD dw = GetLastError();
			Sleep(10);
			continue;
		}

		pThis->m_pContext->_c = pThis->sockCli;
		pThis->m_pContext->_s = pThis->sockSrv;
		while (true)
		{
			nLen = recv(pThis->sockCli, szBuff, 1024, 0);
			if (nLen <= 0)
			{
				closesocket(pThis->sockCli);
				pThis->sockCli = INVALID_SOCKET;
				break;
			}
			szBuff[nLen] = 0;
			pThis->m_pContext->pszBuff = szBuff;
			pThis->m_pContext->nLen = nLen;
			pThis->m_pCallback->OnMSG(pThis->m_pContext);
		}
	}
	return 0;
}

CSocketServer::CSocketServer()
{
	bIsInit = false;
}