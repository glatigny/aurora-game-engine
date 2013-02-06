/*--- INCLUDES ---*/
#include "directx.h"
#include "tinyxml.h"

/*--- DEFINES ---*/

/*--- VARIABLES ---*/
HWND GraphicalEngineDX::hWnd = NULL;
bool win32WindowClosed = false;

/*--- CODE ---*/

GraphicalEngineDX* GraphicalEngineDX::open() {
	GraphicalEngineDX* engine = new GraphicalEngineDX();
	if( engine->initGraphics() )
		return engine;
	return NULL;
}

/*------------------------------------------------------------------------------*/

GraphicalEngineDX::GraphicalEngineDX() : GraphicalEngine() 
{
	//init			= false;
	DxInterface		= NULL;
	DxDevice		= NULL;
	DxSpriteManager	= NULL;
}

/*------------------------------------------------------------------------------*/

GraphicalEngineDX::~GraphicalEngineDX() 
{
	if(DxInterface != NULL)
		DxInterface->Release();
	DxInterface		= NULL;

	if(DxDevice != NULL)
		DxDevice->Release();
	DxDevice		= NULL;

	if(DxSpriteManager != NULL)	
		DxSpriteManager->Release();
	DxSpriteManager	= NULL;

	/*- FENETRE -*/
	DestroyAuroraWindow();
}

/*------------------------------------------------------------------------------*/

bool GraphicalEngineDX::initGraphics()
{
	GraphicalEngineDX::hWnd = CreateAuroraWindow();

	/*- Creation Interface -*/
	// En mode D3D SDK
	DxInterface = Direct3DCreate9(D3D_SDK_VERSION);
	if (DxInterface == NULL) return false;
	
	/*- Display Mode -*/
	// Recuperation de l'affichage en cours
	D3DDISPLAYMODE dxdm;
	DxInterface->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dxdm);

	/*-  Parametres -*/
	// Permet de recuperer des informations pour la creation de l'interface
	D3DPRESENT_PARAMETERS dxpp = {0};
	ZeroMemory(&dxpp, sizeof(dxpp));

	dxpp.BackBufferWidth			= CST_WIDTH / CST_DIVISOR;
	dxpp.BackBufferHeight			= CST_HEIGHT / CST_DIVISOR;
	dxpp.BackBufferCount			= 1;
	dxpp.BackBufferFormat			= dxdm.Format;
	dxpp.SwapEffect					= D3DSWAPEFFECT_COPY;
	dxpp.Windowed					= !CST_FULLSCREEN;
	dxpp.EnableAutoDepthStencil		= true;
	dxpp.AutoDepthStencilFormat		= D3DFMT_D16;
	dxpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
										/* Defaut => 0% CPU  |  Immediate => 100% CPU */
	dxpp.PresentationInterval		= D3DPRESENT_INTERVAL_DEFAULT/*D3DPRESENT_INTERVAL_IMMEDIATE*/;

	/*- Creation Device :: HARDWARE MODE -*/
	bool init = false;

#define INITDX(devtype) do { if( !init && !FAILED( DxInterface->CreateDevice( D3DADAPTER_DEFAULT, devtype, GraphicalEngineDX::hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,&dxpp,&DxDevice))) { init = true; } \
if( !init && !FAILED( DxInterface->CreateDevice( D3DADAPTER_DEFAULT, devtype, GraphicalEngineDX::hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,&dxpp,&DxDevice))) { init = true; } } while(0)

	// On lance la creation du device en mode hardware
	INITDX( D3DDEVTYPE_HAL );
	// On lance la creation du device en mode software
	INITDX( D3DDEVTYPE_SW );
	// On lance la creation du device en mode d'implementation de reference 
	INITDX( D3DDEVTYPE_REF );

#undef INITDX
	
	if( !init )
		return false;	

	/*- Creation Sprite -*/
	// Creation du Spirte Manager
	D3DXCreateSprite(DxDevice, &DxSpriteManager);
	return true;
}

/*------------------------------------------------------------------------------*/

