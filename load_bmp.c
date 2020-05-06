#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bmp.h"

#define BILLION 1000000000

void read_bmp(FILE *fname, BITMAPINFOHEADER bi, unsigned char *img, int padding)
{
// 	fseek(fname, 56, SEEK_SET);
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
// 			if ( i==125 ) printf("%x, %x, %x\n", triple.rgbtBlue, triple.rgbtGreen, triple.rgbtRed);

            // put into char array (BGR order)
            img[img_idx] = .3*triple.rgbtBlue + .59*triple.rgbtGreen + .11*triple.rgbtRed;
// 			img[img_idx] = triple.rgbtBlue/3 + triple.rgbtGreen/3 + triple.rgbtRed/3;
            img_idx ++;

        }
        // skip over padding, if any
//         for (int k = 0; k < padding; k++)
//         {
//             printf("padding: %d\n", fgetc(fname));
//         }
        fseek(fname, padding, SEEK_CUR);
    }
}

void write_bmp(FILE *fname, BITMAPINFOHEADER bi, unsigned char *img, int padding)
{
    int img_idx = 0;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
		unsigned char gray;
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // write RGB triple to outfile
            // temporary storage
            RGBTRIPLE triple;
            unsigned char r, g, b;
            gray = img[img_idx];
// 			if ( i == 125 ) printf("%x\n", gray);
            r = gray;//.3;
            g = gray;//.59;
            b = gray;//.11; 
			triple.rgbtBlue = b; triple.rgbtGreen = g; triple.rgbtRed = r;
			fwrite(&triple, sizeof(RGBTRIPLE), 1, fname); 
            // fputc(r, fname);
            // fputc(g, fname);
            // fputc(b, fname);
            img_idx ++;
        }

        // then add it back (to demonstrate how)
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, fname);
        }
    }
//     printf("%d\n", img_idx);
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
	// timing
	struct timespec start, end;
	double time;

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
	
    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bi.biBitCount != 24 || bi.biCompression != 0)
    {
		printf("bfType: %x, bfOffBits: %d, biSize: %d, biBitCount: %d, biCompression: %d\n", bf.bfType, bf.bfOffBits, bi.biSize, bi.biBitCount, bi.biCompression);
        fclose(source);
        fclose(dest);
        fprintf(stderr, "Wrong input file format.\n");
        return 4;
    }
    
    // write outfile's BITMAPFILEHEADER and BITMAPINFOHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, dest);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, dest);
	fprintf(dest, "\r\n");

	// move to write at start of file
	fseek(source, bf.bfOffBits, SEEK_SET);
	fseek(dest, bf.bfOffBits, SEEK_SET);

	// init image buffers
    unsigned char img[bi.biWidth * bi.biHeight];
    unsigned char edited_img[bi.biWidth * bi.biHeight];

	printf("biHeight: %d, biWidth: %d\n", bi.biHeight, bi.biWidth);
    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

	// start timing
	clock_gettime(CLOCK_MONOTONIC, &start);
    
	// read infile into char array
    read_bmp(source, bi, img, padding);

    // TODO: add a case/switch here for different modes of operation
    // TODO: OMP declarations

    //  binary thresh for now
    threshold(img, edited_img, bi, 128);

//     write_bmp(dest, bi, img, padding);
    write_bmp(dest, bi, edited_img, padding);

	clock_gettime(CLOCK_MONOTONIC, &end);
	time = (double) (BILLION*(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec));
	time /= BILLION;
	printf("Elapsed time: %6.4f\n", time);

    // close infile
    fclose(source);

    // close outfile
    fclose(dest);

    return 0;
}
