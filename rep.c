#include "header.h"

int main(int argc, char **argv) {

    if (argc == 1) return 0;

    char *filename = argv[1];

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

    if(argc == 2){
        printf("Decode steganography? [Y/N] ");
        int ch = getc(stdin);
        if (ch == 'Y' || ch == 'y')
            decode(filename);
    }

    else if(argc == 3){
    	//printf("CASE 2: tograyscale\n");
    	char *fileout = argv[2];
        tograyscale(&ImgCol, fileout, &fh, &ih, rowlength, offset);
    }

    else{
        //printf("CASE 3: steganography\n");
        char *fileout = argv[2];
        char *hidetext =  malloc(255);
        hidetext[0] = '\0';

        for (int i=3; i<argc; i++)
        {
            strcat(hidetext, argv[i]);
            if (i+1 != argc) strcat(hidetext, " ");
        }
    	//printf("TEXT TO HIDE: %s\n", hidetext);
    	steganography(hidetext, fileout, &ImgCol, &fh, &ih, rowlength, offset);
    	free(hidetext);
    }

    free(ImgCol.RED);
    free(ImgCol.GREEN);
    free(ImgCol.BLUE);
    free(offset);
    fclose(fp);
    return 0;
}
