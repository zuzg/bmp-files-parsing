# bmp-files-parsing
uni project to parse bitmaps

## what it does
The program works with basic 24-bit BMP files. It parses and prints values from bitmap's headers: ```BITMAPFILEHEADER``` and ```BITMAPINFOHEADER```, calculates histogram  (percents for every color in intervals, note: BMP files are BGR). 
### grayscale
Additionialy it is possible to convert any 24-bit bitmap to grayscale (using the simplest formula - computed for each pixel: gray=(red+green+blue)/3
### steganography
Use the least significant bit of the first 8 bytes to encode text length (0-255 characters long). Then use the least significant bit of the next bytes to encode bits of the characters. In this method, the image looks exactly the same for an unsuspecting user, but the message is hidden in the bits of pixels.
## input
1. ```$ ./program PATH-TO-BMP-FILE``` print headers and calculated histogram of picture, decode message
2. ```$ ./program PATH-TO-INPUT-BMP-FILE PATH-TO-OUTPUT-BMP-FILE``` print info (as in 1.) and parse bmp file to grayscale
3. ```$ ./program PATH-TO-INPUT-BMP PATH-TO-ENCODED-BMP "text to be hidden"``` print info (as in 1.) and encode given message up to 255 characters - steganography
