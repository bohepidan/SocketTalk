#include "stdbohe.h"

void Connect(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2) {
	GroupList member_1 = malloc(sizeof(struct _grouP));
	GroupList member_2 = malloc(sizeof(struct _grouP));
	member_1->pmember = SocketInfo_1;
	member_2->pmember = SocketInfo_2;
	member_1->next = member_2;
	member_2->next = NULL;
	SocketInfo_2->grouphead = SocketInfo_1->grouphead;
	SocketInfo_1->grouphead->next = member_1;
}

void JoinIn(LPSOCKET_INFORMATION SocketInfo_1, LPSOCKET_INFORMATION SocketInfo_2) {
	GroupList newmember = malloc(sizeof(struct _grouP));
	newmember->next = NULL;
	newmember->pmember = SocketInfo_2;
	GroupList here = SocketInfo_1->grouphead->next;
	while (here->next) {
		here = here->next;
	}
	here->next = newmember;
	SocketInfo_2->grouphead->next = SocketInfo_1->grouphead->next;
}

int SwitchCommand(LPSOCKET_INFORMATION SocketInfo) {
	if (SocketInfo->situation == -1) {
		//���˾ܾ�������  ������Ҫ�û�������Ϣ�����Լ�
		SocketInfo->command = 5;
		SocketInfo->situation = 0;
		return 9;
	}
	if (SocketInfo->command == 0 && strcmp(SocketInfo->DataBuf.buf, "/register") != 0 && strcmp(SocketInfo->DataBuf.buf, "/sign in") != 0) {
		return 6;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/public group") == 0) {
		SocketInfo->situation = 0;
		return 14;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/quit") == 0) {
		SocketInfo->situation = 0;
		return 10;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/invite") == 0) {
		SocketInfo->situation = 0;
		return 3;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/join in") == 0) {
		SocketInfo->situation = 0;
		return 11;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/accept") == 0) {
		return 7;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/refuse") == 0) {
		return 8;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/yes") == 0) {
		return 12;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/no") == 0) {
		return 13;
	}
	if (SocketInfo->command != 0) {
		return SocketInfo->command;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/register") == 0) {
		return 1;
	}
	if (strcmp(SocketInfo->DataBuf.buf, "/sign in") == 0) {
		return 2;
	}
	if (SocketInfo->DataBuf.buf[0] == '/') {
		return 4;
	}
	return 5;
}

void QuitGroup(LPSOCKET_INFORMATION SI) {
	if (SI->grouphead->next != NULL) {
		//���û�����һ��Ⱥ�ڵ�ʱ����Ҫ���û���Ⱥ���˳�
		char tmpstr[DATA_BUFSIZE];
		char tmpstr2[DATA_BUFSIZE];
		GroupList tmphere = NULL;
		GroupList tmphere2 = NULL;
		strcpy_s(tmpstr, DATA_BUFSIZE, SI->name);
		strcat_s(tmpstr, DATA_BUFSIZE, "(");
		_itoa_s(SI->account, tmpstr2, DATA_BUFSIZE, 10);
		strcat_s(tmpstr, DATA_BUFSIZE, tmpstr2);
		strcat_s(tmpstr, DATA_BUFSIZE, ")---");
		strcat_s(tmpstr, DATA_BUFSIZE, "�˳�Ⱥ�ģ�\n");
		strcpy_s(SI->DataBuf.buf, DATA_BUFSIZE, tmpstr);
		tmphere = SI->grouphead->next;
		while (tmphere) {
			//ΪȺ���ڵ�ÿһλ��Ա������Ϣ
			strcpy_s(tmphere->pmember->DataBuf.buf, DATA_BUFSIZE, tmpstr);
			tmphere->pmember->BytesRECV = strlen(tmphere->pmember->DataBuf.buf);
			tmphere = tmphere->next;
		}
		GroupList tmpmember = SI->grouphead;
		if (SI->grouphead->next->next->next == NULL) {
			//Ⱥ��ֻ��������
			tmphere = SI->grouphead->next->next;
			tmphere2 = SI->grouphead->next;
			tmphere->next = NULL;
			tmphere2->next = NULL;
			//������Ҫ˼��
			//talk.c �������޸�
			tmphere->pmember->grouphead = tmphere;
			tmphere2->pmember->grouphead = tmphere2;
		}
		else {
			//Ⱥ���ж����
			while (tmpmember) {
				if (tmpmember->next->pmember == SI) {
					//���û��˳�
					tmphere = tmpmember->next;
					tmpmember->next = tmpmember->next->next;
					break;
				}
				tmpmember = tmpmember->next;
			}
			//���û����ڿ�Ⱥ��
			SI->grouphead = tmphere;
			tmphere->next = NULL;
		}
	}
}