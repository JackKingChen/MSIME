//MAIN.C
/************************************************************************ 
* 
* Title: 
* 
*   STED.C - IME Full-aware Simple Text Editor (DBCS version) 
* 
* Purpose: 
* 
*   Sample program for DBCS programming and IME full-aware appliction. 
* 
* Synopsis: 
* 
*   This program is designed as a bare-bone example to demonstrate the 
*   basic elements of DBCS-enabling, and how to design an IME full-aware 
*   application. 
* 
*   The data structure is a fixed-size 2-dimensional byte array.  The 
*   font is the fixed-pitch system font.  Rudimentary text editing 
*   functions, such as the basic cursor movements, insertion, deletion, 
*   have been implemented. 
* 
*   When you run this program in a Far East Windows environment, it 
*   should be apparent that it doesn't handle DBCS character very well. 
*   It is your job to locate and modify the pieces inside this program 
*   that need to be DBCS-enabled. 
* 
* DBCS-enabling notes: 
* 
*   This version of STE is DBCS-enabled with respect to character input, 
*   caret shape and movement, and mouse clicking.  It should work on 
*   any version of Far East Windows since 3.0. 
* 
*   As far as source code maintenance goes, there are generally two 
*   approaches. 
* 
*   The first is to add DBCS enabling code under '#ifdef DBCS'.  The 
*   advantage of this approach is that it keeps the DBCS enabling code 
*   distinct from the SBCS, so it's easier to add them in, and also to 
*   remove them later.  (For example, when you want to replace DBCS 
*   enabling with Unicode enabling.)  The drawback is that because the 
*   DBCS and the SBCS logic are not integrated, they can easily get out 
*   of sync (as the SBCS code evolves.) 
* 
*   The second approach, which is adopted by this sample app, is to 
*   integrate DBCS enabling with SBCS.  It takes longer to do, but 
*   the resulting source is easier to maintain.  Since IsDBCSLeadByte 
*   is at the heart of any DBCS-enabling logic, an additional speed up 
*   for generating an SBCS-only version is to define that function as 
*   FALSE, and let the compiler optimize the DBCS logic away. 
* 
* IME Full-Aware notes: 
* 
*   This version of STE is an IME full-aware application with most of 
*   IME UI capiabilities to display IME UI by itself. It should work 
*   on any version of Far Windows since 4.0. 
* 
*   This kind of application typically wants to be fully responsible to 
*   display any information given by IME. Ii will handle the input context 
*   by itself and display any necessary information given by the 
*   input context not using IME UI. 
* 
* History: 
* 
*   17-Aug-1992  created 
*   28-Sep-1992  added DBCS-enabling 
*   30-Sep-1992  bug fixes 
*   25-Mar-1994  added IME full-aware logics 
*   05-Sep-2012  Midified and Compliled by vc60
************************************************************************/ 
#include <assert.h> 
#include <windows.h> 
#include <imm.h> 
#include "resource.h" 
#include "fullime.h" 
// Function prototype 
/************************************************************************ 
* 
*   SteRegister - standard class registration routine 
* 
************************************************************************/ 
int SteRegister( HANDLE hInstance ) 
{ 
    long WINAPI SteWndProc( HWND, UINT, UINT, LONG ); 
 
    WNDCLASS WndClass; 
 
    memset(&WndClass,0,sizeof(WNDCLASS));
 
    WndClass.hCursor       = LoadCursor( NULL, IDC_IBEAM ); 
    WndClass.hIcon         = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ICON)); 
    WndClass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU); 
    WndClass.hInstance     = hInstance; 
    WndClass.lpszClassName = (LPSTR)szSteClass; 
    WndClass.lpfnWndProc   = SteWndProc; 
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
    WndClass.style         = CS_BYTEALIGNCLIENT | CS_CLASSDC; 
 
    if ( !RegisterClass(&WndClass ) ) return FALSE; 
 
    WndClass.hCursor       = LoadCursor( NULL,IDC_ARROW ); 
    WndClass.hIcon         = NULL; 
    WndClass.lpszMenuName  = NULL; 
    WndClass.hInstance     = hInstance; 
    WndClass.lpszClassName = (LPSTR)szCandClass; 
    WndClass.lpfnWndProc   = CandWndProc; 
    WndClass.hbrBackground = GetStockObject( LTGRAY_BRUSH ); 
    WndClass.style         = CS_HREDRAW | CS_VREDRAW; 
 
    if ( !RegisterClass(&WndClass ) ) return FALSE; 
