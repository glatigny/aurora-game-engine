#ifndef HARD_DIRECT_X_H
#define HARD_DIRECT_X_H

/*--- INCLUDES ---*/
#include "../../hardware_engine.h"
#include "win32_init_window.h"

#define DIRECTINPUT_VERSION 0x800

#  include <d3d9.h>
#  include <d3dx9.h>
#  include <dxerr9.h>
#  include <dinput.h>
#  include <d3dx9math.h>

/*--- DEFINES ---*/

/*--- VARIABLES ---*/

/*--- DEFINITION ---*/
class GraphicalEngineDX : public GraphicalEngine { friend class InputEngineDX;
	private:
		static HWND		hWnd;

		//bool init;
		bool windowClosed;

		// Graphic
		IDirect3D9			*DxInterface;
		IDirect3DDevice9	*DxDevice;
		ID3DXSprite			*DxSpriteManager;

		/*-- FUNCTIONS --*/

		// Constructeurs
		GraphicalEngineDX();

		/** Specific hardware Engine texture loading
		 * @param filename of texture we want to load
		 * @return texture pointer or NULL if failed
		 */
		Texture*	myLoadTexture(const char* p_file);
		/** Specific hardware Engine texture releasing
		 * @param texture pointer we want to release
		 * @return result of the operation
		 */
		bool		myFreeTexture(Texture* p_texture);
		
		/** Specific hardware Engine initialization of Graphic Engine.
		 */
		bool		initGraphics();

	public:
		static GraphicalEngineDX* open();
		~GraphicalEngineDX();

		/** Initialize the drawing phase.
		 */
		void preDraw();
		/** Terminate the drawing phase.
		 */
		void postDraw();
		/** Draw a texture on the screen using graphical effects.
		 * @param texture to draw.
		 * @param the position of the texture on the screen.
		 * @param graphical effects to use.
		 */
		void draw(Texture* p_texture, Translatable* p_coordinates, GraphicalEffect* p_effect);

		/** Indicate if this is the end of the game, if someone close the windows or ask for stop the program.
		 * @return true if this is the end of the game, false if not.
		 */
		bool isTheEnd();
};

/*---- DirectX Input Def ----*/
#ifdef MAKE_KEYTABLE
#undef MAKE_KEYTABLE
#endif

/*-- KEYTABLE : binds keys to constants --*/
#define MAKE_KEYTABLE(keys) do {		\
	keys[ KEY_up ]		= DIK_UP;	\
	keys[ KEY_down ]	= DIK_DOWN;	\
	keys[ KEY_left ]	= DIK_LEFT;	\
	keys[ KEY_right ]	= DIK_RIGHT;	\
						\
	keys[ KEY_A ]		= DIK_LCONTROL;	\
	keys[ KEY_B ]		= DIK_LSHIFT;	\
						\
	keys[ KEY_X ]		= DIK_Q;	\
	keys[ KEY_Y ]		= DIK_W;	\
	keys[ KEY_L ]		= DIK_A;	\
	keys[ KEY_R ]		= DIK_S;	\
						\
	keys[ KEY_Start ]	= DIK_RETURN;	\
	keys[ KEY_Select ]	= DIK_ESCAPE;	\
						\
	keys[ KEY_StateStep ]	= DIK_P;	\
	keys[ KEY_StatePlay ]	= DIK_O;	\
	keys[ KEY_IncreaseStateStep ]	= DIK_M;	\
	keys[ KEY_DecreaseStateStep ]	= DIK_L;	\
} while(0) /*-- KEYTABLE end --*/

/*---- End of directx input def ----*/

class InputEngineDX : public InputEngine {
	private:
		// Input
		LPDIRECTINPUT			DxInput;
		LPDIRECTINPUTDEVICE		DxKeyboard;
		unsigned int			m_keys[KEY_COUNT];
		LPDIRECTINPUTDEVICE2	DxJoystick;
		DIJOYSTATE				*joystick;

#ifdef MOUSE_SUPPORT
		LPDIRECTINPUTDEVICE8	DxMouse;
		Renderable*				mouseCursor;
		int						pointerX, pointerY;
		int						mouseMode;
#endif
		/*-- FUNCTIONS --*/
			
		// Constructeurs
		InputEngineDX();

		/** Specific hardware Engine process controller.
		 * In this case, we use the keyboard.
		 * @param pointer of the ControllerState we want to modify
		 */
		void		myProcessController(ControllerState *p_controller);

		/** Specific hardware Engine initialization of Input Engine.
		 * In this case, we use the keyboard.
		 */
		void		initKeyboard();
#ifdef MOUSE_SUPPORT
		void		initMouse();
#endif


	public:
		static InputEngineDX* open();
		~InputEngineDX();

#ifdef MOUSE_SUPPORT
		virtual bool relativeMouse();
		virtual bool absoluteMouse();
		virtual bool releaseMouse();
		virtual void setMouseCursor(Renderable* newCursor);
#endif
};

#endif /* HARD_DIRECT_X_H */
