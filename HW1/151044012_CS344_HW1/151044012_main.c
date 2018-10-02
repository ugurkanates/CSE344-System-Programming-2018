#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define MAX_PATH 255
#define FILELEN 398
#define MAX_FILE 15000

#define BLOCK_SIZE 4096
#define KILOBYTE 1024
#define IFD_BYTE 12
/*
 *  UGURKAN ATES 151044012 System Programming HW01
 *  opening-reading and extracting data from tiff file
 *  and printing out of screens with no allocation errors or warnings
 *  black pixels are 0
 *  white pixels are 1
 *  reference linked in below of code.
 *
 */
uint32_t littleIndianBigIndian(uint32_t num){
    // uint32_t num = 0x12345678; -> formatted like this
    uint32_t b0,b1,b2,b3,b4,b5,b6,b7;
    uint32_t res = 0;

    b0 = (num & 0xf) << 28;
    b1 = (num & 0xf0) << 24;
    b2 = (num & 0xf00) << 20;
    b3 = (num & 0xf000) << 16;
    b4 = (num & 0xf0000) << 12;
    b5 = (num & 0xf00000) << 8;
    b6 = (num & 0xf000000) << 4;
    b7 = (num & 0xf0000000) << 4;

    res = b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    return res;
}
size_t getFilesize(const char* filename) {
    struct stat st;
    if(stat(filename, &st) != 0) {
        return 0;
    }
    return (size_t) st.st_size;
}
//for System choose of Motorola or Intel
// not used in System.


int ascii_to_hex(char c)
{
    int num = (int) c;
    if(num < 58 && num > 47)
    {
        return num - 48;
    }
    if(num < 103 && num > 96)
    {
        return num - 87;
    }
    return num;
}
uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}
int charHexInt(char byte){
    int don = 0;
    if (byte >= '0' && byte <= '9') don = byte - '0';
    else if (byte >= 'a' && byte <='f') don = byte - 'a' + 10;
    else if (byte >= 'A' && byte <='F') don = byte - 'A' + 10;
}
int charHexToInt(char *hex){
    int willReturn = 0 ;
    int size = strlen(hex);
    int sayi[4];
    for(int i = 0 ; i < size ; i++)
        sayi[i] = charHexInt(hex[i]);


    willReturn = sayi[0]*pow(16,1)+sayi[1]*pow(16,0);
    willReturn += sayi[2]*pow(16,3)+sayi[3]*pow(16,2);
}
unsigned char * read_file(FILE * file) //Don't forget to free retval after use
{
    int size = 0;
    unsigned int val;
    int startpos = ftell(file);
    while (fscanf(file, "%x ", &val) == 1)
    {
        ++size;
    }
    unsigned char * retval = (unsigned char *) malloc(size);
    fseek(file, startpos, SEEK_SET); //if the file was not on the beginning when we started
    int pos = 0;
    while (fscanf(file, "%x ", &val) == 1)
    {
        retval[pos++] = (unsigned char) val;
    }
    return retval;
}

