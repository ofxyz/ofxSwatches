#pragma once

#include "imgui.h"

namespace ofxSwatches {

enum class SwatchDropZone { Before, After };

struct SwatchIndexDropResult {
    bool accepted = false;
    int dragged = -1;
    int target = -1;
    SwatchDropZone zone = SwatchDropZone::Before;
};

SwatchIndexDropResult swatchIndexDragDrop(const char* payloadTag,
                                          int index,
                                          const char* previewLabel,
                                          float rowMinY,
                                          float rowMaxY);

} // namespace ofxSwatches
