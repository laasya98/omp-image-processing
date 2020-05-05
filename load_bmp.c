#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

void read_bmp(FILE *fname, BITMAPINFOHEADER bi, unsigned char *img, int padding)
{
    int img_idx = 0;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, fname);
            //printf("%x, %x, %x\n", triple.rgbtBlue, triple.rgbtGreen, triple.rgbtRed);

            // put into char array (BGR order)
            img[img_idx] = .3*triple.rgbtBlue + .59*triple.rgbtGreen + .11*triple.rgbtRed;
            img_idx ++;

        }
        // skip over padding, if any
        fseek(fname, padding, SEEK_CUR);
    }
}

void write_bmp(FILE *fname, BITMAPINFOHEADER bi, unsigned char *img, int padding)
{
    int img_idx = 0;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // write RGB triple to outfile
            unsigned char r, g, b, gray;
            gray = img[img_idx];
            r = gray/.3;
            g = gray/.59;
            b = gray/.11; 
            fputc(r, fname);
            fputc(g, fname);
            fputc(b, fname);
            img_idx ++;
        }

        // then add it back (to demonstrate how)
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, fname);
        }
    }
    printf("%d\n", img_idx);
}

// perform simple binary threshold
void threshold(unsigned char *img, unsigned char *edited_img, BITMAPINFOHEADER bi, int thresh) 
{
    for (int i = 0; i < (bi.biHeight * bi.biWidth); i++) {
        if (img[i] < thresh) {
            edited_img[i] = 0x00;
            //printf("%d, %d | ",  img[i], edited_img[i]);
        }
        else {
            edited_img[i] = 0xFF;
            //printf("%d, %d | ",  img[i], edited_img[i]);
        }

    }
}

int main(int argc, char* argv[])
{

    // remember filenames
    char* infile = argv[1];
    char* outfile = argv[2];
    // operation to perform
    int mode = atoi(argv[3]);
    
    FILE *source = fopen(infile, "rb+");
    FILE *dest = fopen(outfile, "wb+");
    
    // read infile's BITMAPFILEHEADER and BITMAPINFOHEADER
    // output file and info headers will be the same
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, source);
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, source);
    
    // write outfile's BITMAPFILEHEADER and BITMAPINFOHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, dest);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, dest);

    // init image buffers
    unsigned char img[bi.biWidth * bi.biHeight];
    unsigned char edited_img[bi.biWidth * bi.biHeight];

    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // read infile into char array
    read_bmp(source, bi, img, padding);

    // TODO: add a case/switch here for different modes of operation
    // TODO: OMP declarations

    //  binary thresh for now
    threshold(img, edited_img, bi, 128);

    write_bmp(dest, bi, edited_img, padding);

    // close infile
    fclose(source);

    // close outfile
    fclose(dest);

    return 0;
}