Texture* GraphicalEngineDX::myLoadTexture(const char* p_file)
{
	size_t len = strlen(p_file);
	wchar_t	l_filew[1024];
	mbstowcs(l_filew, p_file, len);
	l_filew[len] = '\0';

	Texture *n_texture = new Texture;

	// Creation d'une image
	LPDIRECT3DTEXTURE9* image = new LPDIRECT3DTEXTURE9;

	// Pour les infos de l'image
	D3DXIMAGE_INFO infosImage;
	ZeroMemory(&infosImage, sizeof(D3DXIMAGE_INFO));

	// Recuperation des informations
	if (FAILED(D3DXGetImageInfoFromFile(l_filew, &infosImage)))
	{
		delete n_texture;
		return NULL; // Si echec, pas de texture
	}

	// Affectation des informations

	n_texture->w = infosImage.Width;
	n_texture->h = infosImage.Height;
	
	// Creation
	D3DXCreateTextureFromFileEx(	DxDevice,
									l_filew,
									infosImage.Width,
									infosImage.Height,
									1,
									0,
									D3DFMT_UNKNOWN,
									D3DPOOL_MANAGED,
									D3DX_DEFAULT,
									D3DX_DEFAULT,
									0xFFFF00FF,
									&infosImage,
									NULL,
									image
									);
	// Affectation de l'image
	n_texture->data = (void*)image;

	return n_texture;
}

/*------------------------------------------------------------------------------*/

bool GraphicalEngineDX::myFreeTexture(Texture* p_texture)
{
	if(p_texture != NULL) {
		try 
		{ 
			(*((LPDIRECT3DTEXTURE9*)p_texture->data))->Release(); 
			delete p_texture;
			return true;
		} 
		catch(...) { }
	}
	return false;
}

/*------------------------------------------------------------------------------*/

void GraphicalEngineDX::preDraw()
{
	DxDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	DxDevice->BeginScene();
	DxSpriteManager->Begin(D3DXSPRITE_ALPHABLEND);
}

/*------------------------------------------------------------------------------*/

void GraphicalEngineDX::postDraw()
{
	DxSpriteManager->End();
	DxDevice->EndScene();
	DxDevice->Present(NULL,NULL,NULL,NULL);
}

/*------------------------------------------------------------------------------*/

