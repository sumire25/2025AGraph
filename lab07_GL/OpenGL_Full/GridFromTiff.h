#include <string>
#include <vector>
#include <tiffio.h>
#include <iostream>
#define INDEX(x, y, z, gridX, gridY) ((x) + (y) * (gridX) + (z) * (gridX) * (gridY))

class GridFromTiff {
public:
    void run(const std::string& filePath, std::vector<bool>& grid, int& gridX, int& gridY, int& gridZ) {
        int aux=0;
        grid.clear();
        TIFF* tiff = TIFFOpen(filePath.c_str(), "r");
        if (!tiff) {
            std::cerr << "Error: Could not open the TIFF file." << std::endl;
            return;
        }
        TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &gridX);
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &gridY);
        gridZ = 0;

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
                        aux++;
                    } else {
                        grid.push_back(false);
                    }
                }
            }
            _TIFFfree(raster);
            gridZ++;
        } while (TIFFReadDirectory(tiff));

        std::cout << "white pixels: "<<aux<<std::endl;
        TIFFClose(tiff);
    }
};