#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

unsigned char *read_bmp(char *fname, BITMAPINFOHEADER bi, unsigned char *img, int padding)
{
    int img_idx = 0
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to outfile
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

        // then add it back (to demonstrate how)
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }
}

void write_bmp(char *fname, int size, unsigned char *header, unsigned char *img)
{
    int img_idx = 0
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to outfile
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

        // then add it back (to demonstrate how)
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }
}

// perform simple binary threshold
void thresh(unsigned char *img, unsigned char *edited_img, size, thresh) 
{
    for (int i; i < size; i++) {
        for (int j; j < size; j++) {
            if (img[i][j] < thresh) {
                edited_img[i][j] = 0;
            }
            else {
                edited_img[i][j] = 255;
            }
        }
    }
}

int main(int argc, char* argv[])
{

    // remember filenames
    char* infile = argv[1];
    char* outfile = argv[2];
    // operation to perform
    int mode = argv[3];
    
    

    FILE *source = fopen(infile, "rb+");
    FILE *dest = fopen(outfile, "wb+");
    
    // read infile's BITMAPFILEHEADER and BITMAPINFOHEADER
    // output file and info headers will be the same
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // init image buffers
    unsigned char img[bi.biWidth * bi.biHeight * 3];
    unsigned char edited_img[bi.biWidth * bi.biHeight * 3];

    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // read infile into char array
    read_bmp(source, img);

    // TODO: add a case/switch here for different modes of operation
    // TODO: OMP declarations

    //  binary thresh for now
    thresh(img, edited_img, bi, 128);

    write_bmp("dest.bmp", size, header, img);

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    return 0;
}