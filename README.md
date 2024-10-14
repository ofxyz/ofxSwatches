ofxSwatches
===========

Classic, simple, swatches addon.

> This addon extends `ofColor` to support CMYK. Calculations are still done in the Raw RGB space (for now) but this addon will try and create clean plates for printing from OF including Spot Colors.

![preview](./preview.png)

## Baked Features

  -[ ] Save and load swatches to JSON format 
  -[ ] Convert between types (Spot, RGB, CMYK)
  -[ ] Delete/Update swatches
  -[ ] Check if colour already exist before adding to swatches
  -[ ] Add rich black control.

## How to use

    #import ofxSwatches.h

    ofxSwatches swatches; //Or push to vector for multiple swatches libraries
    swatches.setLibraryName("MyColorLib"); // Optional

    swatches.addCMYK({0,255,255,0}, "Red" /* Optional*/ );
    swatches.addRGB({255,0,0,255}, "Red" /* Optional*/ );
    swatches.addRGB({255,0,0}, "Red" /* Optional*/ );

> Note: Adding swatches with the same name will overwrite swatches with that name.

    // Returns glm::vec4
    swatches.getRGB("Red");  //{255,0,0,255}
    swatches.getCMYK("Red"); //{0,255,255,0}

    // Returns ofColor
    swatches.getColor("Red");


## Version History

 -[Version 0.1]() (14.10.24) Initial commit, hopefully not too many bugs.
