#ifndef AUDIO_EFFECT_H
#define AUDIO_EFFECT_H

#include "point.h"

#define MAX_VOLUME	(10000)

/** AudioEffect stores information about effects to apply to sound objects.
 */
typedef struct {
	int fadeOutTime;
	int currentFadeOutTime;	// Off == (currentTime >= time)
	int fadeInTime;
	int currentFadeInTime;	// Off == (currentTime >= time)
} AudioEffect;

#endif /* AUDIO_EFFECT_H */
