#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *file_main;
    for (int i = 0; i < argc; ++i) {
        if (!strcmp("--file", argv[i]))
            file_main = argv[i + 1];
        else if (!strcmp("--create", argv[i])) {
            int col_argc = argc - 4;
            FILE *arc = fopen(file_main, "wb");
            FILE *tmp;
            fprintf(arc, "%d\n", col_argc);
            unsigned long long size_of_file;
            for (int j = 4; j < argc; ++j) {
                tmp = fopen(argv[j], "rb");
                if (tmp == NULL)
                    continue;
                fseek(tmp, 0, SEEK_END);
                size_of_file = ftell(tmp);
                fseek(tmp, 0, SEEK_SET);
                fprintf(arc, "%s - %llu\n", argv[j], size_of_file);
                int c1 = fgetc(tmp);
                while (!feof(tmp))
                {
                    fputc(c1, arc);
                    c1 = fgetc(tmp);
                }
                fprintf(arc, "\n");
                fclose(tmp);
            }
            
        }
        else if (!strcmp("--extract", argv[i])){    
            FILE *file = fopen(file_main, "rb+");
            int col_argc;
            fscanf(file, "%d", &col_argc);
            unsigned long long start = 2;
            unsigned long long prev_start = 0;
            unsigned long long end = 0;
            int temp;
            fseek(file, 2, SEEK_SET);
            while ((temp = getc(file)) != EOF) {
                start++;
                if (temp == '\n')
                    break;
            }
            fseek(file, 2, SEEK_SET);
            char name_of_file[128] = {0};
            char rubbish[128] = {0};
            unsigned long long sizefl;
            FILE *out_file;
            for (int j = 0; j < col_argc; ++j) {
                fscanf(file, "%s%s%llu", name_of_file, rubbish, &sizefl);
                out_file = fopen(name_of_file, "wb");
                if (out_file == NULL)
                    break;
                fseek(file, start, SEEK_SET);
                start += sizefl;
                while ((sizefl--) > 0)
                    putc((temp = getc(file)), out_file);
                prev_start = start;
                fseek(file, start+1, SEEK_SET);
                while ((temp = getc(file)) != EOF) {
                    start++;
                    if (temp == '\n')
                        break;
                }
                start+=1;
                fseek(file, prev_start, SEEK_SET);
                fclose(out_file);
            }
        }
        else if (!strcmp("--list", argv[i])){
            FILE *file = fopen(file_main, "rb+");
            int col_argc;
            fscanf(file, "%d", &col_argc);
            printf("Всего файлов в архиве: %d\n", col_argc);
            unsigned long long start = 2;
            unsigned long long prev_start = 0;
            unsigned long long end = 0;
            int temp;
            fseek(file, 2, SEEK_SET);
            while ((temp = getc(file)) != EOF) {
                start++;
                if (temp == '\n')
                    break;
            }
            fseek(file, 2, SEEK_SET);
            char tmp[128] = {0};
            char rubbish[128] = {0};
            unsigned long long sizefl;
            FILE *out_file;
            for (int j = 0; j < col_argc; ++j) {
                fscanf(file, "%s%s%llu", tmp, rubbish, &sizefl);
                printf("%s %s %llu байт\n", tmp, rubbish, sizefl);
                fseek(file, start, SEEK_SET);
                start += sizefl;
                prev_start = start;
                fseek(file, start+1, SEEK_SET);
                while ((temp = getc(file)) != EOF) {
                    start++;
                    if (temp == '\n')
                        break;
                }
                start+=1;
                fseek(file, prev_start, SEEK_SET);
                fclose(out_file);
            }
            fclose(file);
        }
    }
}
