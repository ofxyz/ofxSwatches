#include "SwatchLibrary.h"
#include <cmath>

namespace ofxSwatches {

std::vector<int> SwatchLibrary::findSameGreyValue(float percent, float tolerance) const {
    std::vector<int> indices;
    for (int i = 0; i < (int)colors.size(); i++) {
        const float g = colors[i].getGreyValuePercent();
        if (std::abs(g - percent) <= tolerance) {
            indices.push_back(i);
        }
    }
    return indices;
}

void SwatchLibrary::reorderColor(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= (int)colors.size()) return;
    if (toIndex < 0 || toIndex >= (int)colors.size()) return;
    if (fromIndex == toIndex) return;
    SwatchColor item = colors[fromIndex];
    colors.erase(colors.begin() + fromIndex);
    colors.insert(colors.begin() + toIndex, item);
}

} // namespace ofxSwatches
