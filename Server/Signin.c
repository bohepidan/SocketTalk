#include "stdbohe.h"

int Register(char name[], char password[], long account) {
	FILE* pMembers;
	int ferror;
	ferror = fopen_s(&pMembers, "Members.txt", "a+");
	if (ferror != 0) {
		printf("���ļ�ʧ�ܣ�������룺%d", ferror);
		return -1;
	}
	fprintf(pMembers, "%ld %s %s\n", account, password, name);
	fclose(pMembers);
	return 0;
}

char* FindAccount(LPSOCKET_INFORMATION SocketInfo, long goal) {
	FILE* pMembers;
	int ferror;
	long Account;
	char Name[NAME_SIZE];
	char password[PASSWORD_SIZE];
	ferror = fopen_s(&pMembers, "Members.txt", "r");
	printf("goal == %ld\n", goal);
	if (ferror == 0) {
		while (!feof(pMembers)) {
			fscanf_s(pMembers, "%ld %s %s\n", &Account, password, PASSWORD_SIZE, Name, NAME_SIZE);
			printf("** account == %ld **\n", Account);
			if (Account == goal) {
				SocketInfo->account = Account;
				strcpy_s(SocketInfo->name, NAME_SIZE, Name);
				fclose(pMembers);
				return password;
			}
		}
	}
	else {
		printf("Members.txt�ļ���ʧ�ܣ�\n");
	}
	fclose(pMembers);
	strcpy_s(password, PASSWORD_SIZE, "notfound");
	return password;
}