// 
    return TRUE; 
} 
/************************************************************************ 
* 
*   WinMain 
* 
************************************************************************/ 
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int nCmdShow) 
{ 
    MSG msg; 
    HWND hWnd; 
	int x,y,cx,cy;

    LoadString( hInstance, IDS_CLASS, szSteClass, 14 ); 
    LoadString( hInstance, IDS_TITLE, szSteTitle, 55 ); 
    LoadString( hInstance, IDS_CANDUI, szSteCandUIClass, 12 ); 
    LoadString( hInstance, IDS_COMPTITLE, szSteCompTitle, 55 ); 
    LoadString( hInstance, IDS_CANDTITLE, szSteCandTitle, 55 ); 
    LoadString( hInstance, IDS_CANDCLASS, szCandClass, 20 ); 
 
    if ( !SteRegister( hInstance ) ) return FALSE; 
 
    if ( !(hWnd = CreateWindow( szSteClass, szSteTitle, 
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
				CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, (HWND)NULL, 
				(HMENU)NULL, hInstance, NULL)) )
	{
		return FALSE;
	}
// Create window with just enough client space for the text buffer 
	cx  = MAXCOL*cxMetrics + GetSystemMetrics(SM_CXBORDER)*2;
	x   = (GetSystemMetrics(SM_CXFULLSCREEN)-cx)/2;
	cy  = MAXROW*cyMetrics;
	cy += GetSystemMetrics(SM_CYBORDER)*2;
	cy += GetSystemMetrics(SM_CYCAPTION);
	cy += GetSystemMetrics(SM_CYMENU);
	y   = (GetSystemMetrics(SM_CYFULLSCREEN)-cy)/2;
	SetWindowPos(hWnd, 0, x, y, cx, cy, SWP_NOZORDER); 
    ShowWindow( hWnd, nCmdShow ); 
// IME initial state. 
    gImeUIData.ImeState = 0; 
 
    while ( GetMessage( &msg, NULL, 0, 0 ) )
	{ 
		TranslateMessage( &msg ); 
		DispatchMessage( &msg ); 
    } 
 
    return msg.wParam; 
} 
//********************************************************************** 
// 
// SteIMEOpenClose() 
// 
// This routines calls IMM API to open or close IME. 
// 
//********************************************************************** 
void SteImeOpenClose( HWND hWnd, BOOL fFlag ) 
{ 
	HIMC hIMC;
// If fFlag is true then open IME; otherwise close it. 
    if ( !( hIMC = ImmGetContext( hWnd ) ) ) return; 
    ImmSetOpenStatus( hIMC, fFlag );// return 1 
    ImmReleaseContext( hWnd, hIMC ); 
} 
//********************************************************************* 
// 
// WM_CREATE message handler 
// 
//********************************************************************* 
void SteCreate( HWND hWnd ) 
{ 
    HDC hdc; 
    TEXTMETRIC tm; 
    int        i; 
    WORD       patern = 0xA4A4; 
    SIZE       size; 
    HFONT      hFont; 
    LOGFONT    lFont; 
    // Note that this window has a class DC 
    hdc = GetDC( hWnd ); 
    // Select fixed pitch system font and get its text metrics 
    hfntFixed = GetStockObject( SYSTEM_FIXED_FONT ); 
    hfntOld = SelectObject( hdc, hfntFixed ); 
    GetTextMetrics( hdc, &tm ); 
    ReleaseDC( hWnd, hdc ); 
 
    GetTextExtentPoint( hdc, (LPSTR)&patern, sizeof( WORD), (LPSIZE) &size ); 
//  cxMetrics = tm.tmAveCharWidth; 
//  cyMetrics = tm.tmHeight; 
    cxMetrics = (UINT) size.cx / 2; 
    cyMetrics = (UINT) size.cy; 
// Determine the version of DBCS Windows from system font charset ID. 
// Then hardcode a DBCS character value for the 'Pattern/DBCS' command. 
// The value is the Han character for 'door' or 'gate', which is 
// left-right symmetrical. 
    switch( tm.tmCharSet ) 
    { 
    case SHIFTJIS_CHARSET:// japan 
		DBCSFillChar = 0x96e5; 
	break; 
    case HANGEUL_CHARSET: // Korea
		DBCSFillChar = 0xdaa6; 
	break; 
    case CHINESEBIG5_CHARSET: // big5
		DBCSFillChar = 0xaaf9; 
	break; 
    default: 
		DBCSFillChar = 0x7071;  // 'pq'= �� 
	break; 
    } // end switch( tm.tmCharSet )
// Initialize caret width.  Fat in INSERT mode, skinny in OVERTYPE mode. 
    fInsertMode = FALSE; 
    CaretWidth = cxOverTypeCaret = GetSystemMetrics( SM_CXBORDER ); 
    // Sets the logical font to be used to display characters in the  
    // composition window. Especially for at caret or near caret operation,  
    // application should set composition font. 
    // 
    // If Application provides user to dynamicly change font, each time after 
    // user change font, application should set composition font again. 
    if ( ( hFont = (HFONT)SendMessage( hWnd, WM_GETFONT, 0, 0L ) ) != NULL ) 
    { 
        if ( GetObject( hFont, sizeof(LOGFONT), (LPVOID)&lFont ) ) 
        { 
            HIMC hImc; 
            if ( (  hImc = ImmGetContext( hWnd ) ) ) 
			{ 
				ImmSetCompositionFont( hImc, &lFont ); 
				ImmReleaseContext( hWnd, hImc ); 
			} 
		} 
	} 
    // Get the property and apiabilities of current keyboard layout(IME). 
    // If the keyboard layout is US, the return value will be zero. 
    gImeUIData.fdwProperty = ImmGetProperty( GetKeyboardLayout(0), IGP_PROPERTY ); 
    // Initialize candidate list window array. 
    for( i = 0; i < MAX_LISTCAND; i++ ) 
    { 
         gImeUIData.hListCand[ i ] = NULL; 
         gImeUIData.hListCandMem[ i ] = NULL; 
    } 
 
    PostMessage( hWnd, WM_COMMAND, IDC_CLEAR, 0L ); 
// Initialise the current keyboard layout. 
    hCurKL = GetKeyboardLayout(0L); 
} 
/************************************************************************ 
* 
*   ResetCaret - Reset caret shape to match input mode (overtype/insert) 
* 
************************************************************************/ 
 
