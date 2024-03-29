#ifndef SHOVELER_IMAGE_H
#define SHOVELER_IMAGE_H

#include <shoveler/color.h>

typedef struct ShovelerImageStruct {
  unsigned int width;
  unsigned int height;
  unsigned int channels;
  unsigned char* data;
} ShovelerImage;

ShovelerImage* shovelerImageCreate(unsigned int width, unsigned int height, unsigned int channels);
ShovelerImage* shovelerImageCreateCopy(ShovelerImage* input);
ShovelerImage* shovelerImageCreateFlippedX(ShovelerImage* input);
ShovelerImage* shovelerImageCreateFlippedY(ShovelerImage* input);
ShovelerImage* shovelerImageCreateRotatedClockwise(ShovelerImage* input);
ShovelerImage* shovelerImageCreateRotatedCounterClockwise(ShovelerImage* input);
ShovelerImage* shovelerImageCreateAnimationTileset(ShovelerImage* input, int shiftAmount);
void shovelerImageClear(ShovelerImage* image);
void shovelerImageSet(ShovelerImage* image, ShovelerColor color, unsigned char alpha);
void shovelerImageAddFrame(ShovelerImage* image, unsigned int size, ShovelerColor color);
void shovelerImageAddSubImage(
    ShovelerImage* image, int xOffset, int yOffset, ShovelerImage* subImage);
void shovelerImageFree(ShovelerImage* image);

#define shovelerImageGet(image, x, y, c) \
  (image)->data[(y) * (image)->width * (image)->channels + (x) * (image)->channels + (c)]

#endif
