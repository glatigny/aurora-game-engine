class ExecutionPointer { friend class ScriptedFunction;
private:
	ExecutionPointer* super;
	Context* tc; // Top context
	Context* ec; // Execution context
	bool oneInstr; // Only one instruction to execute
	bool oneInstrDone; // One instruction has been executed
	const char* scriptStart; // Used for serialization to compute real ptr position
	Context* scriptHolder; // Context holding currently running script
	const char* scriptHolderVariable; // Variable name of the currently running script
	const char* ptr; // Actual execution pointer
	char* instr; // Instruction currently being executed
	const char* instr_data[4]; // Data pointers used to remember execution status
public:
};