void GraphicalEngineDX::draw(Texture *p_texture, Translatable *p_coordinates, GraphicalEffect *p_effect)
{
	// Transformations
	D3DXMATRIX mat_ref, mat_t;

	unsigned char alpha = 255;
	unsigned char redF = 255;
	unsigned char greenF = 255;
	unsigned char blueF = 255;

	int x = p_coordinates->getX() / CST_DIVISOR;
	int y = p_coordinates->getY() / CST_DIVISOR;

	RECT source_rect;
	source_rect.left	= 0;
	source_rect.top		= 0;
	source_rect.right	= p_texture->w;
	source_rect.bottom	= p_texture->h;

	if( p_effect != NULL )
	{
		// Get the matrix
		DxSpriteManager->GetTransform(&mat_ref);

		float scaleX = (float)((float)p_effect->horizontalScale)/100;
		float scaleY = (float)((float)p_effect->verticalScale)/100;
		float rot_x = x+p_effect->rotationCenter.getX();
		float rot_y = y+p_effect->rotationCenter.getY();
		float l_angle = 0.0;

		alpha	= p_effect->opacity;
		redF	= p_effect->redFilter;
		greenF	= p_effect->greenFilter;
		blueF	= p_effect->blueFilter;

		if( p_effect->rotationAngle != 0 )
			l_angle = D3DXToRadian(p_effect->rotationAngle);

		// SECTION
		if( (p_effect->sectionWidthHeight.getX() != 0) && (p_effect->sectionWidthHeight.getY() != 0) )
		{
			source_rect.left	= p_effect->sectionPointStart.getX();
			source_rect.top		= p_effect->sectionPointStart.getY();
			source_rect.right	= p_effect->sectionPointStart.getX() + p_effect->sectionWidthHeight.getX();
			source_rect.bottom	= p_effect->sectionPointStart.getY() + p_effect->sectionWidthHeight.getY();

			x += (p_texture->w/2) - (p_effect->sectionWidthHeight.getX()/2);
			y += (p_texture->h/2) - (p_effect->sectionWidthHeight.getY()/2);
		}

		D3DXMatrixTransformation2D(&mat_t, 
			&D3DXVECTOR2((float)x, (float)y), 
			0.0, 
			&D3DXVECTOR2((float)scaleX, (float)scaleY), 
			&D3DXVECTOR2((float)rot_x, (float)rot_y), 
			-l_angle,
			&D3DXVECTOR2(0.0f, 0.0f));

		DxSpriteManager->SetTransform(&mat_t);
	}

	/*
	// The SetTextureStageState function : http://msdn2.microsoft.com/en-us/library/bb174461.aspx
	// 2 Parameter for the function : http://msdn2.microsoft.com/en-us/library/bb172617.aspx
	// TextureOperator : http://msdn2.microsoft.com/en-us/library/bb172616.aspx
	DxDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	*/

	DxSpriteManager->Draw(	*((LPDIRECT3DTEXTURE9*)p_texture->data),
							&source_rect,
							&D3DXVECTOR3((float)(p_texture->w / 2),(float)(p_texture->h / 2), 0.0f),
							&D3DXVECTOR3((float)x, (float)y, 0.0f),
							D3DCOLOR_ARGB(alpha, redF, greenF, blueF)
						);

	// Set the old matrix
	if( p_effect != NULL )
	{
		// Apply new matrix
		DxSpriteManager->SetTransform(&mat_t);

		if( p_effect->motionBlurAmount > 0 )
		{
			int steps = p_effect->motionBlurAmount / MOTION_BLUR_MIN_STEPPING;
			float stepping = MOTION_BLUR_MIN_STEPPING;
			float x_step, y_step;
			float x_pos_step, y_pos_step;
			unsigned char finalOpacity = p_effect->opacity;

			if(steps > MOTION_BLUR_DETAIL_LEVEL) {
				stepping = (float)p_effect->motionBlurAmount / MOTION_BLUR_DETAIL_LEVEL;
				steps = MOTION_BLUR_DETAIL_LEVEL;
			}

			// Calculate step
			x_step = (cosf( D3DXToRadian( (float)p_effect->motionBlurDirection )) *stepping );
			y_step = (sinf( D3DXToRadian( -(float)p_effect->motionBlurDirection )) *stepping );

			x_pos_step = 0;
			y_pos_step = 0;
			do 
			{
				x_pos_step += x_step;
				y_pos_step += y_step;

				finalOpacity = (unsigned char)(finalOpacity * 2/3);
				
				// Draw
				DxSpriteManager->Draw(	*((LPDIRECT3DTEXTURE9*)p_texture->data),
							&source_rect,
							&D3DXVECTOR3((float)(p_texture->w / 2),(float)(p_texture->h / 2), 0.0f),
							&D3DXVECTOR3(x + x_pos_step, y + y_pos_step, 0.0f),
							D3DCOLOR_ARGB(finalOpacity, p_effect->motionBlurRedFilter, p_effect->motionBlurGreenFilter, p_effect->motionBlurBlueFilter)
						);
			} 
			while(--steps > 0);
		}

		DxSpriteManager->SetTransform(&mat_ref);
	}
}

/*------------------------------------------------------------------------------*/

