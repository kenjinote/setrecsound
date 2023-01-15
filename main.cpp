#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"winmm.lib")
#include <windows.h>

CHAR szClassName[]="window";
HWND hList;
UINT m_nNumMixers;
HMIXER m_hMixer;
MIXERCAPS m_mxcaps;
char m_strDstLineName[64];
char m_strSelectControlName[64];
DWORD m_dwControlType,m_dwSelectControlID,m_dwMultipleItems;

BOOL amdInitialize(HWND hWnd)
{
	m_nNumMixers=mixerGetNumDevs();
	m_hMixer=NULL;
	ZeroMemory(&m_mxcaps,sizeof(MIXERCAPS));
	strcat(m_strDstLineName,"");
	strcat(m_strSelectControlName,"");
	m_dwControlType=0;
	m_dwSelectControlID=0;
	m_dwMultipleItems=0;
	if(m_nNumMixers!=0)
	{
		if(mixerOpen(&m_hMixer,0,reinterpret_cast<DWORD>(hWnd),NULL,MIXER_OBJECTF_MIXER|CALLBACK_WINDOW)!=MMSYSERR_NOERROR)return FALSE;
		if(mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),&m_mxcaps,sizeof(MIXERCAPS))!=MMSYSERR_NOERROR)return FALSE;
	}
	return TRUE;
}

BOOL amdUninitialize()
{
	BOOL bSucc=TRUE;
	if(m_hMixer!=NULL)
	{
		bSucc=(mixerClose(m_hMixer)==MMSYSERR_NOERROR);
		m_hMixer=NULL;
	}
	return bSucc;
}

BOOL amdGetMicSelectControl()
{
	if(m_hMixer==NULL)return FALSE;
	MIXERLINE mxl;
	mxl.cbStruct=sizeof(MIXERLINE);
	mxl.dwComponentType=MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	if(mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),&mxl,MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_COMPONENTTYPE)!=MMSYSERR_NOERROR)return FALSE;
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	m_dwControlType=MIXERCONTROL_CONTROLTYPE_MIXER;
	mxlc.cbStruct=sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID=mxl.dwLineID;
	mxlc.dwControlType=m_dwControlType;
	mxlc.cControls=1;
	mxlc.cbmxctrl=sizeof(MIXERCONTROL);
	mxlc.pamxctrl=&mxc;
	if(mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),
		&mxlc,
		MIXER_OBJECTF_HMIXER|
		MIXER_GETLINECONTROLSF_ONEBYTYPE)
		!=MMSYSERR_NOERROR)
	{
		m_dwControlType=MIXERCONTROL_CONTROLTYPE_MUX;
		mxlc.cbStruct=sizeof(MIXERLINECONTROLS);
		mxlc.dwLineID=mxl.dwLineID;
		mxlc.dwControlType=m_dwControlType;
		mxlc.cControls=1;
		mxlc.cbmxctrl=sizeof(MIXERCONTROL);
		mxlc.pamxctrl=&mxc;
		if(mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),&mxlc,MIXER_OBJECTF_HMIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE)!=MMSYSERR_NOERROR)return FALSE;
	}
	strcpy(m_strDstLineName,mxl.szName);
	strcpy(m_strSelectControlName,mxc.szName);
	m_dwSelectControlID=mxc.dwControlID;
	m_dwMultipleItems=mxc.cMultipleItems;
	if(m_dwMultipleItems==0)return FALSE;
	MIXERCONTROLDETAILS_LISTTEXT *pmxcdSelectText=new MIXERCONTROLDETAILS_LISTTEXT[m_dwMultipleItems];
	if(pmxcdSelectText!=NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID=m_dwSelectControlID;
		mxcd.cChannels=1;
		mxcd.cMultipleItems=m_dwMultipleItems;
		mxcd.cbDetails=sizeof(MIXERCONTROLDETAILS_LISTTEXT);
		mxcd.paDetails=pmxcdSelectText;
		if(mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),&mxcd,MIXER_OBJECTF_HMIXER|MIXER_GETCONTROLDETAILSF_LISTTEXT)==MMSYSERR_NOERROR)
		{
			for(DWORD dwi=0;dwi<m_dwMultipleItems;dwi++)SendMessage(hList,LB_ADDSTRING,0,(LPARAM)pmxcdSelectText[dwi].szName);
		}
		delete []pmxcdSelectText;
	}
	return TRUE;
}

