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

typedef struct COLORS {
    int* RED;
    int* GREEN;
    int* BLUE;
} COLORS;

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

void histogram(COLORS * ImgCol, FILE* fp, BITMAPFILEHEADER* fh, BITMAPINFOHEADER* ih, int rowlength) {
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
        for (int i = 0; i < ih->biWidth; i ++) {
            fread(&r, sizeof r, 1, fp);
            fread(&g, sizeof g, 1, fp);
            fread(&b, sizeof b, 1, fp);

            //printf("%u %u %u ", r, g, b);
            ImgCol->RED[k] = r;
            ImgCol->GREEN[k] = g;
            ImgCol->BLUE[k] = b;
            //printf("%d %d %d\n", r, g, b);
            //printf("%d %d %d\n", redB[k], greenB[k], blueB[k]);
            k++;

            red[r / 16]++;
            green[g / 16]++;
            blue[b / 16]++;
        }
        if (rowlength > ih->biWidth*3) //PADDING!
        {
            uint8_t temp;
            for (int x=0; x < rowlength - ih->biWidth*3; x++)
                fread(&temp, sizeof temp, 1, fp);
        }
    }
    printf("\nRED\n");
    printhistogram(red, ih->biHeight * ih->biWidth);
    printf("\nGREEN\n");
    printhistogram(green, ih->biHeight * ih->biWidth);
    printf("\nBLUE\n");
    printhistogram(blue, ih->biHeight * ih->biWidth);
    //fclose(fp);

}

void tograyscale(COLORS * ImgCol, char* filename, BITMAPFILEHEADER* fh, BITMAPINFOHEADER* ih, int rowlength, uint8_t * offset) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("File opening failed");
        return ;
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
            gray = (ImgCol->RED[k] + ImgCol->GREEN[k] + ImgCol->BLUE[k]) / 3;
            //gray = (uint8_t)temp;
            //printf("%d %d %d %d\n", gray, redB[i], greenB[i], blueB[i]);
            fwrite(&gray, sizeof gray, 1, fp);
            fwrite(&gray, sizeof gray, 1, fp);
            fwrite(&gray, sizeof gray, 1, fp);
            k++;
        }
        if (rowlength > ih->biWidth*3) //PADDING!
        {
            uint8_t temp = 0;
            for (int x=0; x< rowlength - ih->biWidth*3; x++)
                fwrite(&temp, sizeof temp, 1, fp);
        }

    }
    fclose(fp);
}

