//FULLIME.H
#define MAXROW          12 
#define MAXCOL          60 
#define FIRSTROW         0 
#define FIRSTCOL         0 
#define LASTROW         (MAXROW-1) 
#define LASTCOL         (MAXCOL-1) 
#define TABSTOP          4 
#define MAX_LISTCAND     32 
//#define MAX_CHARS_PER_LINE   10 
//#define MAX_COMP_STRING_LEN  10 
#define DEFAULT_CAND_NUM_PER_PAGE 5 
#define X_INDENT         10 
#define Y_INDENT         10 
 
//#define IDC_LIST1                0 
//#define IDC_LIST32              31 

#define IME_IN_COMPOSITION      1 
#define IME_IN_CHOSECAND        2 
 
#define CheckProperty {\
if ( ( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) || \
	!( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) ) \
goto call_defwinproc; \
}
 
typedef struct _IMEUIDATA
{ 
    int        ImeState;   // Current Ime state. 
    UINT       uCompLen;   // To save previous composition string length. 
    DWORD      fdwProperty; 
    HWND       hListCand[ MAX_LISTCAND ]; 
    HGLOBAL    hListCandMem[ MAX_LISTCAND ]; 
} IMEUIDATA; 
// Prototype declaration 
long WINAPI SteWndProc( HWND, UINT, UINT, LONG ); 
long WINAPI CandWndProc( HWND, UINT, UINT, LONG ); 
 
void ResetCaret( HWND ); 
 
void ImeUIStartComposition( HWND ); 
void ImeUIComposition( HWND, WPARAM, LPARAM ); 
void GetCompositionStr( HWND, LPARAM ); 
void GetResultStr( HWND ); 
void ImeUIEndComposition( HWND ); 
void ImeUIOpenCandidate( HWND, LPARAM ); 
void ImeUICloseCandidate( HWND, LPARAM ); 
void ImeUISetOpenStatus( HWND ); 
void DisplayResultString( HWND, LPSTR ); 
void DisplayCompString( HWND, LPSTR, LPSTR ); 
void RestoreImeUI( HWND ); 
BOOL ImeUINotify( HWND, WPARAM, LPARAM ); 
void ImeUIChangeCandidate( HWND, LPARAM ); 
void DisplayCandStrings( HWND, LPCANDIDATELIST ); 
void CandUIPaint( HWND ); 
void ImeUIMoveCandWin( HWND ); 
BOOL MoveCaret( HWND ); 
void ImeUIClearData( HWND ); 
void SetIMECompFormPos( HWND ); 
// Global data 
char szSteClass[80]; 
char szSteCandUIClass[80]; 
char szSteTitle[80]; 
char szSteCompTitle[80]; 
char szSteCandTitle[80]; 
char szCandClass[80]; 
 
UINT   cxMetrics, 
       cxOverTypeCaret, 
       cyMetrics; 
 
int    xPos, yPos; 
HFONT  hfntFixed; 
HFONT  hfntOld; 
BOOL   fInsertMode; 
int    CaretWidth; 
int    DBCSFillChar; 
BYTE   textbuf[MAXROW][MAXCOL]; 
 
IMEUIDATA gImeUIData; 

HKL    hCurKL; 
