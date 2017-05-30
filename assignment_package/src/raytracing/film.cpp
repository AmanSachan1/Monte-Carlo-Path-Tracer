#include <raytracing/film.h>
#include <QImage>
#include <QColor>

Film::Film() : Film(400, 400){}

Film::Film(unsigned int width, unsigned int height)
    : bounds(Point2i(0.f, 0.f), Point2i(width, height))
{
    SetDimensions(width, height);
}

void Film::SetDimensions(unsigned int w, unsigned int h)
{
    bounds = Bounds2i(Point2i(0,0), Point2i(w, h));
    pixels.clear();
    pixels = std::vector<std::vector<Color3f>>(w);
    for(unsigned int i = 0; i < w; i++){
        pixels[i] = std::vector<Color3f>(h);
    }
    pixel_has_color = std::vector<std::vector<bool>>(w, std::vector<bool>(h, false));
}

void Film::SetPixelColor(const Point2i &pixel, const Color3f pixelColor)
{
    pixels[pixel.x][pixel.y] = pixelColor;
    pixel_has_color[pixel.x][pixel.y] = true;
}

Color3f Film::GetColor(const Point2i &pixel)
{
    return pixels[pixel.x][pixel.y];
}

void Film::WriteImage(QString path)
{
    if(QString::compare(path.right(4), QString(".png"), Qt::CaseInsensitive) != 0)
    {
        path.append(QString(".png"));
    }

    Vector2i dims = bounds.Diagonal();

    QImage output(dims.x, dims.y, QImage::Format_RGB32);

    for(unsigned int i = 0; i < dims.x; i++)
    {
        for(unsigned int j = 0; j < dims.y; j++)
        {
            Color3f color = (pixels[i][j]) * 255.f;
            output.setPixel(i, j, QColor(color.r, color.g, color.b).rgb());
        }
    }
    output.save(path);
}

bool Film::IsPixelColorSet(const Point2i &pixel)
{
    return pixel_has_color[pixel.x][pixel.y];
}

void Film::cleanPixels() {
    Vector2i diag = this->bounds.Diagonal();
    for(unsigned int i = 0; i < diag.x; ++i)
        pixels.at(i).clear();
    pixels.clear();
    SetDimensions(diag.x, diag.y);
}

