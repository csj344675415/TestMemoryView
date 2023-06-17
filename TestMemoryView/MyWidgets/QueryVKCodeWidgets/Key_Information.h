#pragma once

#include <tchar.h> 
#include <vector> 
using namespace std;

typedef struct _KEY_INFORMATION
{
	unsigned long Key;
	char *Keyname;
	char *VK_Key;
	unsigned long EventKey;
	char *EventKeyname;

}KEY_INFORMATION, *PKEY_INFORMATION;

int nKey[121] = {
	//F1-F12
	112,113,114,115,116,117,118,119,120,121,122,123,	
	//大键盘：0-9
	48,49,50,51,52,53,54,55,56,57,
	//大键盘：A-Z
	65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
	//小键盘：0-9
	96,97,98,99,100,101,102,103,104,105,
	//小键盘：+，-，*，/
	107,109,106,111,
	//方向键：左上右下
	37,38,39,40,
	//大键盘符号键：` - = [ ] ; ' \ , . /
	192,189,187,219,221,186,222,220,188,190,191,
	//功能键：Esc,回车,退格,空格,Tab,Caps,Shift,Ctrl,Win,Alt,Ins,Del,Home,End,Pup,Pdn,Num=144,Prt=44,Scrall=145,Pause=19
	27,13,8,32,9,20,16,17,91,18,45,46,36,35,33,34,144,44,145,19,
	//LShift,LCtrl,LAlt,Ralt,RCtrl,RShift,打印键，小键盘Del(.,RWin
	160,162,164,165,163,161,93,110,92,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

char *sKeyname[121] = {
	//VS
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
	
	//VS+QT
	"0","1","2","3","4","5","6","7","8","9",
	//VS+QT
	"A","B","C","D","E","F","G","H","I","J","K","L","M",
	"N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
	//VS
	"Num 0","Num 1","Num 2","Num 3","Num 4","Num 5","Num 6","Num 7","Num 8","Num 9",
	
	//VS
	"Num +","Num -","Num *","Num /",
	
	//VS
	"Up","Down","Left","Right",
	
	//VS
	"`~","-_","=+","[{","]}",";:","'\"","\\|",",<",".>","/?",
	
	//VS
	"Esc","Enter","Backspace","Space","Tab","Caps Lock","Shift","Ctrl","Left Windows","Alt",
	"Insert","Delete","Home","End","Page Up","Page Down","Num Lock","Prt Sc Sys Rq","Scroll Lock","Pause",
	
	"Left Shift","Left Ctrl","Left Alt","Right Alt","Right Ctrl","Right Shift",u8"打印键","Num Del","Right Windows",	

	"","","","","","","","","","","","","","",""
};

char *sVK_Key[121] = {
	//VS
	"VK_F1","VK_F2","VK_F3","VK_F4","VK_F5","VK_F6","VK_F7","VK_F8","VK_F9","VK_F10","VK_F11","VK_F12",

	//VS+QT
	"VK_0","VK_1","VK_2","VK_3","VK_4","VK_5","VK_6","VK_7","VK_8","VK_9",
	//VS+QT
	"VK_A","VK_B","VK_C","VK_D","VK_E","VK_F","VK_G","VK_H","VK_I","VK_J","VK_K","VK_L","VK_M",
	"VK_N","VK_O","VK_P","VK_Q","VK_R","VK_S","VK_T","VK_U","VK_V","VK_W","VK_X","VK_Y","VK_Z",
	//VS
	"VK_NUMPAD0","VK_NUMPAD1","VK_NUMPAD2","VK_NUMPAD3","VK_NUMPAD4","VK_NUMPAD5","VK_NUMPAD6","VK_NUMPAD7","VK_NUMPAD8","VK_NUMPAD9",
	//VS
	"VK_ADD","VK_SUBTRACT","VK_MULTIPLY","VK_DIVIDE",
	//VS
	"VK_LEFT","VK_UP","VK_RIGHT","VK_DOWN",
	//VS
	"VK_OEM_3","VK_OEM_MINUS","VK_OEM_PLUS","VK_OEM_4","VK_OEM_6","VK_OEM_1","VK_OEM_7","VK_OEM_5","VK_OEM_COMMA","VK_OEM_PERIOD","VK_OEM_2",
	//VS
	"VK_ESCAPE","VK_RETURN","VK_BACK","VK_SPACE","VK_TAB","VK_CAPITAL","VK_SHIFT","VK_CONTROL","VK_LWIN","VK_MENU",
	"VK_INSERT","VK_DELETE","VK_HOME","VK_END","VK_PRIOR","VK_NEXT","VK_NUMLOCK","VK_SNAPSHOT","VK_SCROLL","VK_PAUSE",

	"VK_LSHIFT","VK_LCONTROL","VK_LMENU","VK_RMENU","VK_RCONTROL","VK_RSHIFT","VK_APPS","VK_DECIMAL","VK_RWIN",

	"","","","","","","","","","","","","","",""
};

int nEventKey[121] = {
	//QT_F1-F12
	16777264,16777265,16777266,16777267,16777268,16777269,16777270,16777271,16777272,16777273,16777274,16777275,
	//大键盘：0-9
	48,49,50,51,52,53,54,55,56,57,
	//大键盘：A-Z
	65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
	//QT_小键盘：0-9
	48,49,50,51,52,53,54,55,56,57,
	//QT_小键盘：+，-，*，/
	43,45,42,47,
	//QT_方向键：左上右下
	16777234,16777235,16777236,16777237,
	//QT大键盘符号键：` - = [ ] ; ' \ , . /
	96,45,61,91,93,59,39,92,44,46,47,
	//QT_功能键：Esc,回车,退格,空格,Tab,Caps,Shift,Ctrl,Win,Alt,Ins,Del,Home,End,Pup,Pdn,Num=144,Prt=44,Scrall=145,Pause=19
	16777216,16777220,16777219,32,16777217,16777252,16777248,16777249,16777250,16777251,16777222,
	16777223,16777232,16777233,16777238,16777239,16777253,-1,16777254,16777224,16777221,
	//大键盘符号键 ~ ! @ # $ % ^ & * ( ) _ + { } : " | < > ? 打印键,空键
	126,33,64,35,36,37,94,38,42,40,41,95,43,123,125,58,34,124,60,62,63,16777301,6777227
};

char *sEventKeyname[121] = {
	//QT
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
	//VS+QT
	"0","1","2","3","4","5","6","7","8","9",
	//VS+QT
	"A","B","C","D","E","F","G","H","I","J","K","L","M",
	"N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
	//QT
	"Num 0","Num 1","Num 2","Num 3","Num 4","Num 5","Num 6","Num 7","Num 8","Num 9",
	//QT
	"Num +","Num -","Num *","Num /",
	//QT
	"Up","Down","Left","Right",
	//QT
	"`","-","=","[","]",";","'","\\",",",".","/",
	//QT
	"Esc","Enter","Backspace","Space","Tab","Caps Lock","Left Shift","Left Ctrl","Left Windows","Left Alt",
	"Insert","Delete","Home","End","Page Up","Page Down","Num Lock","Prt Sc Sys Rq","Scroll Lock","Pause","Num Enter",

	"~","!","@","#","$","%","^","&","*","(",")","_","+","{","}",":","\"","|","<",">","?",u8"打印键","Num 5_"

};

vector<_KEY_INFORMATION> Keyvector;

void pushKeyvector()
{
	KEY_INFORMATION KeyInformation;
	//memset(&pKeyInformation, 0, sizeof(PKEY_INFORMATION));
	for (int i = 0; i < 121; i++)
	{
		KeyInformation.Key = nKey[i];
		KeyInformation.Keyname = sKeyname[i];
		KeyInformation.VK_Key = sVK_Key[i];

		KeyInformation.EventKey = nEventKey[i];
		KeyInformation.EventKeyname = sEventKeyname[i];
		Keyvector.push_back(KeyInformation);
	}
}

bool GetKeyInformation(KEY_INFORMATION &Keyinfo, unsigned int nKey)
{
	int vSize = Keyvector.size();
	for (int i = 0; i < vSize; i++)
	{
		if (nKey == Keyvector[i].Key)
		{
			Keyinfo.Key = Keyvector[i].Key;
			Keyinfo.Keyname = Keyvector[i].Keyname;
			Keyinfo.VK_Key = Keyvector[i].VK_Key;
			return 1; //ture=1,false=0
		}

	}
	return 0;
}

bool GetEventKeyInformation(KEY_INFORMATION &Keyinfo, unsigned long nEventKey)
{
	int vSize = Keyvector.size();
	for (int i = 0; i < vSize; i++)
	{
		if (nEventKey == Keyvector[i].EventKey)
		{
			Keyinfo.EventKey = Keyvector[i].EventKey;
			Keyinfo.EventKeyname = Keyvector[i].EventKeyname;

			//Keyinfo.Key = Keyvector[i].Key;
			//Keyinfo.Keyname = Keyvector[i].Keyname;
			//Keyinfo.VK_Key = Keyvector[i].VK_Key;
			return 1; //ture=1,false=0
		}

	}
	return 0;
}

bool GetEventKeyToVKKeyInformation(KEY_INFORMATION &Keyinfo, unsigned long nEventKey)
{
	int vSize = Keyvector.size();
	for (int i = 0; i < vSize; i++)
	{
		if (nEventKey == Keyvector[i].EventKey)
		{
			//Keyinfo.EventKey = Keyvector[i].EventKey;
			//Keyinfo.EventKeyname = Keyvector[i].EventKeyname;

			Keyinfo.Key = Keyvector[i].Key;
			Keyinfo.Keyname = Keyvector[i].Keyname;
			Keyinfo.VK_Key = Keyvector[i].VK_Key;
			return 1; //ture=1,false=0
		}

	}
	return 0;
}

int getKey(char * s)
{
	int vSize = Keyvector.size();
	for (int i = 0; i < vSize; i++)
	{
		if (strcmp(s, Keyvector[i].Keyname) == 0)
		{
			return Keyvector[i].Key;
		}
	}
	return -1;
}