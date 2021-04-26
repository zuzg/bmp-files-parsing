#include "header.h"

///to wrzucamy zeby dzialalo w linuxie, roboczo do testow zostawilam chara

/*
int main(int argc, char **argv) {

    char * filename = strtok(argv[1], " ");
    char * fileout = strtok(NULL, " ");
    char * hidetext = strtok(NULL, " ");
*/
///dalam ify zeby w zaleznosci od inputu robilo rozne rzeczy, generalnie jak ogarniemy strtoka to bedzie git

int main() {
    //idk o co chodzi
    char argg[] = "tux.bmp output.bmp eluwina";
    //printf("%s\n", argg);

    char * filename = strtok(argg, " ");
    char * fileout = strtok(NULL, " ");
    char * hidetext = strtok(NULL, " ");

    printf("%s %s %s\n", filename, fileout, hidetext);

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

    if(fileout == NULL){
        printf("Decode steganography? [Y/N] ");
        int ch = getc(stdin);
        if (ch == 'Y' || ch == 'y')
            decode(filename);
    }

    else if(hidetext == NULL){
    	//printf("XD0");
        tograyscale(&ImgCol, fileout, &fh, &ih, rowlength, offset);
    }

    else{
    	//printf("XD1");
    	steganography(hidetext, fileout, &ImgCol, &fh, &ih, offset);
    }

    free(ImgCol.RED);
    free(ImgCol.GREEN);
    free(ImgCol.BLUE);
    fclose(fp);
    return 0;
}
