#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#include "bmp.h"

#define BILLION 1000000000
#define MAX_BRIGHTNESS 255
#define M_PI 3.14159265358979323846264338327

typedef unsigned char pixel_t;

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

void convolution(const pixel_t *in, pixel_t *out, const float *kernel,
                 const int nx, const int ny, const int kn,
                 const bool normalize)
{
    assert(kn % 2 == 1);
    assert(nx > kn && ny > kn);
    const int khalf = kn / 2;
    float min = FLT_MAX, max = -FLT_MAX;
 
    if (normalize)
        for (int m = khalf; m < nx - khalf; m++)
            for (int n = khalf; n < ny - khalf; n++) {
                float pixel = 0.0;
                size_t c = 0;
                for (int j = -khalf; j <= khalf; j++)
                    for (int i = -khalf; i <= khalf; i++) {
                        pixel += in[(n - j) * nx + m - i] * kernel[c];
                        c++;
                    }
                if (pixel < min)
                    min = pixel;
                if (pixel > max)
                    max = pixel;
                }
 
    for (int m = khalf; m < nx - khalf; m++)
        for (int n = khalf; n < ny - khalf; n++) {
            float pixel = 0.0;
            size_t c = 0;
            for (int j = -khalf; j <= khalf; j++)
                for (int i = -khalf; i <= khalf; i++) {
                    pixel += in[(n - j) * nx + m - i] * kernel[c];
                    c++;
                }
 
            if (normalize)
                pixel = MAX_BRIGHTNESS * (pixel - min) / (max - min);
            out[n * nx + m] = (pixel_t)pixel;
        }
}
 
/*
 * gaussianFilter:
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 * determine size of kernel (odd #)
 * 0.0 <= sigma < 0.5 : 3
 * 0.5 <= sigma < 1.0 : 5
 * 1.0 <= sigma < 1.5 : 7
 * 1.5 <= sigma < 2.0 : 9
 * 2.0 <= sigma < 2.5 : 11
 * 2.5 <= sigma < 3.0 : 13 ...
 * kernelSize = 2 * int(2*sigma) + 3;
 */
void gaussian_filter(const pixel_t *in, pixel_t *out,
                     const int nx, const int ny, const float sigma)
{
    const int n = 2 * (int)(2 * sigma) + 3;
    const float mean = (float)floor(n / 2.0);
    float kernel[n * n]; // variable length array
 
    fprintf(stderr, "gaussian_filter: kernel size %d, sigma=%g\n",
            n, sigma);
    size_t c = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            kernel[c] = exp(-0.5 * (pow((i - mean) / sigma, 2.0) +
                                    pow((j - mean) / sigma, 2.0)))
                        / (2 * M_PI * sigma * sigma);
            c++;
        }
 
    convolution(in, out, kernel, nx, ny, n, true);
}
 
/*
 * Links:
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 * http://www.tomgibara.com/computer-vision/CannyEdgeDetector.java
 * http://fourier.eng.hmc.edu/e161/lectures/canny/node1.html
 * http://www.songho.ca/dsp/cannyedge/cannyedge.html
 *
 * Note: T1 and T2 are lower and upper thresholds.
 */
