#include "header.h"

int main() {

    char * filename = "tux.bmp";

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

    BITMAPFILEHEADER fh;
    readfile(&fh, fp);

    BITMAPINFOHEADER ih;
    readinfo(&ih, fp);

    int offset_size = fh.bfOffBits-14-sizeof(BITMAPINFOHEADER);
    uint8_t * offset = (uint8_t *)calloc(offset_size, sizeof(uint8_t));
    fread(offset,sizeof(uint8_t),offset_size,fp);

    print_all(fh, ih);

    int rowlength = floor((ih.biBitCount * ih.biWidth + 31) / 32) * 4;
    int el = ih.biWidth * ih.biHeight;

    COLORS ImgCol;
    ImgCol.RED = (int*)calloc(el, sizeof(int));
    ImgCol.GREEN = (int*)calloc(el, sizeof(int));
    ImgCol.BLUE = (int*)calloc(el, sizeof(int));

    histogram(&ImgCol, fp, &fh, &ih, rowlength);
    tograyscale(&ImgCol, "test.bmp", &fh, &ih, rowlength, offset);

    char * text = "Milego poniedzialku ;pp";
    steganography(text, filename, &ImgCol, &fh, &ih, offset);
    printf("\nDecode steganography? [Y/N] ");
    int ch = getc(stdin);
    if (ch == 'Y' || ch == 'y')
       decode();


    free(ImgCol.RED);
    free(ImgCol.GREEN);
    free(ImgCol.BLUE);
    fclose(fp);
    return 0;
}
