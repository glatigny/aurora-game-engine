#ifndef SIGNALABLE_H
#define SIGNALABLE_H

#include "scriptable_object.h"
#include "context.h"

typedef enum SignalId {
#include "signal_table.aos"
} SignalId;

/** Signalable provides an interface to send signals to objects.
 * A signal is nothing more than a function call, but it provides a way to
 * trigger actions across apparently-incompatible objects.
 *
 * Signals can carry events, triggers, or even a more complex protocol.
 * The only limitation is the fact that a signal has only one integer
 * parameter.
 */
class Signalable: virtual public ScriptableObject {
public:
#define HEADER signalable
#define HELPER scriptable_object_h
#include "include_helper.h"
	virtual ~Signalable() {}

	/** Send a signal to this object.
	 * A signal carries the instigator of the signal, the signal number
	 * and an optional parameter.
	 *
	 * Signals are defined in signal_table.sss
	 *
	 * @param other the ScriptableObject which sent the signal.
	 * @param signalId the signal sent by "other". See signal_table.sss for the list of available signals.
	 * @param parameter the parameter of the signal.
	 * @return the return value of the signal.
	 */
	virtual int signal(ScriptableObject* other, SignalId signalId, int parameter) = 0;
};
#endif