BOOL amdGetMicSelectValue(LONG &lVal)
{
	if(m_hMixer==NULL||m_dwMultipleItems==0)return FALSE;
	BOOL bRetVal=FALSE;
	MIXERCONTROLDETAILS_BOOLEAN *pmxcdSelectValue=new MIXERCONTROLDETAILS_BOOLEAN[m_dwMultipleItems];
	if(pmxcdSelectValue !=NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID=m_dwSelectControlID;
		mxcd.cChannels=1;
		mxcd.cMultipleItems=m_dwMultipleItems;
		mxcd.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails=pmxcdSelectValue;
		if(mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
			&mxcd,
			MIXER_OBJECTF_HMIXER|
			MIXER_GETCONTROLDETAILSF_VALUE)
			==MMSYSERR_NOERROR)
		{
			for(DWORD dwi=0;dwi<m_dwMultipleItems;dwi++)if(pmxcdSelectValue[dwi].fValue)break;
			lVal=dwi;
			bRetVal=TRUE;
		}
		delete []pmxcdSelectValue;
	}
	return bRetVal;
}

BOOL amdSetMicSelectValue(LONG lVal)
{
	if(m_hMixer==NULL||m_dwMultipleItems==0)return FALSE;
	BOOL bRetVal=FALSE;
	MIXERCONTROLDETAILS_BOOLEAN *pmxcdSelectValue=new MIXERCONTROLDETAILS_BOOLEAN[m_dwMultipleItems];
	if(pmxcdSelectValue !=NULL)
	{
		MIXERCONTROLDETAILS mxcd;
		mxcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
		mxcd.dwControlID=m_dwSelectControlID;
		mxcd.cChannels=1;
		mxcd.cMultipleItems=m_dwMultipleItems;
		mxcd.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
		mxcd.paDetails=pmxcdSelectValue;
		if(mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),&mxcd,MIXER_OBJECTF_HMIXER|MIXER_GETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR)
		{
			if(m_dwControlType==MIXERCONTROL_CONTROLTYPE_MUX)ZeroMemory(pmxcdSelectValue,m_dwMultipleItems*sizeof(MIXERCONTROLDETAILS_BOOLEAN));
			pmxcdSelectValue[lVal].fValue=1;
			mxcd.cbStruct=sizeof(MIXERCONTROLDETAILS);
			mxcd.dwControlID=m_dwSelectControlID;
			mxcd.cChannels=1;
			mxcd.cMultipleItems=m_dwMultipleItems;
			mxcd.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			mxcd.paDetails=pmxcdSelectValue;
			if(mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),&mxcd,MIXER_OBJECTF_HMIXER|MIXER_SETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR)bRetVal=TRUE;
		}
		delete []pmxcdSelectValue;
	}
	return bRetVal;
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	LONG lVal;
	switch(msg){
	case MM_MIXM_CONTROL_CHANGE:
		if(reinterpret_cast<HMIXER>(wParam)==m_hMixer&&static_cast<DWORD>(lParam)==m_dwSelectControlID)
			if(amdGetMicSelectValue(lVal))SendMessage(hList,LB_SETCURSEL,lVal,0);
		break;
	case WM_COMMAND:
		if(HIWORD(wParam)==LBN_SELCHANGE)
		{
			lVal=SendMessage(hList,LB_GETCURSEL,0,0);
			amdSetMicSelectValue(lVal);
		}
		break;
	case WM_CREATE:
			hList=CreateWindow("LISTBOX","",WS_CHILD|WS_VISIBLE|WS_BORDER|LBS_NOTIFY,10,10,200,300,hWnd,(HMENU)NULL,((LPCREATESTRUCT)lParam)->hInstance,NULL);
			amdInitialize(hWnd);
			amdGetMicSelectControl();
			if(amdGetMicSelectValue(lVal))SendMessage(hList,LB_SETCURSEL,lVal,0);
			break;
		case WM_DESTROY:
			amdUninitialize();
			PostQuitMessage(0);
			break;
		default:
			return(DefWindowProc(hWnd,msg,wParam,lParam));
	}
	return(0L);
}

int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hPreInst,
				   LPSTR pCmdLine,int nCmdShow)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wndclass;
	if(!hPreInst){
		wndclass.style=CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc=WndProc;
		wndclass.cbClsExtra=0;
		wndclass.cbWndExtra=0;
		wndclass.hInstance=hinst;
		wndclass.hIcon=NULL;
		wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
		wndclass.lpszMenuName=NULL;
		wndclass.lpszClassName=szClassName;
		if(!RegisterClass(&wndclass))
			return FALSE;
	}
	hWnd=CreateWindow(szClassName,
		"録音コントロールを選択する",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hinst,
		NULL);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}
