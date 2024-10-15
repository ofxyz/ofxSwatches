ofxSwatches
===========

Classic, simple, swatches addon.

> This addon extends `ofColor` to support CMYK. Calculations are still done in the Raw RGB space (for now) but this addon will try and create clean plates for printing from OF including Spot Colors.

![preview](./preview.png)

## Baked Features

  -[x] Spot colour flag  
  -[x] Save and load swatches to JSON format  
  -[ ] Rich black control  

## How to use

```
    #import ofxSwatches.h

    ofxSwatches Swatches_Library;               // Or push to vector

    swatches.addCMYK({0,255,255,0}, "MY 100");  // Name Optional, setName("MY 100")

    swatches.getRGB("MY 100");                  // {255,0,0,255}
    swatches.getCMYK("MY 100");                 // {0,255,255,0}

    // Pass into OF function
    ofSetColor(swatches.get("MY 100"));         // Used as operator always returns RGBA

```

## Version History

 -[Version 0.2]() (15.10.24) Usable now, but not yet how it should be  
 
 -[Version 0.1]() (14.10.24) Initial commit, hopefully not too many bugs  
