#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char red;
    char green;
    char blue;

} RGB;

typedef struct
{
    int width;
    int height;

} SizeOfImage;

typedef struct
{
    SizeOfImage size;
    int **colorsOfPixels;
} Image;

int row;
int col;

int readBytesFromImage(FILE *file, int numberOfBytes)
{
    int bytes = 0;
    for (int byte = 0; byte < numberOfBytes; byte++)
    {
        bytes += getc(file) * (1 << (8 * byte));
    }
    return bytes;
}

void writeBytesToImage(FILE *file, int value, int bytesNumber)
{
    for (int byte = 0; byte < bytesNumber; byte++)
    {
        fputc(value >> (8 * byte), file);
    }
}

void skipNbytes(FILE *file, int bytesNumber)
{
    for (int byte = 0; byte < bytesNumber; byte++)
        getc(file);
}

Image arrFromImage(char *filePath)
{
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) printf("Sad :( can't open the file ");

    int fileType = readBytesFromImage(file, 2);
    int fileSize = readBytesFromImage(file, 4);
    skipNbytes(file, 8);

    SizeOfImage sizeOfImage;
    skipNbytes(file, 4);
    sizeOfImage.width = readBytesFromImage(file, 4);
    sizeOfImage.height = readBytesFromImage(file, 4);
    skipNbytes(file, 28);

    int **colorsOfPixels = (int **)malloc(sizeOfImage.width * sizeof(int *));
    for (int x = 0; x < sizeOfImage.width; x++)
    {
        colorsOfPixels[x] = (int *)malloc(sizeOfImage.height * sizeof(int));
    }
    Image image = {sizeOfImage, colorsOfPixels};

    for (int y = 0; y < image.size.height; y++)
    {
        for (int x = 0; x < image.size.width; x++)
        {
            RGB pixel = {getc(file), getc(file), getc(file)};
            image.colorsOfPixels[x][y] = (!pixel.red) && (!pixel.green) && (!pixel.blue);
        }
    }

    return image;
}

void arrToImage(char *filePath, Image image, int count)
{
    int fileSize = image.size.width * image.size.height * 3 + 54;
    char *path = (char *)malloc(sizeof(filePath) + 10);
    sprintf(path, "%s/%d.bmp", filePath, count); //форматируем путь в соответствии с папкой и count итерацией
    
    FILE *imageToWrite = fopen(path, "wb");
    if (imageToWrite == NULL)
    {
        printf("Failed to open file");
        exit(1);
    }

    fputc('B', imageToWrite);
    fputc('M', imageToWrite);
    writeBytesToImage(imageToWrite, fileSize, 4);
    writeBytesToImage(imageToWrite, 0, 4);
    writeBytesToImage(imageToWrite, 54, 4);
    writeBytesToImage(imageToWrite, 40, 4);
    writeBytesToImage(imageToWrite, image.size.width, 4);
    writeBytesToImage(imageToWrite, image.size.height, 4);
    writeBytesToImage(imageToWrite, 1, 2);
    writeBytesToImage(imageToWrite, 24, 2);
    writeBytesToImage(imageToWrite, 0, 8);
    writeBytesToImage(imageToWrite, 3780, 4);
    writeBytesToImage(imageToWrite, 3780, 4);
    writeBytesToImage(imageToWrite, 0, 8);

    for (int y = 0; y < image.size.height; y++)
    {
        for (int x = 0; x < image.size.width; x++)
        {
            RGB pixel = {!image.colorsOfPixels[x][y] * 255, !image.colorsOfPixels[x][y] * 255, !image.colorsOfPixels[x][y] * 255};
            putc(pixel.red, imageToWrite);
            putc(pixel.green, imageToWrite);
            putc(pixel.blue, imageToWrite);
        }
    }
}

int countLiveLeighbours(int **gameField, int curRow, int curCol)
{
    int count = 0;
    for (int i = curRow - 1; i <= curRow + 1; i++)
    {
        for (int j = curCol - 1; j <= curCol + 1; j++)
        {
            if ((i == curRow && j == curCol) || (i < 0 || j < 0) || (i >= row || j >= col))
            {
                continue;
            }
            if (gameField[i][j] == 1)
            {
                count++;
            }
        }
    }
    return count;
}

void nextGeneration(int **gameField, int **tempGameField, int dumpFreq)
{
    int neighbourLiveCell;
    for (int countOfGenerations = 0; countOfGenerations < dumpFreq; countOfGenerations++)
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                neighbourLiveCell = countLiveLeighbours(gameField, i, j);
                if (gameField[i][j] == 1 && (neighbourLiveCell == 2 || neighbourLiveCell == 3))
                {
                    tempGameField[i][j] = 1;
                }

                else if (gameField[i][j] == 0 && neighbourLiveCell == 3)
                {
                    tempGameField[i][j] = 1;
                }

                else
                {
                    tempGameField[i][j] = 0;
                }
            }
        }
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                gameField[i][j] = tempGameField[i][j];
            }
        }
    }
}

int main(int argc, char **argv)
{
    int maxIter = 1;
    int dumpFreq = 1;
    char *outputFolder = "frames";
    char *inputFilePath = "test1.bmp";

    if (argc % 2 == 0)
    {
        printf("%s", "sad :( invalid number of arguments");
        return 1;
    }
    for (int i = 1; i < argc - 1; i = i + 2)
    {
        if (strcmp(argv[i], "--input") == 0)
        {
            inputFilePath = argv[i + 1];
        }
        else if (strcmp(argv[i], "--output") == 0)
        {
            outputFolder = argv[i + 1];
        }
        else if (strcmp(argv[i], "--max_iter") == 0)
        {
            maxIter = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "--dump_freq") == 0)
        {
            dumpFreq = atoi(argv[i + 1]);
        }
    }

    Image image = arrFromImage(inputFilePath);

    row = image.size.width;
    col = image.size.height;

    int **gameField = (int **)malloc(row * sizeof(int *));
    int **tempGameField = (int **)malloc(row * sizeof(int *));

    for (int i = 0; i < row; i++)
    {
        gameField[i] = (int *)malloc(col * sizeof(int));
        tempGameField[i] = (int *)malloc(col * sizeof(int));
    }

    for (int j = col - 1; j >= 0; j--)
    {
        for (int i = 0; i < row; i++)
        {
            if (image.colorsOfPixels[i][j])
                gameField[i][j] = 1;
            else
                gameField[i][j] = 0;
        }
    }

    for (int i = 0; i < maxIter; i++)
    {
        nextGeneration(gameField, tempGameField, dumpFreq);

        for (int j = col - 1; j >= 0; j--)
        {
            for (int i = 0; i < row; i++)
            {
                image.colorsOfPixels[i][j] = gameField[i][j];
            }
        }
        arrToImage(outputFolder, image, i + 1);
    }
    return 0;
}
