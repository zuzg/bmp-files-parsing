#include "header.h"

int main() {

    char * filename = "X.bmp";

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

    int* redB = (int*)calloc(el, sizeof(int));
    int* greenB = (int*)calloc(el, sizeof(int));
    int* blueB = (int*)calloc(el, sizeof(int));



    histogram(redB, greenB, blueB, fp, &fh, &ih, rowlength);
    tograyscale(redB, greenB, blueB, "test.bmp", &fh, &ih, rowlength, offset);

    char * text = "Hello!";
    steganography_lsb(text, redB, greenB, blueB, filename, &fh, &ih, offset);


    free(redB);
    free(greenB);
    free(blueB);
    fclose(fp);
    return 0;
}