char * string_to_binary(char* line) {
    if(line == NULL) return NULL;
    int len = strlen(line);
    char * binary = malloc(len*8 + 1);
    char * result = malloc(len*8 + 1);

    binary[0] = '\0';
    for(int i = 0; i < len; i++) {
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
    return result;
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

int bin_dec (char * s)
{
    int x = 1;
    int res = 0;
    int temp = strlen(s);
    for (int i = 0; i < temp; i++)
    {
        res += (s[i]-'0')*x;
        x *= 2;
    }
    return res;
}

uint8_t eval_value (uint8_t color_value, uint8_t temp)
{
    //if (color_value%2==0 && temp ==0); //nic nie robimy
    if (color_value%2==0 && temp == 1) color_value+=1;
    else if (color_value%2==1 && temp == 0) color_value-=1;
    //if (color_value%2==1 && temp == 1); //nic nie robimy
    return color_value;
}

void steganography(char * text, char * fileout, COLORS * ImgCol, BITMAPFILEHEADER * fh, BITMAPINFOHEADER * ih, int rowlength,  uint8_t * offset)
{
    int x = strlen(text);
    char * only_number = dec_bin(x); //freed, fully prepared
    char * only_text = string_to_binary(text); //freed, fully prepared

    char * bin_text = malloc(strlen(text)*8+8+1);
    bin_text[0]='\0';
    strcat(bin_text,only_number);
    strcat(bin_text,only_text);

    free(only_number);
    free(only_text);

    int bin_len = strlen(bin_text);
    //printf("string to encode:\n<%s>\n", bin_text);
    //printf("number of characters (+1 bo strlen musi tez byc encoded): %d\n", bin_len);

    FILE* fp = fopen(fileout, "wb");
    if (!fp) {
        perror("File opening failed");
        return ;
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
    int encoded = 0;
    bool text_encoded = false;

    uint8_t temp;

    for (int j = 0; j < ih->biHeight; j++) {
        for (int i = 0; i < ih->biWidth; i++) {
            if (!text_encoded){
                //RED
                temp = eval_value(ImgCol->RED[k], bin_text[encoded]-'0');
                fwrite(&temp, sizeof (uint8_t), 1, fp);
                encoded++;

                //GREEN
                if (encoded != bin_len)
                {
                    temp = eval_value(ImgCol->GREEN[k], bin_text[encoded]-'0');
                    fwrite(&temp, sizeof (uint8_t), 1, fp);
                    encoded++;
                }
                else fwrite(&(ImgCol->GREEN[k]), sizeof (uint8_t), 1, fp);

                //BLUE
                if (encoded != bin_len)
                {
                    temp = eval_value(ImgCol->BLUE[k], bin_text[encoded]-'0');
                    fwrite(&temp, sizeof (uint8_t), 1, fp);
                    encoded++;
                }
                else fwrite(&(ImgCol->BLUE[k]), sizeof (uint8_t), 1, fp);

                if (encoded == bin_len) text_encoded = true;
                }
            else{
                fwrite(&(ImgCol->RED[k]), sizeof (uint8_t), 1, fp);
                fwrite(&(ImgCol->GREEN[k]), sizeof (uint8_t), 1, fp);
                fwrite(&(ImgCol->BLUE[k]), sizeof (uint8_t), 1, fp);
            }
            k++;
        }
        if (rowlength > ih->biWidth*3) //PADDING!
        {
            uint8_t temp =0;
            for (int x=0; x< rowlength-ih->biWidth*3; x++)
                fwrite(&temp, sizeof temp, 1, fp);
        }

    }
    free(bin_text);
    fclose(fp);
}

void decode (char * filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("File opening failed");
        return ;
    }
    BITMAPFILEHEADER fh;
    readfile(&fh, fp);

    BITMAPINFOHEADER ih;
    readinfo(&ih, fp);

    int offset_size = fh.bfOffBits-14-sizeof(BITMAPINFOHEADER);
    uint8_t * offset = (uint8_t *)calloc(offset_size, sizeof(uint8_t)); //freed
    fread(offset,sizeof(uint8_t),offset_size,fp);
    free(offset);
    int rowlength = floor((ih.biBitCount * ih.biWidth + 31) / 32) * 4;

    uint8_t temp;

    char * bin_val = malloc(8+1); //freed
    bin_val[0]='\0';

    for (int x=0; x<8; x++) //reading how many chars will be to decode
    {
        fread(&temp, sizeof temp, 1, fp);
        if (temp%2==0) strcat(bin_val, "0");
        else strcat(bin_val, "1");
    }
    //printf("bin_val: %s\n", bin_val);
    int val = bin_dec(bin_val);
    //printf("val: %d\n", val);
    free(bin_val);

    char * text = malloc(val+1); //freed
    text[0]='\0';
    int counter = 8;

    for (int i=0; i<val; i++)
    {
        char * bin_ch = malloc(8+1); //freed
        bin_ch[0]='\0';
        for (int j=0; j<8; j++)
        {
            fread(&temp, sizeof temp, 1, fp);
            if (temp%2==0) strcat(bin_ch, "0");
            else strcat(bin_ch, "1");

            counter++; //PADDING!
            if (counter==ih.biWidth*3 && rowlength > ih.biWidth*3)
            {
                uint8_t pom;
                for (int x=0; x< rowlength - ih.biWidth*3; x++)
                    fread(&pom, sizeof pom, 1, fp);
                counter=0;
            }
        }
        //printf("%s\n", bin_ch);
        text[i] = (char)(bin_dec(bin_ch));
        free(bin_ch);
    }
    text[val]='\0';
    printf("DECODED TEXT: %s\n", text);
    free(text);
    fclose(fp);
}