void ResetCaret( HWND hWnd ) 
{ 
 
    HideCaret( hWnd ); 
    DestroyCaret(); 
    CreateCaret( hWnd, 
 NULL, 
 (fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ? 
   CaretWidth*2 : CaretWidth, 
 cyMetrics ); 
    SetCaretPos( xPos * cxMetrics, yPos * cyMetrics ); 
 
    if ( !( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) && 
         !( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) ) 
    { 
// near caret. 
        SetIMECompFormPos( hWnd ); 
    } 
  
    ShowCaret( hWnd ); 
 
} 
//********************************************************************** 
// 
// SetIMECompFormPos() 
// 
//********************************************************************** 
void SetIMECompFormPos( HWND hWnd ) 
{ 
 
    HIMC hIMC = ImmGetContext(hWnd); 
    POINT   point; 
    COMPOSITIONFORM CompForm; 
 
    GetCaretPos( &point ); 
 
    CompForm.dwStyle = CFS_POINT; 
 
    CompForm.ptCurrentPos.x = (long) point.x; 
    CompForm.ptCurrentPos.y = (long) point.y; 
 
    if ( hIMC ) 
ImmSetCompositionWindow(hIMC,&CompForm); 
    ImmReleaseContext( hWnd , hIMC); 
 
} 
/************************************************************************ 
* 
*   SteCommand - WM_COMMAND handler 
* 
************************************************************************/ 
void SteCommand( HWND hWnd, UINT cmd, LPARAM lParam ) 
{ 
    switch( cmd ) 
    { 
    case IDC_CLEAR: 
// Blank out text buffer.  Return caret to home position 
		for ( yPos = FIRSTROW; yPos <= LASTROW; yPos++ ) 
			for ( xPos = FIRSTCOL; xPos <= LASTCOL; xPos++ ) 
				textbuf[yPos][xPos] = ' '; 
	break; 
    case IDC_ANSIFILL: 
    case IDC_DBCSFILL: 
// Fill text buffer with ANSI or DBCS pattern 
		for ( yPos = FIRSTROW; yPos <= LASTROW; yPos++ ) 
			for ( xPos = FIRSTCOL; xPos <= LASTCOL; xPos++ ) 
				if ( cmd == IDC_ANSIFILL ) 
					textbuf[yPos][xPos] = 'a'; 
				else
				{ 
					textbuf[yPos][xPos]   = HIBYTE(DBCSFillChar); 
					textbuf[yPos][++xPos] = LOBYTE(DBCSFillChar); 
				} 
	break; 
    // The following messages are to control IME. 
    case IDC_OPENIME: 
		SteImeOpenClose( hWnd, TRUE ); 
		goto RETURN; 
 
    case IDC_CLOSEIME: 
		SteImeOpenClose( hWnd, FALSE ); 
		goto RETURN; 
    } // end switch( cmd ) 
 
    yPos = FIRSTROW; 
    xPos = FIRSTCOL; 
 
    InvalidateRect( hWnd, (LPRECT)NULL, TRUE ); 
    ResetCaret(hWnd); 
 
RETURN: 
    return; 
} 
/************************************************************************ 
* 
*   IsDBCSTrailByte - returns TRUE if the given byte is a DBCS trail byte 
* 
*                     The algorithm searchs backward in the string, to some 
*                     known character boundary, counting consecutive bytes 
*                     in the lead byte range. An odd number indicates the 
*                     current byte is part of a two byte character code. 
* 
*   INPUT: PCHAR  - pointer to a preceding known character boundary. 
*          PCHAR  - pointer to the character to test. 
* 
*   OUTPUT:BOOL   - indicating truth of p==trailbyte. 
* 
************************************************************************/ 
BOOL IsDBCSTrailByte( char *base, char *p ) 
{ 
    int lbc = 0;    // lead byte count 
 
    assert(base <= p); 
 
    while ( p > base )
	{ 
		if ( !IsDBCSLeadByte(*(--p)) ) break; 
		lbc++; 
    } 
 
    return (lbc & 1); 
} 
//********************************************************************** 
// 
// BOOL MoveCaret() 
// 
//********************************************************************** 
BOOL MoveCaret( HWND hwnd ) 
{ 
    HIMC        hIMC; 
    BOOL        retVal = TRUE; 
	 
    if ( !( hIMC = ImmGetContext( hwnd ) ) ) return retVal;                 
 
    if ( ImmGetCompositionString( hIMC, GCS_CURSORPOS, (void FAR *)NULL, 0 ) ) 
		retVal = FALSE;                                     
 
    ImmReleaseContext( hwnd, hIMC ); 
 
    return retVal; 
} 
/************************************************************************ 
* 
*   VirtualKeyHandler - WM_KEYDOWN handler 
* 
* 
*   INPUT:  HWND - handle to the window for repainting output. 
*           UINT - virtual key code. 
* 
************************************************************************/ 
void VirtualKeyHandler( HWND hWnd, UINT wParam ) 
{ 
    int i; 
    HDC hdc; 
    static int delta = 1; 
 
    if ( ( gImeUIData.ImeState & IME_IN_CHOSECAND ) || 
         ( gImeUIData.ImeState & IME_IN_COMPOSITION && !MoveCaret( hWnd ) ) ) 
	{
		 return; 
	}
//	
    switch( wParam ) 
    { 
    case VK_HOME:   // beginning of line 
		xPos = FIRSTCOL; 
	break; 
 
    case VK_END:    // end of line 
		xPos = LASTCOL; 
	goto check_for_trailbyte; 
 
    case VK_RIGHT: 
		if ( IsDBCSLeadByte( textbuf[yPos][xPos] ) )
		{ 
			if (xPos==LASTCOL - 1) break;  //last character don't move 
			xPos += 2;                     //skip 2 for DB Character 
		} 
		else
		{
			xPos = min( xPos+1, LASTCOL ); 
		}
	break; 
 
    case VK_LEFT: 
		xPos = max( xPos-1, FIRSTCOL ); 
check_for_trailbyte: 
		if ( IsDBCSTrailByte( textbuf[yPos], &(textbuf[yPos][xPos]) ) ) xPos--; 
	break; 
 
    case VK_UP: 
		yPos = max( yPos-1, FIRSTROW ); 
		goto Virtical_Check_Trail; 
 
    case VK_DOWN: 
		yPos = min( yPos+1, LASTROW ); 
Virtical_Check_Trail: 
		if ( IsDBCSTrailByte( textbuf[yPos], &(textbuf[yPos][xPos]) ) )
		{ 
		if (xPos<LASTCOL)
		{ 
			xPos+=delta; 
			delta *= -1; 
		} 
		else
		{
			xPos--;
		}
	} 
	break; 

    case VK_INSERT: 
// Change caret shape to indicate insert/overtype mode 
		fInsertMode = !fInsertMode; 
		CaretWidth = fInsertMode ? cxMetrics : cxOverTypeCaret; 
	break; 
 
	case VK_BACK:   // backspace 
		if ( xPos > FIRSTCOL )
		{ 
			xPos--; 
		    // DB Character so backup one more to allign on boundary 
		    if ( IsDBCSTrailByte( textbuf[yPos], &(textbuf[yPos][xPos]) ) ) xPos--; 
			// Fall Through to VK_DELETE to adjust row 
		} 
		else     //FIRST COLUMN  don't backup -- this would change for wrapping 
		{
			break;
		}
 // Fall Through
    case VK_DELETE: 
		if ( !IsDBCSLeadByte( textbuf[yPos][xPos] ) )
		{ 
		    // Move rest of line left by one, then blank out last character 
			for ( i = xPos; i < LASTCOL; i++ ) textbuf[yPos][i] = textbuf[yPos][i+1]; 
			textbuf[yPos][LASTCOL] = ' '; 
		}
		else
		{ 
	    // Move line left by two bytes, blank out last two bytes 
		    for ( i = xPos; i < LASTCOL-1; i++ ) textbuf[yPos][i] = textbuf[yPos][i+2]; 
			textbuf[yPos][LASTCOL-1] = ' '; 
			textbuf[yPos][LASTCOL]   = ' '; 
		} 
// Repaint the entire line 
		hdc = GetDC( hWnd ); 
		HideCaret( hWnd ); 
		TextOut( hdc, 0, yPos*cyMetrics, textbuf[yPos], MAXCOL ); 
		ReleaseDC( hWnd, hdc ); 
	break; 
 
    case VK_TAB:    // tab  -- tabs are column allignment not character 
	{ 
		int xTabMax = xPos + TABSTOP; 
		int xPosPrev; 
		do
		{ 
			xPosPrev = xPos; 
			SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L ); 
		} while ( (xPos % TABSTOP) && 
		(xPos < xTabMax) && 
		(xPos != xPosPrev)); 
	} 
	break; 
 
    case VK_RETURN: // linefeed 
		yPos = min( yPos+1, LASTROW ); 
		xPos = FIRSTCOL; 
	break; 
    } // END switch( wParam ) 
 
    ResetCaret( hWnd ); 
} 
/************************************************************************ 
* 
*   StoreChar - Stores one SBCS character into text buffer and advances 
*               cursor 
* 
************************************************************************/ 
void StoreChar( HWND hWnd, UCHAR ch ) 
{ 
    int i; 
    HDC hdc; 
    // If insert mode, move rest of line to the right by one 
    if ( fInsertMode )
	{ 
		for ( i = LASTCOL; i > xPos; i-- ) textbuf[yPos][i] = textbuf[yPos][i-1]; 
// If the row ends on a lead byte, blank it out 
// To do this we must first traverse the string 
// starting from a known character boundry until 
// we reach the last column. If the last column 
// is a character boundry then the last character 
// is either a single byte or a lead byte 
		for ( i = xPos+1; i < LASTCOL; )
		{ 
			if ( IsDBCSLeadByte( textbuf[yPos][i] ) ) i++; 
			i++; 
		} 
		if (i==LASTCOL) 
		if ( IsDBCSLeadByte( textbuf[yPos][LASTCOL] ) ) textbuf[yPos][LASTCOL] = ' '; 
 
    }
	else
	{  // overtype mode 
		if ( IsDBCSLeadByte( textbuf[yPos][xPos] ) ) 
    // Blank out trail byte 
		    textbuf[yPos][xPos+1] = ' '; 
    // or shift line left on character and blank last column 
    // 
    // for ( i = xPos+1; i < LASTCOL; i++ ) 
    //     textbuf[yPos][i] = textbuf[yPos][i+1]; 
    // textbuf[yPos][LASTCOL] = ' '; 
 
    } 
    // Store input character at current caret position 
    textbuf[yPos][xPos] = ch; 
    // Display input character. 
    hdc = GetDC( hWnd ); 
    HideCaret( hWnd ); 
    TextOut( hdc, xPos*cxMetrics, yPos*cyMetrics, &(textbuf[yPos][xPos]), MAXCOL-xPos ); 
    ShowCaret( hWnd ); 
    ReleaseDC( hWnd, hdc ); 
 
    SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L ); 
} 
/************************************************************************ 
* 
*   StoreDBCSChar - Stores one DBCS character into text buffer and 
*                   advances cursor 
* 
************************************************************************/ 
void StoreDBCSChar( HWND hWnd, WORD ch ) 
{ 
    int i; 
    HDC hdc; 
    // If there is no room for a DBCS character, discard it 
    if ( xPos == LASTCOL ) return; 
    // If insert mode, move rest of line to the right by two 
    if ( fInsertMode )
	{ 
		for ( i = LASTCOL; i > xPos+1; i-- ) textbuf[yPos][i] = textbuf[yPos][i-2]; 
// If the row ends on a lead byte, blank it out 
// To do this we must first traverse the string 
// starting from a known charcter boundry until 
// we reach the last column. If the last column 
// is not a trail byte then it is a single byte 
// or a lead byte 
		for ( i = xPos+2; i < LASTCOL; )
		{ 
			if ( IsDBCSLeadByte( textbuf[yPos][i] ) ) i++; 
			i++; 
		} 
		if (i==LASTCOL) 
			if (IsDBCSLeadByte( textbuf[yPos][LASTCOL] ) ) textbuf[yPos][LASTCOL] = ' '; 
 
    }
	else
	{  // overtype mode 
		if ( !IsDBCSLeadByte( textbuf[yPos][xPos] ) ) 
    // Overtyping the current byte, plus the following byte, 
    // which could be a lead byte. 
		if ( IsDBCSLeadByte( textbuf[yPos][xPos+1] ) ) textbuf[yPos][xPos+2] = ' '; 
    } 
    // Store input character at current caret position 
    textbuf[yPos][xPos]   = LOBYTE(ch);     // lead byte 
    textbuf[yPos][xPos+1] = HIBYTE(ch);     // trail byte 
    // Display input character. 
    hdc = GetDC( hWnd ); 
    HideCaret( hWnd ); 
    TextOut( hdc, xPos*cxMetrics, yPos*cyMetrics, &(textbuf[yPos][xPos]), MAXCOL-xPos ); 
    ShowCaret( hWnd ); 
    ReleaseDC( hWnd, hdc ); 
 
    SendMessage( hWnd, WM_KEYDOWN, VK_RIGHT, 1L ); 
} 
/************************************************************************ 
* 
*   CharHandler - WM_CHAR handler 
* 
************************************************************************/ 
void CharHandler( HWND hWnd, WORD wParam ) 
{ 
    unsigned char ch = (unsigned char)wParam; 
 
    // 
    // Because DBCS characters are usually generated by IMEs (as two 
    // PostMessages), if a lead byte comes in, the trail byte should 
    // arrive very soon after.  We wait here for the trail byte and 
    // store them into the text buffer together. 
 
    if ( IsDBCSLeadByte( ch ) )
	{ 
// Wait an arbitrary amount of time for the trail byte to 
// arrive.  If it doesn't, then discard the lead byte. 
// 
// This could happen if the IME screwed up.  Or, more likely, 
// the user generated the lead byte through ALT-numpad. 
// 
 
		MSG msg; 
		int i = 10; 
 
		while (!PeekMessage((LPMSG)&msg, hWnd, WM_CHAR, WM_CHAR, PM_REMOVE))
		{ 
			if ( --i == 0 ) return; 
			Yield(); 
		} 
 
		StoreDBCSChar( hWnd,  (WORD)(((unsigned)(msg.wParam)<<8) | (unsigned)ch )); 
 
    }
	else
	{ 
		switch( ch ) 
		{ 
		case '\r': 
		case '\t': 
		case '\b': 
			// Throw away.  Already handled at WM_KEYDOWN time. 
			break; 
		default: 
			StoreChar( hWnd, ch ); 
			break; 
		} 
    } 
} 
/************************************************************************ 
* 
*   MouseHandler - WM_BUTTONDOWN handler 
* 
************************************************************************/ 
void MouseHandler( HWND hWnd, LONG lParam ) 
{ 
 
    if ( ( gImeUIData.ImeState & IME_IN_CHOSECAND ) || 
         ( gImeUIData.ImeState & IME_IN_COMPOSITION && !MoveCaret( hWnd ) ) ) 
	{
		return;
	}
 
    HideCaret( hWnd ); 
    // Calculate caret position based on fixed pitched font 
    yPos = MAKEPOINTS(lParam).y / cyMetrics; 
    xPos = MAKEPOINTS(lParam).x / cxMetrics; 
    // Adjust caret position if click landed on a trail byte 
    if ( IsDBCSTrailByte( textbuf[yPos], &(textbuf[yPos][xPos]) ) ) 
// If click landed on the last quarter of the DBCS character, 
// assume the user was aiming at the next character. 
		if ( (MAKEPOINTS(lParam).x - xPos * cxMetrics) > (cxMetrics / 2) ) 
			xPos++; 
	else 
		xPos--; 
 
    DestroyCaret(); 
    CreateCaret( hWnd, NULL, (fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ? 
								CaretWidth*2 : CaretWidth, 
								cyMetrics ); 
    SetCaretPos( xPos * cxMetrics, yPos * cyMetrics ); 
    ShowCaret( hWnd ); 
} 
/************************************************************************ 
* 
*   InputChangeHandler - WM_INPUTLANGCHANGE handler 
* 
************************************************************************/ 
void InputChangeHandler( HWND hWnd ) 
{ 
    HIMC hIMC; 
    // If the old keyboard layout is IME, the ime ui data have to be free. 
    if (ImmIsIME(hCurKL)) 
    { 
        // If application prefers to use near caret provded by IME, or 
        // IME provides special UI, then no need to clean UD data. 
        if ( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) 
            ; 
        else if ( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) 
            ImeUIClearData(hWnd); 
        else 
            ; 
    } 
// Set new keyboard layout. 
    hCurKL = GetKeyboardLayout(0L); 
// Get new property. 
    gImeUIData.fdwProperty = ImmGetProperty( hCurKL, IGP_PROPERTY ); 
    // if this application set the candidate position, it need to set 
    // it to default for the near caret IME 
 
    if ( hIMC = ImmGetContext( hWnd ) )
	{ 
        UINT i; 
 
        for (i = 0; i < 4; i++)
		{ 
            CANDIDATEFORM CandForm; 
 
            if ( gImeUIData.fdwProperty & IME_PROP_AT_CARET )
			{ 
                CandForm.dwIndex = i; 
                CandForm.dwStyle = CFS_CANDIDATEPOS; 
 
#if 0           // This application do not want to set candidate window to 
                // any position. Anyway, if an application need to set the 
                // candiadet position, it should remove the if 0 code 
 
                // the position you want to set 
                CandForm.ptCurrentPos.x = ptAppWantPosition[i].x; 
                CandForm.ptCurrentPos.y = ptAppWantPosition[i].y; 
 
                ImmSetCandidateWindow( hIMC, &CandForm ); 
#endif 
            }
			else
			{ 
                if ( !ImmGetCandidateWindow( hIMC, i, &CandForm ) ) continue; 
                if ( CandForm.dwStyle == CFS_DEFAULT )              continue; 
 
                CandForm.dwStyle = CFS_DEFAULT; 
 
                ImmSetCandidateWindow( hIMC, &CandForm ); 
            } 
        } 
 
        ImmReleaseContext( hWnd, hIMC ); 
    } 
 
    return; 
} 
/************************************************************************ 
* 
*   SteWndProc - STE class window procedure 
* 
************************************************************************/ 
long WINAPI SteWndProc( HWND hWnd, UINT msg, UINT wParam, LONG lParam ) 
{ 
    int i; 
    HDC hdc; 
    PAINTSTRUCT ps; 
 
    switch( msg )
	{ 
    case WM_CREATE: 
		SteCreate( hWnd ); 
	break; 
 
    case WM_DESTROY: 
		PostQuitMessage(0); 
	break; 
 
    case WM_CLOSE: 
		DestroyWindow( hWnd ); 
	break; 
 
    case WM_SETFOCUS: 
		CreateCaret( hWnd, NULL,(fInsertMode && IsDBCSLeadByte( textbuf[yPos][xPos] )) ? 
									CaretWidth*2 : CaretWidth, 
									cyMetrics ); 
		SetCaretPos( xPos * cxMetrics, yPos * cyMetrics ); 
		ShowCaret( hWnd ); 
	break; 
 
    case WM_KILLFOCUS: 
		HideCaret( hWnd ); 
		DestroyCaret(); 
	break; 
 
    case WM_IME_KEYDOWN: 
    case WM_KEYDOWN: 
		VirtualKeyHandler( hWnd, wParam ); 
	break; 
 
    case WM_KEYUP: 
	break; 
 
    case WM_CHAR: 
		CharHandler( hWnd, (WORD)wParam ); 
	break; 
 
    case WM_LBUTTONDOWN: 
		MouseHandler( hWnd, lParam ); 
	break; 
 
    case WM_MOVE: 
		ImeUIMoveCandWin( hWnd ); 
	break; 
 
    case WM_COMMAND: 
		SteCommand( hWnd, wParam, lParam ); 
	break; 
 
    case WM_PAINT: 
		//InvalidateRect(hWnd,NULL,FALSE);  //for repaint allignment problem?? 
		// WinChi3.0 
		hdc = BeginPaint( hWnd, &ps ); 
// Refresh display from text buffer 
		for ( i = FIRSTROW; i <= LASTROW; i++ ) TextOut(hdc,0,i*cyMetrics,textbuf[i],MAXCOL); 
		EndPaint( hWnd, &ps ); 
 
		RestoreImeUI( hWnd ); 
	break; 
 
    case WM_INPUTLANGCHANGE: 
        InputChangeHandler( hWnd ); 
        goto call_defwinproc; 
	break; 
 
    case WM_IME_SETCONTEXT: 
        // The application have to pass WM_IME_SETCONTEXT to DefWindowProc. 
// When the application want to handle the IME at the timing of 
        // focus changing, the application should use WM_SETFOCUS or 
        // WM_KILLFOCUS. 
        if ( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) 
            goto call_defwinproc; 
        else if ( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) 
        {  // application wants to draw UI ny itself. 
            lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW); 
		} 
	return DefWindowProc( hWnd, msg, wParam, lParam ); 
 
    case WM_IME_STARTCOMPOSITION: 
// CheckProperty is a macro, if IME already provides near caret or 
// special UI then let IME handle this message. 
		CheckProperty; 
        ImeUIStartComposition( hWnd ); 
	break; 
 
    case WM_IME_COMPOSITION: 
		CheckProperty; 
        ImeUIComposition( hWnd, wParam, lParam ); 
    break; 
 
    case WM_IME_ENDCOMPOSITION: 
		CheckProperty; 
        ImeUIEndComposition( hWnd ); 
    break; 
 
    case WM_IME_COMPOSITIONFULL: 
        // Make sure the size for drawing the composition string. 
        // Application should draw the composition string correctly. 
        //  
    break; 
 
    case WM_IME_NOTIFY: 
		CheckProperty; 
        if ( !ImeUINotify( hWnd, wParam, lParam ) ) 
        // This application does not handle all notification message. 
        // So we pass those notification messages which are not hanlded 
        // by this application to the DefWindowProc. 
            goto call_defwinproc; 
    break; 
    case WM_IME_CONTROL: 
        // This message is not received by the application window. 
        // But don't pass it to DefWindowProc(). 
    break; 

    default: 
		call_defwinproc: 
		return DefWindowProc( hWnd, msg, wParam, lParam ); 
    }// END switch( msg ) 
 
    return 0; 
} 
