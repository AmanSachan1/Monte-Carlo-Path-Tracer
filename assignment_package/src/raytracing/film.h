#pragma once
#include <globals.h>
#include <vector>

//This class stores the array of pixels for the image being rendered, that is later copied into an actual image file.

class Film
{
public:
    Film();
    Film(unsigned int width, unsigned int height);

    void SetDimensions(unsigned int w, unsigned int h);
    void SetPixelColor(const Point2i& pixel, const Color3f pixelColor);
    Color3f GetColor(const Point2i& pixel);
    void WriteImage(const std::string &path);
    void WriteImage(QString path);
    Bounds2i bounds;
    bool IsPixelColorSet(const Point2i& pixel);

    void cleanPixels();

private:
    std::vector<std::vector<Color3f>> pixels;//A 2D array of pixels in which we can store colors
    std::vector<std::vector<bool>> pixel_has_color;// A 2D array to indicate if there is something rendered on the pixel
};
