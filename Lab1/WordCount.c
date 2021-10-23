#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void print_number_of_lines()
{
    int letter, numberOfLines, countEntr, flagEntr;
    numberOfLines = 0;
    while (letter != EOF)
    {
        letter = getchar();
        if (letter == '\n')
            numberOfLines++;
    }
    printf("%d\n", (numberOfLines + 1));
}

void print_number_of_words()
{
    int numberOfWords, isWord, letter;

    numberOfWords = isWord = 0;
    letter = getchar();

    while (letter != EOF)
    {
        if (letter == ' ' || letter == '\n' || letter == '\t')
            isWord = 0;
        else if (isWord == 0)
        {
            isWord = 1;
            numberOfWords++;
        }
        letter = getchar();
    }
    printf("%d\n", numberOfWords);
}

int main(int argc, char *argv[])
{
    struct stat st;

    if (argc != 3)
    {
        printf("\nОшибка: Неверное число параметров!\n\n");
        printf("Правила вызова программы:\n");
        printf("%s <опция> <имя_файла>\n\n\n", argv[0]);
        return 1;
    }

    /* 
    FILE *g;
    g = fopen(argv[2], "r");
    fseek(g, 0, SEEK_END);
    long pos = ftell(g);
    printf("%ld", pos);
    if (pos == 0)
    {
        printf("Размер файла '%s': 0 байт.", argv[2]);
        return 1;
    }
    */

    stat(argv[2], &st);
    if (st.st_size == 0)
    {
        printf("Некорректный ввод файла");
        return 1;
    }

    freopen(argv[2], "r", stdin);

    if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--lines"))
        print_number_of_lines();
    else if (!strcmp(argv[1], "-c") || !strcmp(argv[1], "--bytes"))
        printf("Размер файла '%s': %lld байт.\n", argv[2], st.st_size);
    else if (!strcmp(argv[1], "-w") || !strcmp(argv[1], "--words"))
        print_number_of_words();
    else
    {
        printf("Неправильнный ввод опции %s", argv[1]);
        return 1;
    }

    return 0;
}
