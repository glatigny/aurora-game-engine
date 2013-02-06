#ifdef WIN32

/*--- INCLUDES ---*/

#include "win32_init_window.h"
#include "../../aurora_consts.h"

/*--- DEFINES ---*/

#define AURORA_WIN32_CLASSNAME		L"AuroraGameWindow"
#define AURORA_WIN32_WINDOWNAME		L".: Aurora Studio :."

/*--- VARIABLES ---*/

extern HINSTANCE hInstance;
extern bool win32WindowClosed;

#ifdef MOUSE_SUPPORT
int mouseX;
int mouseY;
#endif

/*--- CODE ---*/

HWND CreateAuroraWindow() 
{
	WNDCLASSEX windowClass;	// Structure contenant les infos pour la classe fenêtre
	HWND hWnd = NULL;		// Handle pour la fenêtre applicative

	// Remplissage de la structure d'information 
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WindowProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);			
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);	
	windowClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName	= NULL;						
	windowClass.lpszClassName	= AURORA_WIN32_CLASSNAME;
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);	

	// Enregistrement de la classe fenêtre avec contrôle d'erreur
	if (!RegisterClassEx(&windowClass))
		return NULL;
	
	/*FIXME*/
	// Gestion de la taille de la fenetre et de sa position

	// Creation de la fenetre
	hWnd = CreateWindowEx(
		NULL,
		AURORA_WIN32_CLASSNAME,
		AURORA_WIN32_WINDOWNAME,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		50,
		50,
		800,
		600,
 		NULL,
		NULL,
		hInstance,
		NULL);	

	// Contrôle d'erreur a la creation
	if (!hWnd)
		return NULL;

#ifdef MOUSE_SUPPORT
	mouseX = 0;
	mouseY = 0;
#endif

	// On masque le curseur de souris
	ShowCursor(false);
	return hWnd;
}

/*------------------------------------------------------------------------------*/

void DestroyAuroraWindow()
{
	UnregisterClass(AURORA_WIN32_CLASSNAME, hInstance);
}

/*------------------------------------------------------------------------------*/

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			return 0;
		
		case WM_CLOSE:
			win32WindowClosed = true;
			// XXX
			exit(0);
			return 1;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			
#ifdef MOUSE_SUPPORT
		case WM_MOUSEMOVE:
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);
			break;
		case WM_LBUTTONDOWN:
			break;
		case WM_LBUTTONUP:
			break;
		case WM_RBUTTONDOWN:
			break;
		case WM_RBUTTONUP:
			break;
		case WM_MBUTTONDOWN:
			break;
		case WM_MBUTTONUP:
			break;
		case WM_MOUSELEAVE:
			break;
		case WM_MOUSEWHEEL:
			break;
#endif

		default:
			break;
	}

	return (DefWindowProc(hWnd, message, wParam, lParam));
}

/*------------------------------------------------------------------------------*/

#endif /* WIN32 */
