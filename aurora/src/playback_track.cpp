#include "playback_track.h"
#include "signalable.h"

PlaybackTrack::PlaybackTrack(int newDuration, int newPeriod, int newLoopType, int newStart, int newEnd, int newReverse,
                             int newSpeed) :
	speedErrorPropagation(0), period(newPeriod), duration(newDuration), reverse(newReverse), loopType(newLoopType),speed(newSpeed),
	currentPosition(newStart), finished(false)
{
	setPlaybackBoundaries(newStart, newEnd);
	assert(period);
}

PlaybackTrack::~PlaybackTrack() {
	if(!events.empty()) {
		for(EventMap::iterator mit = events.begin(); mit != events.end(); mit++) {
			for(EventList::iterator lit = mit->second.begin(); lit != mit->second.end(); lit++) {
				delete (*lit);
			}
		}
	}
}

void PlaybackTrack::addEvent(const char* position, Signalable* evt) {
	addEvent(atoi(position), evt);
}

void PlaybackTrack::addEvent(int position, Signalable* evt) {
	events[position].push_back(evt);
}

void PlaybackTrack::trigger(EventList& eventList) {
	for(EventList::iterator it = eventList.begin(); it != eventList.end(); it++) {
		(*it)->signal(NULL, PLAY, 0);
	}
}

void PlaybackTrack::updateObject(dur elapsedTime) {
	if(finished || !elapsedTime || !duration)
		return;

	// Manage targetStart and targetEnd
	if(targetStart != start || targetEnd != end) {
		if(insidePlaybackZone(currentPosition, targetStart, targetEnd)) {
			start = targetStart;
			end = targetEnd;
			if(start >= duration) {
				start = duration;
			}
			if(end > duration) {
				end = duration;
			}
		}
	}

	currentPosition = getFinalPosition(currentPosition, elapsedTime, reverse);
}

inline bool PlaybackTrack::insidePlaybackZone(int position, int start, int end) {
	if(!duration)
		return false;

	if(end <= start)
		end += duration;

	return position >= start && position < end;
}

// QC:G
int PlaybackTrack::getFinalPosition(int position, int delta, bool& reverseParam) {
	if(!duration)
		return 0;

	// Compute playback length and relative position
	int len = end;
	len -= start;
	if(len <= 0)
		len += duration;

	position -= start;
	if(position < 0)
		position += duration;

	int oldPosition = position;

	// Move the playback cursor
	position += reverseParam ? -delta : delta;

	// Apply loop
	switch(loopType){
	default:
	case PLAYBACK_ONCE: {
		if(position < 0) {
			position = 0;
			if(reverseParam) {
				finished = true;
			}
		} else if(position >= len) {
			position = len;
			if(!reverseParam) {
				finished = true;
			}
		}
	}
		break;

	case PLAYBACK_LOOP: {
		// Modulate by playbackLength in a mathematical way.
		position %= len;
		if(position < 0) {
			position += len;
		}
	}
		break;

	case PLAYBACK_PINGPONG: {
		// Modulate by playbackLength*2 in a mathematical way.
		position %= len * 2;
		if(position < 0) {
			position += len * 2;
		}

		if(position >= len) {
			// We are in the pong part of the ping-pong
			reverseParam = !reverseParam;
		}
	}
		break;
	}

	// Trigger events
	// TODO : handle PLAYBACK_PINGPONG.
	if(delta && !events.empty()) {
		for(EventMap::iterator it = events.begin(); it != events.end(); it++) {
			// Handle full loops
			while(loopType == PLAYBACK_LOOP && delta >= duration) {
				trigger(it->second);
				delta -= duration;
			}

			int evp = it->first;

			if(!reverseParam) {
				if(oldPosition < position) {
					if(evp >= oldPosition && evp < position) {
						trigger(it->second);
					}
				} else { // oldPosition >= position
					if(evp >= oldPosition || evp < position) {
						trigger(it->second);
					}
				}
			} else { // reverseParam
				if(oldPosition > position) {
					if(evp <= oldPosition && evp > position) {
						trigger(it->second);
					}
				} else { // oldPosition <= position
					if(evp <= oldPosition || evp > position) {
						trigger(it->second);
					}
				}
			}

		}
	}

	// Put the position back to absolute scale
	position += start;
	position %= duration;
	if(position < 0) {
			position += duration;
	}

	// Ensure that loop calculation is correct
	assert(position < duration);
	assert(position >= 0);

	return position;
}

// QC:?
void PlaybackTrack::setPlaybackBoundaries(int startPosition, int endPosition) {
	finished = false;

	if(!duration) {
		start = targetStart = startPosition;
		end = targetEnd = endPosition;
		return;
	}

	targetStart = startPosition;
	targetEnd = endPosition;

	if(!insidePlaybackZone(currentPosition, targetStart, targetEnd)) {
		// Wait to reach the boundaries before limiting playback
		if(reverse) {
			start = targetStart;
			end = currentPosition + 1;
		} else {
			start = currentPosition;
			end = targetEnd;
		}
	}

	xassert(start >= 0 && start < duration, "Animation start outside playback track.");
	xassert(end >= 0 && end < duration, "Animation end outside playback track.");
}

// QC:P
void PlaybackTrack::getSurroundingKeys(int& currentKey, int& nextKey, int& timeFromCurrentKey, int& timeToNextKey) {
	if(!duration) {
		currentKey = nextKey = timeFromCurrentKey = timeToNextKey = 0;
		return;
	}

	if(currentPosition % period == 0) {
		// The playback cursor is exactly on a key frame. Quickly return and avoid complicated calculations.
		currentKey = nextKey = currentPosition / period;
		timeFromCurrentKey = timeToNextKey = 0;
		return;
	}

	int linearCurrentPosition = currentPosition;
	if(end < start && linearCurrentPosition < start) {
		linearCurrentPosition += duration;
	}

	// Compute timeFromCurrentKey and timeToNextKey
	if(reverse) {
		timeToNextKey = -(linearCurrentPosition % period);
		if(timeToNextKey >= 0) {
			timeToNextKey -= period;
		}
		timeFromCurrentKey = -(timeToNextKey + period);
	} else {
		timeFromCurrentKey = linearCurrentPosition % period;
		if(timeFromCurrentKey < 0) {
			timeFromCurrentKey += period;
		}
		timeToNextKey = period - timeFromCurrentKey;
	}

	// Pre-compute currentKey and nextKey
	nextKey = linearCurrentPosition + timeToNextKey;
	currentKey = linearCurrentPosition - timeFromCurrentKey;

	currentKey = (currentKey % duration) / period;
	nextKey = (nextKey % duration) / period;
}

// QC:?
void PlaybackTrack::setPlaybackPosition(int newPosition) {
	if(!duration) {
		currentPosition = 0;
		return;
	}

	currentPosition = newPosition % duration;
	if(currentPosition < 0) {
		currentPosition += duration;
	}

	if(!insidePlaybackZone(currentPosition, targetStart, targetEnd)) {
		// Wait to reach the boundaries before limiting playback
		if(reverse) {
			end = currentPosition;
		} else {
			start = currentPosition;
		}
	}
}

void PlaybackTrack::setPlaybackSpeed(int newSpeed) {
	speed = newSpeed;
	speedErrorPropagation = 0;
}

