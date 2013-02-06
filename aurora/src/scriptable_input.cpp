#include "scriptable_input.h"

#define HEADER scriptable_input
#define HELPER scriptable_object_cpp
#include "include_helper.h"

ScriptableInput::ScriptableInput(State* attachedState, VFS::File* sourceFile): Controllable(attachedState), SubScriptable(attachedState->script(), sourceFile)
{

}

int ScriptableInput::signal(ScriptableObject* other, SignalId signal, int data)
{
	return script()->callFunction(*this, (char*)"onSignal", "oii", other, signal, data).getint();
}

int ScriptableInput::onSignal(ScriptableObject *other, SignalId signalId, int parameter)
{
	return 0;
}

void ScriptableInput::processInput(ControllerState* state)
{
#define PROCESS_INPUT_KEY(a) do { \
	if(state->isButtonPressed(a)) \
		signal(this, KEY_PRESSED, (int)a); \
	if(state->isButtonDown(ControllerState::BTN_A)) \
		signal(this, KEY_DOWN, (int)ControllerState::BTN_A); } while(0)

	PROCESS_INPUT_KEY( ControllerState::BTN_Up );
	PROCESS_INPUT_KEY( ControllerState::BTN_Down );
	PROCESS_INPUT_KEY( ControllerState::BTN_Left );
	PROCESS_INPUT_KEY( ControllerState::BTN_Right );

	PROCESS_INPUT_KEY( ControllerState::BTN_A );
	PROCESS_INPUT_KEY( ControllerState::BTN_B );
	PROCESS_INPUT_KEY( ControllerState::BTN_X );
	PROCESS_INPUT_KEY( ControllerState::BTN_Y );
	PROCESS_INPUT_KEY( ControllerState::BTN_L );
	PROCESS_INPUT_KEY( ControllerState::BTN_R );

	PROCESS_INPUT_KEY( ControllerState::BTN_Start );
	PROCESS_INPUT_KEY( ControllerState::BTN_Select );
	PROCESS_INPUT_KEY( ControllerState::BTN_StateStep );
	PROCESS_INPUT_KEY( ControllerState::BTN_StatePlay );
	PROCESS_INPUT_KEY( ControllerState::BTN_IncreaseStateStep );
	PROCESS_INPUT_KEY( ControllerState::BTN_DecreaseStateStep );

	//TODO: mouse and joystick management

#undef PROCESS_INPUT_KEY
}
