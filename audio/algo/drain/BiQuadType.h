/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AUDIO_ALGO_DRAIN_ALGO_BIQUAD_TYPE_H__
#define __AUDIO_ALGO_DRAIN_ALGO_BIQUAD_TYPE_H__

#include <memory>

namespace audio {
	namespace algo {
		namespace drain {
			enum biQuadType {
				biQuadType_none, //!< no filter (pass threw...)
				biQuadType_lowPass, //!< low pass filter
				biQuadType_highPass, //!< High pass filter
				biQuadType_bandPass, //!< band pass filter
				biQuadType_notch, //!< Notch Filter
				biQuadType_peak, //!< Peaking band EQ filter
				biQuadType_lowShelf, //!< Low shelf filter
				biQuadType_highShelf, //!< High shelf filter
			};
		}
	}
}

#endif
