#include "SwatchGradient.h"

#include <algorithm>

namespace ofxSwatches {

void SwatchGradient::sortStops()
{
    std::sort(stops.begin(), stops.end(),
              [](const SwatchGradientStop& a, const SwatchGradientStop& b) {
                  return a.position < b.position;
              });
}

} // namespace ofxSwatches
