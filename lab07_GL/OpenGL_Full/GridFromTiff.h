#include <string>
#include <vector>
#include <tiffio.h>
#include <iostream>
#define INDEX(x, y, z, gridX, gridY) ((x) + (y) * (gridX) + (z) * (gridX) * (gridY))

class GridFromTiff {
public:
    void run(const std::string& filePath, std::vector<bool>& grid, int& gridX, int& gridY, int& gridZ, std::vector<float>& objectCenter) {
        grid.clear();
        TIFF* tiff = TIFFOpen(filePath.c_str(), "r");
        if (!tiff) {
            std::cerr << "Error: Could not open the TIFF file." << std::endl;
            return;
        }
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &gridX);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &gridY);
        gridZ = 0;

        objectCenter = std::vector<float>(3, 0.0f);
        int count = 0;
        // Iterate through the images in the TIFF file
        do {
            uint32_t* raster = (uint32_t*)_TIFFmalloc(gridX * gridY * sizeof(uint32_t));
            if (!raster) {
                std::cerr << "Error: Could not allocate memory for raster." << std::endl;
                TIFFClose(tiff);
                return;
            }

            if (!TIFFReadRGBAImage(tiff, gridX, gridY, raster, 0)) {
                std::cerr << "Error: Could not read image from TIFF file." << std::endl;
                _TIFFfree(raster);
                TIFFClose(tiff);
                return;
            }
            for (int y = 0; y < gridY; y++) {
                for (int x = 0; x < gridX; x++) {
                    uint32_t pixel = raster[y * gridX + x];
                    // Assuming completely white pixels are true and black pixels are false
                    if (TIFFGetR(pixel)==255 && TIFFGetG(pixel)==255 && TIFFGetB(pixel)==255) {
                        grid.push_back(true);
                        objectCenter[0] += x;
                        objectCenter[1] += y;
                        objectCenter[2] += gridZ;
                        count++;
                    } else {
                        grid.push_back(false);
                    }
                }
            }
            _TIFFfree(raster);
            gridZ++;
        } while (TIFFReadDirectory(tiff));

        objectCenter[0] /= count;
        objectCenter[1] /= count;
        objectCenter[2] /= count;
        objectCenter[0] = (objectCenter[0] / gridX) * 2.0f - 1.0f;
        objectCenter[1] = (objectCenter[1] / gridY) * 2.0f - 1.0f;
        objectCenter[2] = (objectCenter[2] / gridZ) * 2.0f - 1.0f;
        int maxSize = std::max(std::max(gridX,gridY), gridZ);
        objectCenter[0] *= gridX * 1.0 / maxSize;
        objectCenter[1] *= gridY * 1.0 / maxSize;
        objectCenter[2] *= gridZ * 1.0 / maxSize;
        objectCenter[2] *= 1.5;// scaling factor for the z axis

        TIFFClose(tiff);
    }
};