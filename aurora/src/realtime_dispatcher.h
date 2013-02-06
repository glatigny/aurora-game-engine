#include <list>
class RealtimeDispatcher: public Realtime {
private:
	class DispatchEntry {
	public:
		int period;
		bool enabled;

		DispatchEntry(Realtime* controlledObject, int period, bool enabled); // Will ref object
		~DispatchEntry(); // Will unref object
		Realtime* object;
	};

	list<DispatchEntry*> entries;

public:
	virtual ~RealtimeDispatcher();
};

