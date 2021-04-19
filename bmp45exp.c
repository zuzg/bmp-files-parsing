#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#pragma warning(disable:4996)

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, * LPBITMAPFILEHEADER, * PBITMAPFILEHEADER;

// https://docs.microsoft.com/pl-pl/previous-versions/dd183376(v=vs.85)
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

void readfile(BITMAPFILEHEADER* fh, FILE* fp) {
    fread(&fh->bfType, sizeof fh->bfType, 1, fp);
    fread(&fh->bfSize, sizeof fh->bfSize, 1, fp);
    fread(&fh->bfReserved1, sizeof fh->bfReserved1, 1, fp);
    fread(&fh->bfReserved2, sizeof fh->bfReserved2, 1, fp);
    fread(&fh->bfOffBits, sizeof fh->bfOffBits, 1, fp);
}

void readinfo(BITMAPINFOHEADER* ih, FILE* fp) {
    fread(&ih->biSize, sizeof ih->biSize, 1, fp);
    fread(&ih->biWidth, sizeof ih->biWidth, 1, fp);
    fread(&ih->biHeight, sizeof ih->biHeight, 1, fp);
    fread(&ih->biPlanes, sizeof ih->biPlanes, 1, fp);
    fread(&ih->biBitCount, sizeof ih->biBitCount, 1, fp);
    fread(&ih->biCompression, sizeof ih->biCompression, 1, fp);
    fread(&ih->biSizeImage, sizeof ih->biSizeImage, 1, fp);
    fread(&ih->biXPelsPerMeter, sizeof ih->biXPelsPerMeter, 1, fp);
    fread(&ih->biYPelsPerMeter, sizeof ih->biYPelsPerMeter, 1, fp);
    fread(&ih->biClrUsed, sizeof ih->biClrUsed, 1, fp);
    fread(&ih->biClrImportant, sizeof ih->biClrImportant, 1, fp);
}

void printhistogram(int* arr, float max) {
    for (int i = 0; i < 16; i++)
        printf("%u-%u: %.2f%%\n", i * 16, 16 * i + 15, arr[i] / max * 100);
}


void histogram(int* redB, int* greenB, int* blueB, FILE* fp, BITMAPFILEHEADER* fh, BITMAPINFOHEADER* ih, int rowlength) {
    if (ih->biCompression != 0 || ih->biBitCount != 24) {
        printf("\nhistogram calculation is unsupported");
        return;
    }

    int red[16] = { 0 };
    int green[16] = { 0 };
    int blue[16] = { 0 };

    uint8_t r;
    uint8_t g;
    uint8_t b;
    int k = 0;

    //
    for (int j = 0; j < ih->biHeight; j++) {
        for (int i = 0; i < rowlength; i += 3) {
            fread(&r, sizeof r, 1, fp);
            fread(&g, sizeof g, 1, fp);
            fread(&b, sizeof b, 1, fp);

            //printf("%u %u %u ", r, g, b);
            redB[k] = r;
            greenB[k] = g;
            blueB[k] = b;
            //printf("%d %d %d\n", r, g, b);
            //printf("%d %d %d\n", redB[k], greenB[k], blueB[k]);
            k++;

            red[r / 16]++;
            green[g / 16]++;
            blue[b / 16]++;
        }
    }
    printf("\nRED\n");
    printhistogram(red, ih->biHeight * ih->biWidth);
    printf("\nGREEN\n");
    printhistogram(green, ih->biHeight * ih->biWidth);
    printf("\nBLUE\n");
    printhistogram(blue, ih->biHeight * ih->biWidth);
    fclose(fp);

}


