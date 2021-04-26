#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
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

void print_all(BITMAPFILEHEADER fh, BITMAPINFOHEADER ih)
{
    printf("BITMAPFILEHEADER\n");
    printf("bfType: 0x%X\nbfSize: %u\nbfReserved1: 0x%X\nbfReserved2: 0x%X\nbfOffBits: %u\n",
        fh.bfType, fh.bfSize, fh.bfReserved1, fh.bfReserved2, fh.bfOffBits);

    printf("\nBITMAPINFOHEADER\n");
    printf("biSize: %u\nbiWidth: %d\nbiHeight: %d\nbiPlanes: %u\nbiBitCount: %u\nbiCompression: %u\n",
        ih.biSize, ih.biWidth, ih.biHeight, ih.biPlanes, ih.biBitCount, ih.biCompression);
    printf("biSizeImage: %u\nbiXPelsPerMeter: %u\nbiYPelsPerMeter: %u\nbiClrUsed: %u\nbiClrImportant: %u\n",
        ih.biSizeImage, ih.biXPelsPerMeter, ih.biYPelsPerMeter, ih.biClrUsed, ih.biClrImportant);
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

void tograyscale(int* redB, int* greenB, int* blueB, char* filename, BITMAPFILEHEADER* fh, BITMAPINFOHEADER* ih, int rowlength, uint8_t * offset) {
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
    fwrite(offset,sizeof(uint8_t),fh->bfOffBits-14-sizeof(BITMAPINFOHEADER), fp);


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
    }
    fclose(fp);
}

char * string_to_binary(char* line) {
    if(line == NULL) return NULL;
    size_t len = strlen(line);
    char * binary = malloc(len*8 + 1);
    char * result = malloc(len*8 + 1);

    binary[0] = '\0';
    for(size_t i = 0; i < len; i++) {
        char temp = line[i];
        for(int j = 7; j >= 0; j--){ //obczaic
            if(temp & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    //further string prep
    for (int i=0; i<len; i++)
    {
        for (int j=0; j<8; j++)
        {
            result[8*i+j]=binary[8*i+8-1-j];
        }
    }
    free(binary);
    return result; //string przygotowany do bezposredniego wprowadzenia (na odwrot)
}

char * dec_bin (int x)
{
    char * result = malloc(8 + 1); //+1 bc of '\0'
    int counter = 0;
    result[0] = '\0';
    while (x!=0)
    {
        if (x%2==0) strcat(result, "0");
        else strcat(result, "1");
        x/=2;
        counter++;
    }
    while (counter != 8)
    {
        strcat(result, "0");
        counter++;
    }
    return result;
}


void steganography_lsb(char * text, int * redB, int * greenB, int * blueB, char * filename, BITMAPFILEHEADER * fh, BITMAPINFOHEADER * ih, uint8_t * offset)
{
    int x = strlen(text);
    char * initial = dec_bin(x); //free!, fully prepared
    char * bin_text = string_to_binary(text); //free!, fully prepared
    int bin_len = strlen(bin_text);

    printf("<%s>\n", initial);
    printf("<%s>\n", bin_text);
    printf("<%d>\n", bin_len);

    FILE* fp = fopen("LSB_img.bmp", "wb");
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
    fwrite(offset,sizeof(uint8_t),fh->bfOffBits-14-sizeof(BITMAPINFOHEADER), fp);

    int k = 0;

    int init_pom = 0;
    int encoded = 0;

    bool digit_encoded = false;
    bool text_encoded = false;

    uint8_t temp;

    for (int j = 0; j < ih->biHeight; j++) {
        for (int i = 0; i < ih->biWidth; i ++) {
            if (!digit_encoded || !text_encoded){
                if(!digit_encoded) //encoding a digit
                {
                    //RED
                    temp = redB[k]+(initial[init_pom]-'0');
                    fwrite(&temp, sizeof (uint8_t), 1, fp);
                    init_pom++;

                    //GREEN
                    if (init_pom != 8)
                    {
                        temp = greenB[k]+(initial[init_pom]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        init_pom++;
                    }
                    else
                    {
                        temp = greenB[k]+(bin_text[encoded]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        encoded++;
                    }
                    //BLUE
                    if (init_pom != 8)
                    {
                        temp = greenB[k]+(initial[init_pom]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        init_pom++;
                    }
                    else
                    {
                        temp = blueB[k]+(bin_text[encoded]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        encoded++;
                    }

                    if (init_pom == 8) digit_encoded = true;
                }
                else{ //encoding a message
                    //RED
                    temp = redB[k]+(bin_text[encoded]-'0');
                    fwrite(&temp, sizeof (uint8_t), 1, fp);
                    encoded++;

                    //GREEN
                    if (encoded != bin_len)
                    {
                        temp = greenB[k]+(bin_text[encoded]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        encoded++;
                    }
                    else fwrite(&greenB[k], sizeof (uint8_t), 1, fp);

                    //BLUE
                    if (encoded != bin_len)
                    {
                        temp = blueB[k]+(bin_text[encoded]-'0');
                        fwrite(&temp, sizeof (uint8_t), 1, fp);
                        encoded++;
                    }
                    else fwrite(&blueB[k], sizeof (uint8_t), 1, fp);

                    if (encoded == bin_len) text_encoded = true;
                }
            }
            else{
                fwrite(&redB[k], sizeof (uint8_t), 1, fp);
                fwrite(&greenB[k], sizeof (uint8_t), 1, fp);
                fwrite(&blueB[k], sizeof (uint8_t), 1, fp);
            }
            k++;
        }
    }
    fclose(fp);
}
