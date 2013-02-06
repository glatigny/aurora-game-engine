#ifdef WIN32
#  ifndef WIN32_INIT_WINDOW_H
#  define WIN32_INIT_WINDOW_H

/*--- INCLUDES ---*/
#  include <windows.h>

/*--- VARIABLES ---*/
#ifdef MOUSE_SUPPORT
extern int mouseX;
extern int mouseY;
#endif

/*--- DEFINITION ---*/
	// Pour creer la fenetre
	HWND CreateAuroraWindow();

	// Pour detruire la fenetre
	void DestroyAuroraWindow();

	// Les loopback
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*-EOF-*/
#  endif /* WIN32_INIT_WINDOW_H */
#endif /* WIN32 */

