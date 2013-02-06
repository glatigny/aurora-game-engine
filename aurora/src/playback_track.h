#ifndef PLAYBACK_TRACK_H
#define PLAYBACK_TRACK_H

#define PLAYBACK_ONCE 0
#define PLAYBACK_LOOP 1
#define PLAYBACK_PINGPONG 2

class Signalable;

#include "realtime.h"
#include <list>
#include <map>

class PlaybackTrack: virtual public Realtime {
private:
	typedef std::list<Signalable*> EventList;
	typedef std::map<int, EventList> EventMap;
	EventMap events;

	int speedErrorPropagation;

	int getFinalPosition(int position, int delta, bool& reverseParam);
	bool insidePlaybackZone(int position, int start, int stop);

	void trigger(EventList& eventList);

protected:
	int period; // Period of key points
	int start; // Starting point of the sub part
	int end; // End point of the sub-part
	int targetStart; // Target starting point
	int targetEnd; // Target end point
	int duration; // Duration of the section
	bool reverse; // Playing reverse
	int loopType; // PLAYBACK_*
	int speed; // Speed in %

	int currentPosition;
	bool finished;

public:
	PlaybackTrack() : period(1), start(0), end(0), duration(0), reverse(false), loopType(PLAYBACK_ONCE), finished(true) {}
	PlaybackTrack(int newDuration, int newPeriod, int newLoopType = 0, int newStart = 0, int newEnd = 0, int newReverse = false,
	              int newSpeed = 100);
	virtual ~PlaybackTrack();

	/** Add an event to the playback track.
	 */
	virtual void addEvent(int position, Signalable* evt);
	virtual void addEvent(const char* position, Signalable* evt);

	void rewind() {
		reverse = !reverse;
		finished = false;
	}

	void setReverse(bool newReverse) {
		reverse = newReverse;
		finished = false;
	}

	/** Returns true if the playback is reversed.
	 * @return true if playback is reversed, false otherwise.
	 */
	bool isReversed() {
		return reverse;
	}

	/** Returns the current position of the playback cursor.
	 * The valid values are always within the boundaries.
	 * @return playback cursor position.
	 */
	int getPlaybackPosition() {
		return currentPosition;
	}

	/** Returns key frames surrounding the playback cursor.
	 * @param currentKey output parameter that indicates the key frame that is immediately before the playback cursor.
	 * @param nextKey output parameter that indicates the key frame that is immediately after the playback cursor.
	 * @param timeFromCurrentKey output parameter that tells the distance between currentKey and the current position. When playing reverse, this value is negative.
	 * @param timeToNextKey output parameter that tells the distance between nextKey and the current position. When playing reverse, this value is negative.
	 */
	void getSurroundingKeys(int& currentKey, int& nextKey, int& timeFromCurrentKey, int& timeToNextKey);

	/** Returns the position of a key frame.
	 * @param keyFrame the index of the key frame.
	 * @return the position of the key frame on the track.
	 */
	int getKeyFramePosition(int keyFrame);

	/** Tells whether the playback cursor is in a valid position.
	 * @return true if the cursor is between start and end boundaries, false otherwise.
	 */
	bool isPlaybackPositionValid();

	/** Sets playback boundaries.
	 * Boundaries allow to limit playback to a subset of the entire track.
	 * If the playback cursor is not inside the specified boundaries,
	 * the cursor will wait to be in the valid section before setting final boundaries.
	 * @param startPosition the starting point of the section to play.
	 * @param endPosition the ending point of the section to play.
	 */
	void setPlaybackBoundaries(int startPosition, int endPosition);
	void setDefaultBoundaries() {
		setPlaybackBoundaries(0, duration);
	}
	void setPlaybackPosition(int newPosition);
	void setPlaybackSpeed(int newSpeed);

	void setLoopType(int newLoopType) {
		assert(newLoopType >= 0 && newLoopType <= 2);
		loopType = newLoopType;
		finished = false;
	}

	/* Blocking */

	bool isFinished() {
		return finished;
	}

	/* Realtime */

	void updateObject(dur elapsedTime);

	/* Restartable */

	void restart() {
		finished = false;
		setPlaybackPosition(reverse ? targetEnd : targetStart);
	}
};

#endif /* PLAYBACK_TRACK_H */
