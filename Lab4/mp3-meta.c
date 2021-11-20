#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct HeaderOfTag // header ID3v2.4 (10 байт)
{
    uint8_t idv3[3];       // ID3 (3 байта)
    uint8_t version[2];    // Версия tag (2 байта)
    uint8_t flags;         // Флаги (1 байт)
    uint8_t size[4];       // Размер tag в syncsafe int (4 байта)
} HeaderOfTag;

typedef struct FrameOfTag // frame (10 байт + unicode(2 байта) )
{
    uint8_t frameId[4];   // Frame ID например: TIT2, TYER и т. д.
    uint8_t size[4];      // Размер frame в syncsafe int (4 байта)
    uint8_t flags[2];     // Флаги
    uint8_t unicode;      // Unicode флаги
} FrameOfTag;

HeaderOfTag header;
FrameOfTag frame;

int syncsafeByteToInt(uint8_t size[4]) // syncsafe byte в int
{
    return (int)size[0] << 21 | (int)size[1] << 14 | (int)size[2] << 7 | (int)size[3] << 0; 
    // 1 бит у каждого syncsafe byte всегда 0, поэтому у первого syncsafe byte мы делаем
    // побитовый сдвиг влево на 21 (всего 28 бит), у второго на 14 и т. д.
}

void intToSyncsafeByte(int x, uint8_t *size, int isHeader) // int в syncsafe byte 
{
    int bits = 127;                         // 0111_1111
    size[0] = (x >> 21) & bits;
    size[1] = (x >> 14) & bits;
    size[2] = (x >> 7) & bits;
    size[3] = x & bits;
}

int Show(char *fileName, char *frameName, int set, int *oldFrameSize) // Показываем метаинформацию в консоль
{
    FILE *file;
    file = fopen(fileName, "rb"); // Открываемn файл на чтение в режиме чтения побайтово

    if (file == NULL)             // Проверяем существует ли файл
    {
        printf("Can't open file");
        exit(1);
    }

    fread(&header, 1, 10, file);                     // Читаем header из mp3 файла (10 байт)
    int tagSize = syncsafeByteToInt(header.size); // Получаем размер тега
    int writePos = 0;                                // Позиция откуда записываем/считываем
    while (fread(&frame, 1, 11, file))               // Считываем Frame из файла (11 байт)
    {
        if (frame.frameId[0] == 0 || ftell(file) >= tagSize) // Если первый символ в ID Frame == 0 или прочитали больше чем нужно
        {
            writePos = ftell(file) - 11; // Запомним, где остановились читать, чтобы новый Frame 
            break;                       // ( для Set(), если функция Show() вызвана внутри Set() ) 
                                         // В этом случае мы запишем новый Frame со своим значением и со своим ID
        }

        int sizeOfFrame = syncsafeByteToInt(frame.size);  // Получаем размер значения Frame в int ( длина строки )
        uint8_t *buffer = (uint8_t *)calloc(sizeOfFrame, 1); // Выделяем память под значение Frame
        fread(buffer, 1, sizeOfFrame - 1, file);             // Запоминаем значениие Frame
        buffer[sizeOfFrame - 1] = 0;                         // Обнуляем последний элемент

        if (frameName == NULL)             // Если функция вызвана из main() 
        {
            printf("%s: ", frame.frameId); // Выводим ID Frame
            if (frame.unicode)             // Если есть флаг unicode после основной части Frame
            {
                for (int i = 2; i < sizeOfFrame; ++i) 
                {                          // Выводим значение Frame
                    printf("%c", buffer[i]);
                }
                printf("\n");              
            }
            else
            {
                printf("%s\n", buffer);
            }
        }
        else if (!strcmp((char *)frame.frameId, frameName)) // Если ID Frame равен ID, который мы ищем,
        {                                                   // чтобы заменить значение или, чтобы вывести
            if (set == 0)                                   // Если функция вызвана не из Set(), а из main() (для get)
            {
                printf("%s: ", frame.frameId); // Выводим ID Frame
                if (frame.unicode)
                {
                    for (int i = 2; i < sizeOfFrame; ++i)
                    {                          // Выводим значение Frame
                        printf("%c", buffer[i]);
                    }
                    printf("\n");
                }
                else
                {
                    printf("%s\n", buffer);   // Выводим значение Frame
                }
            }
            else
            {
                *oldFrameSize = sizeOfFrame;                // Запоминаем размер прошлого Frame, чтобы не создавать новый 
                writePos = ftell(file) - 10 - sizeOfFrame;  // Позиция до которой будем читать файл
                break;
            }
        }
        free(buffer); // Очищаем память
    }
    fclose(file);
    return writePos; // Опционально возвращаем позицию ( для случая, если функция вызвана из Set() )
}