int main(int argc,char *argv[]) {
    char *buffer; // reading whole of .tiff file to buffer array with size
                 //dynamically allocated with size of file*(1byte)
    int row=0,column=0; // ImageWidth and ImageLength(100h-101h) for print

    char moder; // Intel mode or Motorola mode.
    if(argc != 2){
        printf("Usage ./tiffprocessor test.tiff\n");
        exit(-2);
    }
    const char *path = argv[1];
    mode_t mode = S_IRUSR | S_IRGRP ; //only giving access to read modes
    int flags = O_RDONLY ; //only open file with read only flag
    int fd = open(path,flags,mode);
    if(fd == -1){ //checking if file opened correctly
        perror("open errror");
        return 1;
    }
    size_t sizeOfFile = getFilesize(path); // size of File implemented with
                                           // Posix function calls error check included
    printf("File size -> %zu Bytes\n",sizeOfFile);
    buffer = malloc(sizeof(char)*sizeOfFile*2); //allocate memory
    int toplam = read(fd,buffer,sizeOfFile);
    //set mode Intel or Motorola
    if(buffer[0] =='I' && buffer[1] == 'I') {
        printf("Byte Order -> Intel\n");
        moder = 'I';
        //little endian -> intel
        // 67 45 23 01 = 0x100 0x101 0x102 0x103
    }
    else if(buffer[0] == 'M' && buffer[1] == 'M') {
        printf("Byte order -> Motorola\n");
        moder = 'M';
        //big endian -> motorola
        // 01 23 45 67  -> 0x100 0x101 0x102 0x103
    }
    else{
        printf("File is likely compressed or something unexpected happened.");
        exit(-1);
    }

    //finding offset to go IFD(image file directory which contains tag count&tags
    //these arrays for getting hex from char correctly
    // 6c00 is 2 byte char but i need 4 byte to use them -> doing in below code
    char offSet[5];
    char buyut1[3];
    char buyut2[3];
    char newSET[MAX_PATH];
    char newSET2[MAX_PATH];
    sprintf( newSET, "%02x", (unsigned char)buffer[4]);
    sprintf( newSET2, "%02x", (unsigned char)buffer[5]);


    if(moder == 'I') {
        if(buffer[4] < 10 && buffer[4] >= 0) {
            buyut1[0] = '0';
            buyut1[1] = buffer[4];
            sprintf(offSet, "%s", buyut1);
        }
        else{
            sprintf(offSet, "%x", buffer[4]);
        }
        if(buffer[5] < 10 && buffer[5] >= 0) {
            buyut2[0] = '0';
            buyut2[1] = buffer[5];
            sprintf(offSet,"%s""%c""%x",offSet,'0',buffer[5]);

        }
        else{
            sprintf(offSet,"%s%x",offSet,buffer[5]);
        }
        sprintf(offSet,"%s%s",newSET,newSET2);
    }
    else if(moder == 'M')
        strncpy(offSet,buffer+8,4);


    int off = charHexToInt(offSet);
    int tagNum = buffer[off];


    for(int x = 0 ; x<tagNum;x++){
            if (moder == 'I') {


                if (buffer[off+2 + x * 12] == 0 && buffer[off+3+x*12] == 1) {
                    //tag 101 ImageWidth 100 tag for Rows
                    row = buffer[off+2+ (x * 12)+8];
                }
                else if (buffer[off+2+(x * 12)] == 1 && buffer[off+3+x*12] == 1) {
                    // 100 tag COLUMNS ImageLength for Columns
                    column = buffer[off+2+ (x * 12) +8];

                }

        }

    }
    close(fd);
    printf("width:%d\n",row);
    printf("column:%d\n",column);
    //printing image according to tiff row column
    for(int k=0;k<row;k++){
        for(int m=0;m<column;m++){
            if(buffer[8+k*column+m] == -1)
                printf("1");
            else if(buffer[8+k*column+m] == 0)
                printf("0");
        }
        printf("\n");
    }
        free(buffer);
    //freeing buffer


    return 0;
}

/*typedef struct _TiffHeader

	WORD  Identifier;  /* Byte-order Identifier
    WORD  Version;     /* TIFF version number (always 2Ah)
    DWORD IFDOffset;   /* Offset of the first Image File Directory
*/
/*
 * The format of an Image File Directory is shown in the following structure:

typedef struct _TifIfd
{
	WORD    NumDirEntries;    /* Number of Tags in IFD
TIFTAG  TagList[];        /* Array of Tags
DWORD   NextIFDOffset;    /* Offset to next IFD
} TIFIFD;
 typedef struct _TifTag
{
	WORD   TagId;       /* The tag identifier
WORD   DataType;    /* The scalar type of the data items
DWORD  DataCount;   /* The number of items in the tag data
DWORD  DataOffset;  /* The byte offset to the data items

 reference:http://www.fileformat.info/format/tiff/egff.htm
} TIFTAG;

*/