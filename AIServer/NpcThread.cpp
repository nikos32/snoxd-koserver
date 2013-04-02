#include "stdafx.h"
#include "NpcThread.h"
#include "Npc.h"

#define DELAY				250

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
UINT NpcThreadProc(LPVOID pParam /* NPC_THREAD_INFO ptr */)
{
	NPC_THREAD_INFO*	pInfo	= (NPC_THREAD_INFO*)pParam;
	CNpc*				pNpc	= NULL;

	int					i			= 0;
	DWORD				dwDiffTime	= 0;
	time_t				dwTickTime  = 0;
	srand((uint32)UNIXTIME);
	myrand( 1, 10000 ); myrand( 1, 10000 );

	time_t fTime2 = 0;
	int    duration_damage=0;

	if(!pInfo) return 0;

	while(!g_bNpcExit)
	{
		fTime2 = getMSTime();

		for(i = 0; i < NPC_NUM; i++)
		{
			pNpc = pInfo->pNpc[i];
			if( !pNpc ) continue;
			//if((pNpc->m_proto->m_tNpcType == NPCTYPE_DOOR || pNpc->m_proto->m_tNpcType == NPCTYPE_ARTIFACT || pNpc->m_proto->m_tNpcType == NPCTYPE_PHOENIX_GATE || pNpc->m_proto->m_tNpcType == NPCTYPE_GATE_LEVER) && !pNpc->m_bFirstLive) continue;
			//if( pNpc->m_bFirstLive ) continue;
			if( pNpc->m_sNid < 0 ) continue;		// �߸��� ���� (�ӽ��ڵ� 2002.03.24)

			dwTickTime = fTime2 - pNpc->m_fDelayTime;

			if(pNpc->m_Delay > (int)dwTickTime && !pNpc->m_bFirstLive && pNpc->m_Delay != 0) 
			{
				if(pNpc->m_Delay < 0) pNpc->m_Delay = 0;

				//���߽߰�... (2002. 04.23����, �������̱�)
				if(pNpc->m_NpcState == NPC_STANDING && pNpc->CheckFindEnermy() )	{
					if( pNpc->FindEnemy() )	{
						pNpc->m_NpcState = NPC_ATTACKING;
						pNpc->m_Delay = 0;
					}
				}
				continue;
			}	
			
			dwTickTime = fTime2 - pNpc->m_fHPChangeTime;
			if( 10000 < dwTickTime )	{	// 10�ʸ��� HP�� ȸ�� �����ش�
				pNpc->HpChange();
			}

			pNpc->DurationMagic_4();		// ���� ó��...
			pNpc->DurationMagic_3();		// ���Ӹ���..

			uint8 bState = pNpc->m_NpcState;
			time_t tDelay = -1;
			switch (bState)
			{
			case NPC_LIVE:					// ��� ��Ƴ� ���
				tDelay = pNpc->NpcLive();
				break;

			case NPC_STANDING:						// �ϴ� �� ���� ���ִ� ���
				tDelay = pNpc->NpcStanding();
				break;
			
			case NPC_MOVING:
				tDelay = pNpc->NpcMoving();
				break;

			case NPC_ATTACKING:
				tDelay = pNpc->NpcAttacking();
				break;

			case NPC_TRACING:
				tDelay = pNpc->NpcTracing();
				break;

			case NPC_FIGHTING:
				tDelay = pNpc->NpcFighting();
				break;

			case NPC_BACK:
				tDelay = pNpc->NpcBack();
				break;

			case NPC_STRATEGY:
				break;

			case NPC_DEAD:
				pNpc->m_NpcState = NPC_LIVE;
				break;

			case NPC_SLEEPING:
				tDelay = pNpc->NpcSleeping();
				break;

			case NPC_FAINTING:
				tDelay = pNpc->NpcFainting();
				break;

			case NPC_HEALING:
				tDelay = pNpc->NpcHealing();
				break;
			}

			// This may not be necessary, but it keeps behaviour identical.
			if (bState != NPC_LIVE && bState != NPC_DEAD
				&& pNpc->m_NpcState != NPC_DEAD)
				pNpc->m_fDelayTime = getMSTime();

			if (tDelay >= 0)
				pNpc->m_Delay = tDelay;
		}	

		Sleep(100);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
UINT ZoneEventThreadProc(LPVOID pParam /* = NULL */)
{
	CServerDlg* m_pMain = (CServerDlg*) pParam;
	int j=0;

	while (!g_bNpcExit)
	{
		foreach_stlmap (itr, g_pMain->g_arZone)
		{
			MAP *pMap = itr->second;
			if (pMap == NULL
				|| pMap->m_byRoomEvent == 0
				|| pMap->IsRoomStatusCheck()) 
				continue;

			for (j = 1; j < pMap->m_arRoomEventArray.GetSize() + 1; j++)
			{
				CRoomEvent* pRoom = pMap->m_arRoomEventArray.GetData(j);
				if( !pRoom ) continue;
				if( pRoom->m_byStatus == 1 || pRoom->m_byStatus == 3 )   continue; // 1:init, 2:progress, 3:clear
				// ���⼭ ó���ϴ� ����...
				pRoom->MainRoom();
			}
		}
		Sleep(1000);	// 1�ʴ� �ѹ�
	}

	return 0;
}

CNpcThread::CNpcThread()
{
	m_hThread = NULL;
	m_sThreadNumber = -1;

	memset(&m_pNpc, 0, sizeof(m_pNpc));
}

CNpcThread::~CNpcThread()
{
/*	for( int i = 0; i < NPC_NUM; i++ )
	{
		if(m_pNpc[i])
		{
			delete m_pNpc[i];
			m_pNpc[i] = NULL;
		}
	}	*/
}

void CNpcThread::InitThreadInfo(HWND hwnd)
{
	m_ThreadInfo.hWndMsg	= hwnd;
}