bool GraphicalEngineDX::isTheEnd()
{
	static MSG msg = {0};

	if( PeekMessage(&msg, GraphicalEngineDX::hWnd, NULL, NULL, PM_REMOVE)) {	
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return win32WindowClosed;
}

/*------------------------------------------------------------------------------*/

InputEngineDX* InputEngineDX::open() {
	return new InputEngineDX();
}

InputEngineDX::InputEngineDX() : InputEngine()
{
	DxJoystick		= NULL;
	DxKeyboard		= NULL;
	initKeyboard();

#ifdef MOUSE_SUPPORT
	DxMouse = NULL;
	initMouse();
#endif
}

/*------------------------------------------------------------------------------*/

InputEngineDX::~InputEngineDX()
{
	if(DxKeyboard != NULL)
		DxKeyboard->Release();
	DxKeyboard = NULL;

	if(DxInput != NULL)
		DxInput->Release();
	DxInput = NULL;
}

/*------------------------------------------------------------------------------*/

void InputEngineDX::initKeyboard()
{
	// Creation de DxInput
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DxInput, NULL);

	// Affectation du clavier sur le DxInput
	DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL);

	// Mode de DxInput
	DxKeyboard->SetDataFormat(&c_dfDIKeyboard);
	DxKeyboard->SetCooperativeLevel(GraphicalEngineDX::hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	DxKeyboard->Acquire();

	MAKE_KEYTABLE(m_keys);

	/*FIXME*/

	TiXmlDocument doc("config_dx.xml");
	if(doc.LoadFile())
	{
		const char* aVal;
		const char* aVal2;
		TiXmlElement* xmlElement = doc.FirstChildElement("Config");
		
		if( xmlElement )
		{
			xmlElement = xmlElement->FirstChildElement("Input");
			if( xmlElement )
			{				
				TiXmlElement* keyElement = xmlElement->FirstChildElement("Key");
				while( keyElement )
				{
					if( (aVal = keyElement->Attribute("id")) )
					{
						if( (aVal2 = keyElement->Attribute("scancode") ) )
						{
							switch( *aVal )
							{
								case 'u':
										m_keys[ KEY_up ]		= atoi(aVal2);
									break;
								case 'd':
										m_keys[ KEY_down ]		= atoi(aVal2);
									break;
								case 'l':
										m_keys[ KEY_left ]		= atoi(aVal2);
									break;
								case 'r':
										m_keys[ KEY_right ]		= atoi(aVal2);
									break;
								case 'A':
										m_keys[ KEY_A ]			= atoi(aVal2);
									break;
								case 'B':
										m_keys[ KEY_B ]			= atoi(aVal2);
									break;
								case 'X':
										m_keys[ KEY_X ]			= atoi(aVal2);
									break;
								case 'Y':
										m_keys[ KEY_Y ]			= atoi(aVal2);
									break;
								case 'L':
										m_keys[ KEY_L ]			= atoi(aVal2);
									break;
								case 'R':
										m_keys[ KEY_R ]			= atoi(aVal2);
									break;
								case 's':
										if( aVal[1] == 't' )
										{
											m_keys[ KEY_Start ]	= atoi(aVal2);
										}
										else
										{
											m_keys[ KEY_Select ]= atoi(aVal2);
										}
									break;
								
							}
						}
					}
					keyElement = keyElement->NextSiblingElement();
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------*/

#ifdef MOUSE_SUPPORT
void InputEngineDX::initMouse()
{
	releaseMouse();
	mouseCursor = NULL;
	setMouseCursor(NULL);

	LPDIRECTINPUTDEVICE lpdiid;

	if( FAILED(DxInput->CreateDevice(GUID_SysMouse, &lpdiid, NULL)) )
		return;

	if( FAILED(lpdiid->QueryInterface(IID_IDirectInputDevice8,(LPVOID*)&DxMouse)))
		return;

	DxMouse->SetDataFormat( &c_dfDIMouse2 );
	DxMouse->SetCooperativeLevel( GraphicalEngineDX::hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
	DxMouse->Acquire();
}

/*------------------------------------------------------------------------------*/

bool InputEngineDX::relativeMouse()
{
	mouseMode=MOUSE_RELATIVE;
	DxMouse->Unacquire();
	DxMouse->SetCooperativeLevel( GraphicalEngineDX::hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );
	DxMouse->Acquire();
	return true;
}

/*------------------------------------------------------------------------------*/

bool InputEngineDX::absoluteMouse()
{
	mouseMode=MOUSE_ABSOLUTE;
	return true;
}

/*------------------------------------------------------------------------------*/

bool InputEngineDX::releaseMouse()
{
	mouseMode=MOUSE_RELEASED;
	
	if( DxMouse )
		DxMouse->Unacquire();

	return true;
}

/*------------------------------------------------------------------------------*/

void InputEngineDX::setMouseCursor(Renderable* newCursor) {
	if(mouseCursor)
	{
		mouseCursor->objUnref(NULL);
	}
	mouseCursor = newCursor;
	if(mouseCursor)
	{
		mouseCursor->objRef(NULL);
	}
}

#endif

/*------------------------------------------------------------------------------*/

void InputEngineDX::myProcessController(ControllerState *p_controller)
{
	unsigned char	keyboard[256];
	bool			l_keys[12];

	DxKeyboard->GetDeviceState(sizeof(unsigned char[256]), (LPVOID)keyboard);

	// XXX
	if( keyboard[DIK_ESCAPE] )
	{
		win32WindowClosed = true;
		exit(0);
	}

	for(int i = 0; i < 12; i++)
	{
		if( keyboard[ m_keys[i] ] )
		{
			l_keys[i] = true;
		}
		else
		{
			l_keys[i] = false;
		}
	}

	p_controller->setKeyMap(l_keys);
	p_controller->setAxis(0,0);

#ifdef MOUSE_SUPPORT

	HRESULT hr;
	DIMOUSESTATE2 dims2;
	ZeroMemory( &dims2, sizeof(dims2) );

	switch(mouseMode) {
	case MOUSE_RELEASED:
		p_controller->setMousePosition(mouseX-CST_WIDTH/2, mouseY-CST_HEIGHT/2);
		break;

	case MOUSE_ABSOLUTE:
		hr = DxMouse->Poll(); 
		if( FAILED(hr) )
		{
			DxMouse->Acquire();
			DxMouse->Poll();
		}
		DxMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
		mouseX += dims2.lX;
		mouseY += dims2.lY;
		p_controller->setMousePosition(mouseX, mouseY);
		for(int i=0; i<MOUSE_BUTTON_COUNT; i++) {
			p_controller->setMouseButton(i, dims2.rgbButtons[i] & 0x80);
		}
		break;
	
	case MOUSE_RELATIVE:
		hr = DxMouse->Poll(); 
		if( FAILED(hr) )
		{
			DxMouse->Acquire();
			DxMouse->Poll();
		}
		DxMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
		p_controller->setMousePosition(dims2.lX, dims2.lY);
		for( int i=0; i < MOUSE_BUTTON_COUNT; i++)
		{
			p_controller->setMouseButton(i, dims2.rgbButtons[i] & 0x80);
		}
		break;
	
	}
#endif
}

/*------------------------------------------------------------------------------*/
/*
HRESULT InputEngineDX::init_j(HWND hWnd) {
	//DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DxInput, NULL);
	LPDIRECTINPUTDEVICE lpdiid;

	if( FAILED(DxInput->CreateDevice(GUID_Joystick, &lpdiid, NULL)) )
		return S_FALSE;

	if( FAILED(lpdiid->QueryInterface(IID_IDirectInputDevice2,(LPVOID*)&DxJoystick)))
	{
		lpdiid->Release();
		return S_FALSE;
	}
	lpdiid->Release();
	lpdiid = NULL;

	if( FAILED(DxJoystick->SetDataFormat(&c_dfDIJoystick)) )
		return S_FALSE;

	if( FAILED(DxJoystick->SetCooperativeLevel(hWnd, DISCL_FOREGROUND|DISCL_EXCLUSIVE)))
		return S_FALSE;

	if( FAILED(DxJoystick->Acquire()))
		return S_FALSE;

	// structure qui contient les propriétés de l'étendue sur l'axe X et Y
	DIPROPRANGE diprg;
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYOFFSET;	// pour utiliser un format de donné pour dwObj	 
    diprg.diph.dwObj        = DIJOFS_X;			// axe à configuré (X)
    diprg.lMin              = -255;				// étendue sur l'axe X
    diprg.lMax              = +255;				// (nombre de points de contrôles)

	DxJoystick->SetProperty( DIPROP_RANGE, &diprg.diph );

	diprg.diph.dwObj        = DIJOFS_Y;
	DxJoystick->SetProperty( DIPROP_RANGE, &diprg.diph );

//	// structure qui contient la zone de neutralité sur l'axe X et Y
//	DIPROPDWORD dipdw;
//	dipdw.diph.dwSize       = sizeof(DIPROPDWORD); 
//	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
//	dipdw.diph.dwHow        = DIPH_BYOFFSET; 	// pour utiliser un format de donné pour dwObj	 
//	dipdw.diph.dwObj        = DIJOFS_X;			// axe à configuré (X)
//	dipdw.dwData			= 100;				// valeur à partir de laquelle les entrée sont détectés
//
//	// affectation des propriétés à l'interface
//	DxJoystick->SetProperty( DIPROP_DEADZONE, &dipdw.diph );
//
//	dipdw.diph.dwObj        = DIJOFS_Y; // Specify the enumerated axis
//	// affectation des propriétés à l'interface
//	DxJoystick->SetProperty( DIPROP_DEADZONE, &dipdw.diph );

	return S_OK;
}

/*------------------------------------------------------------------------------*/
/*
void InputEngineDX::updateJoystick(DIJOYSTATE* p_joystick)
{
	if( DxJoystick )
	{
		DxJoystick->Poll();
		DxJoystick->GetDeviceState( sizeof(DIJOYSTATE), (LPVOID)p_joystick);
	}
}

/*------------------------------------------------------------------------------*/

/*-EOF-*/