pixel_t *canny_edge_detection(const pixel_t *in,
                              const bitmap_info_header_t *bmp_ih,
                              const int tmin, const int tmax,
                              const float sigma)
{
    const int nx = bmp_ih->width;
    const int ny = bmp_ih->height;
 
    pixel_t *G = calloc(nx * ny * sizeof(pixel_t), 1);
    pixel_t *after_Gx = calloc(nx * ny * sizeof(pixel_t), 1);
    pixel_t *after_Gy = calloc(nx * ny * sizeof(pixel_t), 1);
    pixel_t *nms = calloc(nx * ny * sizeof(pixel_t), 1);
    pixel_t *out = malloc(bmp_ih->bmp_bytesz * sizeof(pixel_t));
 
    if (G == NULL || after_Gx == NULL || after_Gy == NULL ||
        nms == NULL || out == NULL) {
        fprintf(stderr, "canny_edge_detection:"
                " Failed memory allocation(s).\n");
        exit(1);
    }
 
    gaussian_filter(in, out, nx, ny, sigma);
 
    const float Gx[] = {-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1};
 
    convolution(out, after_Gx, Gx, nx, ny, 3, false);
 
    const float Gy[] = { 1, 2, 1,
                         0, 0, 0,
                        -1,-2,-1};
 
    convolution(out, after_Gy, Gy, nx, ny, 3, false);
 
    for (int i = 1; i < nx - 1; i++)
        for (int j = 1; j < ny - 1; j++) {
            const int c = i + nx * j;
            // G[c] = abs(after_Gx[c]) + abs(after_Gy[c]);
            G[c] = (pixel_t)hypot(after_Gx[c], after_Gy[c]);
        }
 
    // Non-maximum suppression, straightforward implementation.
    for (int i = 1; i < nx - 1; i++)
        for (int j = 1; j < ny - 1; j++) {
            const int c = i + nx * j;
            const int nn = c - nx;
            const int ss = c + nx;
            const int ww = c + 1;
            const int ee = c - 1;
            const int nw = nn + 1;
            const int ne = nn - 1;
            const int sw = ss + 1;
            const int se = ss - 1;
 
            const float dir = (float)(fmod(atan2(after_Gy[c],
                                                 after_Gx[c]) + M_PI,
                                           M_PI) / M_PI) * 8;
 
            if (((dir <= 1 || dir > 7) && G[c] > G[ee] &&
                 G[c] > G[ww]) || // 0 deg
                ((dir > 1 && dir <= 3) && G[c] > G[nw] &&
                 G[c] > G[se]) || // 45 deg
                ((dir > 3 && dir <= 5) && G[c] > G[nn] &&
                 G[c] > G[ss]) || // 90 deg
                ((dir > 5 && dir <= 7) && G[c] > G[ne] &&
                 G[c] > G[sw]))   // 135 deg
                nms[c] = G[c];
            else
                nms[c] = 0;
        }
 
    // Reuse array
    // used as a stack. nx*ny/2 elements should be enough.
    int *edges = (int*) after_Gy;
    memset(out, 0, sizeof(pixel_t) * nx * ny);
    memset(edges, 0, sizeof(pixel_t) * nx * ny);
 
    // Tracing edges with hysteresis . Non-recursive implementation.
    size_t c = 1;
    for (int j = 1; j < ny - 1; j++)
        for (int i = 1; i < nx - 1; i++) {
            if (nms[c] >= tmax && out[c] == 0) { // trace edges
                out[c] = MAX_BRIGHTNESS;
                int nedges = 1;
                edges[0] = c;
 
                do {
                    nedges--;
                    const int t = edges[nedges];
 
                    int nbs[8]; // neighbours
                    nbs[0] = t - nx;     // nn
                    nbs[1] = t + nx;     // ss
                    nbs[2] = t + 1;      // ww
                    nbs[3] = t - 1;      // ee
                    nbs[4] = nbs[0] + 1; // nw
                    nbs[5] = nbs[0] - 1; // ne
                    nbs[6] = nbs[1] + 1; // sw
                    nbs[7] = nbs[1] - 1; // se
 
                    for (int k = 0; k < 8; k++)
                        if (nms[nbs[k]] >= tmin && out[nbs[k]] == 0) {
                            out[nbs[k]] = MAX_BRIGHTNESS;
                            edges[nedges] = nbs[k];
                            nedges++;
                        }
                } while (nedges > 0);
            }
            c++;
        }
 
    free(after_Gx);
    free(after_Gy);
    free(G);
    free(nms);
 
    return out;
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

	printf("biBitCount: %d, biWidth: %d\n", bi.biCompression, bi.biWidth);
    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

	// start timing
	clock_gettime(CLOCK_MONOTONIC, &start);
    
	// read infile into char array
    read_bmp(source, bi, img, padding);

    // TODO: add a case/switch here for different modes of operation

    //  binary thresh for now
    threshold(img, edited_img, bi, 128);

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