void tograyscale(int* redB, int* greenB, int* blueB, char* filename, BITMAPFILEHEADER* fh, BITMAPINFOHEADER* ih, int rowlength) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    fwrite(&fh->bfType, sizeof fh->bfType, 1, fp);
    fwrite(&fh->bfSize, sizeof fh->bfSize, 1, fp);
    fwrite(&fh->bfReserved1, sizeof fh->bfReserved1, 1, fp);
    fwrite(&fh->bfReserved2, sizeof fh->bfReserved2, 1, fp);
    fwrite(&fh->bfOffBits, sizeof fh->bfOffBits, 1, fp);

    fwrite(&ih->biSize, sizeof ih->biSize, 1, fp);
    fwrite(&ih->biWidth, sizeof ih->biWidth, 1, fp);
    fwrite(&ih->biHeight, sizeof ih->biHeight, 1, fp);
    fwrite(&ih->biPlanes, sizeof ih->biPlanes, 1, fp);
    fwrite(&ih->biBitCount, sizeof ih->biBitCount, 1, fp);
    fwrite(&ih->biCompression, sizeof ih->biCompression, 1, fp);
    fwrite(&ih->biSizeImage, sizeof ih->biSizeImage, 1, fp);
    fwrite(&ih->biXPelsPerMeter, sizeof ih->biXPelsPerMeter, 1, fp);
    fwrite(&ih->biYPelsPerMeter, sizeof ih->biYPelsPerMeter, 1, fp);
    fwrite(&ih->biClrUsed, sizeof ih->biClrUsed, 1, fp);
    fwrite(&ih->biClrImportant, sizeof ih->biClrImportant, 1, fp);


    uint8_t gray;
    int k = 0;

    for (int j = 0; j < ih->biHeight; j++) {
        for (int i = 0; i < ih->biWidth; i ++) {
            gray = (redB[k] + greenB[k] + blueB[k]) / 3;
            //gray = (uint8_t)temp;
            //printf("%d %d %d %d\n", gray, redB[i], greenB[i], blueB[i]);
            fwrite(&gray, sizeof gray, 1, fp);
            fwrite(&gray, sizeof gray, 1, fp);
            fwrite(&gray, sizeof gray, 1, fp);
            k++;
        }
        //fwrite(&gray, sizeof gray, 1, fp);
    }
    for (int j = 0; j < fh->bfOffBits; j++)
        fwrite(&gray, sizeof gray, 1, fp);


    fclose(fp);
}

int main() {

    FILE* fp = fopen("c.bmp", "rb");
    if (!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    BITMAPFILEHEADER fh;
    readfile(&fh, fp);

    BITMAPINFOHEADER ih;
    readinfo(&ih, fp);

    printf("BITMAPFILEHEADER\n");
    printf("bfType: 0x%X\nbfSize: %u\nbfReserved1: 0x%X\nbfReserved2: 0x%X\nbfOffBits: %u\n",
        fh.bfType, fh.bfSize, fh.bfReserved1, fh.bfReserved2, fh.bfOffBits);

    printf("\nBITMAPINFOHEADER\n");
    printf("biSize: %u\nbiWidth: %d\nbiHeight: %d\nbiPlanes: %u\nbiBitCount: %u\nbiCompression: %u\n",
        ih.biSize, ih.biWidth, ih.biHeight, ih.biPlanes, ih.biBitCount, ih.biCompression);
    printf("biSizeImage: %u\nbiXPelsPerMeter: %u\nbiYPelsPerMeter: %u\nbiClrUsed: %u\nbiClrImportant: %u\n",
        ih.biSizeImage, ih.biXPelsPerMeter, ih.biYPelsPerMeter, ih.biClrUsed, ih.biClrImportant);



    int rowlength = floor((ih.biBitCount * ih.biWidth + 31) / 32) * 4;
    //printf("%d", rowlength);

    int el = ih.biWidth * ih.biHeight;

    int* redB = (int*)calloc(el, sizeof(int));
    int* greenB = (int*)calloc(el, sizeof(int));
    int* blueB = (int*)calloc(el, sizeof(int));


    histogram(redB, greenB, blueB, fp, &fh, &ih, rowlength);

    tograyscale(redB, greenB, blueB, "cece.bmp", &fh, &ih, rowlength);

    for(int i = 0; i < el; i++)
        //printf("%d ", redB[i]);

    free(redB);
    free(greenB);
    free(blueB);
    fclose(fp);
    return 0;
}