void Set(char *fileName, char *frameName, char *frameValue) // Функция для изменения ID существующих Frames
{
    FILE *readFile;
    readFile = fopen(fileName, "rb");        // Открываем файл mp3 на чтение в режиме чтения побайтово

    FILE *tempFile;
    tempFile = fopen("temporary.mp3", "wb"); // Открываем файл mp3 на запись в режиме записи побайтово

    if (readFile == NULL || tempFile == NULL)// Проверяем открылись ли файлы mp3
    {
        printf("Can't open file");
        exit(1);
    }

    int *oldFrameSize = calloc(1, sizeof(int));                 // Выделяем память на старый размер Frame
    int writePos = Show(fileName, frameName, 1, oldFrameSize);  // Получаем позицию до куда будем читать файл (в байтах)
    int diffInSize = strlen(frameValue) - (*oldFrameSize - 1);  // Получаем разницу между прошлым значением ID Frame и новым (в байтах)
    uint8_t frameSize[4];                                       // Размер нового значения ID Frame
    uint8_t *buffer = (uint8_t *)calloc(writePos - 10, 1);      // Выделяем память на запись всех данных до writePos 
                                                                // (writePos - 10, так как 10 байт мы выделим на header отдельно)
    
    fread(&header, 1, 10, readFile);            // Читаем header (10 байтов)
    fwrite(&header, 1, 10, tempFile);           // Записываем header в новый файл

    fread(buffer, 1, writePos - 10, readFile);  // Читаем все байты до Frame
    fwrite(buffer, 1, writePos - 10, tempFile); // Записываем все байты до Frame в файл

    memcpy(frame.frameId, frameName, 4);        // Копируем ID Frame из frameName в frame.frameId

    int tagSize = syncsafeByteToInt(header.size);  // Получаем размер tag
    tagSize += diffInSize;                            // Увеличиваем размер tag на разницу между старым и новым значении

    intToSyncsafeByte(tagSize, header.size, 1);              // Записываем в header.size новый размер tag (syncsafe byte)
    intToSyncsafeByte(strlen(frameValue) + 1, frameSize, 1); // Записываем в frameSize значение, которое мы хотим записать (syncsafe byte)

    memcpy(frame.size, frameSize, 4);                        // Записываем в frame.size значение из frameSize (syncsafe byte)

    fwrite(&frame, 1, 11, tempFile);                     // Записываем нужный Frame 
    fwrite(frameValue, 1, strlen(frameValue), tempFile); // Записываем значение нужного Frame

    free(buffer);                                        // Очищаем память
    
    fseek(readFile, 0, SEEK_END);                        // Устанавливаем указатель в файле на конец
    int readAfterSet = *oldFrameSize ? writePos + 10 + *oldFrameSize : writePos; 
    // Если oldFrameSize не пустой, то увеличиваем указатель на значение
    // Frame, который был до этого + сам Frame, иначе оставляем указатель там, где он был ( не нашли такой ID Frame )
    int readSize = ftell(readFile) - readAfterSet; // Получаем позицию с которой будем читать оставшуюся часть файла
    buffer = (uint8_t *)calloc(readSize, 1);       // Выделяем память под оставшуюся часть файла

    fseek(readFile, readAfterSet, SEEK_SET); // Ставим указатель в файле на место с которого будем читать
    fread(buffer, 1, readSize, readFile);    // Читаем оставшуюся часть файла
    fwrite(buffer, 1, readSize, tempFile);   // Записываем оставшуюся часть файла
    free(buffer);                            // Чистим память

    fclose(readFile);                   // Закрываем файл из которого читали
    fclose(tempFile);                   // Закрываем файл в которой записывали
    remove(fileName);                   // Удаляем файл mp3 из которого читали
    rename("temporary.mp3", fileName);  // Меняем название mp3 файла в который 
                                        // записывали на название файла из которого читали 
}

int main(int argc, char *argv[])
{
    char *fileName;             // Название mp3 файла, который будем обрабатывать 
    char *frameName;            // Название ID Frame, которое надо или изменить, или вывести
    char *value;                // На какое значение заменим значение frameName
    int isShow, isSet, isGet;   // Флаги проверки выполнения опций
    isShow = isSet = isGet = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strstr(argv[i], "--filepath"))  // Является ли "--filepath" подстрокой argv[i]
        {
            fileName = strpbrk(argv[i], "=") + 1; // Ищет первое вхождение "=" в строку и возвращает указатель на него.
        }
        else if (strstr(argv[i], "--get"))  // Является ли "--get" подстрокой argv[i]
        {
            isGet = 1;
            frameName = strpbrk(argv[i], "=") + 1; // Ищет первое вхождение "=" в строку и возвращает указатель на него.
            continue;
        }
        else if (strstr(argv[i], "--set"))  // Является ли "--set" подстрокой argv[i]
        {
            isSet = 1;
            frameName = strpbrk(argv[i], "=") + 1; // Ищет первое вхождение "=" в строку и возвращает указатель на него.
            continue;
        }
        else if (strstr(argv[i], "--value"))// Является ли "--value" подстрокой argv[i]
        {
            value = strpbrk(argv[i], "=") + 1; // Ищет первое вхождение "=" в строку и возвращает указатель на него.
            continue;
        }
        else if (!strcmp(argv[i], "--show"))// Равный ли строки "--show" и argv[i]
        {
            isShow = 1;
            continue;
        }
        else                                // Если ввели не то
        {
            printf("Incorrect input format. Pls don't try to breake my program, I will be sad :( ");
            return 1;
        }
    }

    if (isShow)
    {
        Show(fileName, NULL, 0, NULL);
    }
    if (isGet)
    {
        Show(fileName, frameName, 0, NULL);
    }
    if (isSet)
    {
        Set(fileName, frameName, value);
    }

    return 0;
}