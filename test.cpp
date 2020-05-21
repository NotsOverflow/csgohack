#define UNICODE
#pragma comment(lib, "user32")
#include <vector>
#include <iomanip>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

// ca c'est une class qui te permet de trouver un process, de trouver le décalage d'adresse d'une dll
// genre avec ca on cherche csgo.exe et aprés l'adress de client_panorama.dll pour trouver toutes les variables
// et ca permet aussi de lire et écrire a une adresse mais ca tu connais
// ici on déclare les fonction et défini ceux qui font qu'une ligne ou deux
class BlueMethMem
{
public:
	 BlueMethMem();
	~BlueMethMem();

	template <class val>
	val readMem(uintptr_t addr)
	{
		val x;
		ReadProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
		return x;
	}

	template <class val>
	val WMemory(uintptr_t addr, val x)
	{
		WriteProcessMemory(handle, (LPBYTE*)addr, &x, sizeof(x), NULL);
		return x;
	}

	uintptr_t getProcess(const wchar_t*);
	uintptr_t getModule(uintptr_t, const wchar_t*);
	uintptr_t getAddress(uintptr_t, std::vector<uintptr_t>);

private:
	HANDLE handle;
};

// ces deux truc c'est que faire quand on crée la class et quand on la détruit

BlueMethMem::BlueMethMem() { handle = NULL; }

BlueMethMem::~BlueMethMem()
{
	CloseHandle(handle);
}

// ici ont défini les fonctions qui fesai plus de deux trois lignes

uintptr_t BlueMethMem::getProcess(const wchar_t* proc)
{
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	uintptr_t process;
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	do
	{
		if (!_wcsicmp(pEntry.szExeFile, proc))
		{
			process = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
			handle = OpenProcess(PROCESS_ALL_ACCESS, false, process);
		}
	} while (Process32Next(hProcessId, &pEntry));
	return process;
}

uintptr_t BlueMethMem::getModule(uintptr_t procId, const wchar_t* modName)
{
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);
	do
	{
		if (!_wcsicmp(mEntry.szModule, modName))
		{
			CloseHandle(hModule);
			return (uintptr_t)mEntry.hModule;
		}
	} while (Module32Next(hModule, &mEntry));
	return 0;
}

uintptr_t BlueMethMem::getAddress(uintptr_t addr, std::vector<uintptr_t> vect)
{
	for (int i = 0; i < vect.size(); i++)
	{
		ReadProcessMemory(handle, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += vect[i];
	}
	return addr;
}

BlueMethMem MethClass;


// ici c'est les décallages par rapport a l'adress de "client_panorama.dll"

struct offset
{
	DWORD m_iTeamNum = 0xF4;
	DWORD m_iGlowIndex = 0xA428;
	DWORD dwEntityList = 0x4D43AB4;
	DWORD dwLocalPlayer = 0xD2FB84;
	DWORD dwGlowOBJManager = 0x528B880;
}offset;

struct variables
{
	DWORD localPlayer;
	DWORD gameModule;
}val;

// ca tu connais

int main()
{

	// la ca s'éxplique tout seul
	int ProcessID = MethClass.getProcess(L"csgo.exe");
	val.gameModule = MethClass.getModule(ProcessID, L"client_panorama.dll");
	val.localPlayer = MethClass.readMem<DWORD>(val.gameModule + offset.dwLocalPlayer);

	// tand que le joueur existe pas, on attend
	if (val.localPlayer == NULL)
		while (val.localPlayer == NULL)
			val.localPlayer = MethClass.readMem<DWORD>(val.gameModule + offset.dwLocalPlayer);


	// le programme
	while (true)
	{
		// trouve chaque joueur et les parcour un a un pour les colorer, le réste s'éxplique tout seul
		DWORD GlowOBJ = MethClass.readMem<DWORD>(val.gameModule + offset.dwGlowOBJManager);
		int myTeam = MethClass.readMem<int>(val.localPlayer + offset.m_iTeamNum);
		for (short int i = 0; i < 60; i++)
		{

			DWORD entity = MethClass.readMem<DWORD>(val.gameModule + offset.dwEntityList + i * 0x10);

			if (entity != NULL)
			{
				int GlowINDX = MethClass.readMem<int>(entity + offset.m_iGlowIndex);
				int entityTeam = MethClass.readMem<int>(entity + offset.m_iTeamNum);

				if (myTeam == entityTeam)
				{
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x4), 0);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x8), 0);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0xC), 2);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x10),1);
				}
				else
				{
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x4), 2);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x8), 0);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0xC), 0);
					MethClass.WMemory<float>(GlowOBJ + ((GlowINDX * 0x38) + 0x10),1);
				}

				MethClass.WMemory<bool>(GlowOBJ + ((GlowINDX * 0x38) + 0x24), true);
				MethClass.WMemory<bool>(GlowOBJ + ((GlowINDX * 0x38) + 0x25), false);
			}

		}
		
		Sleep(1);
	}
	return 0;
}
