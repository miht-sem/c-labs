#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// Base of our u_int1024_t
const int base = 1000 * 1000 * 1000;


// ----------------------- Creating our struct (start) ---------------------------
typedef struct
{
    u_int32_t el[35];
    int lenOfArray;
    int flagOfСarryover;
} u_int1024_t;
// ----------------------- Creating our struct (start) ---------------------------


// ----------------------- Our "vector" (start) ---------------------------
void init(u_int1024_t *arr)
{
    for (int i = 0; i < 35; i++)
        arr->el[i] = 0;
    arr->lenOfArray = 0;
    arr->flagOfСarryover = 0;
}

void push_back(u_int1024_t *arr, u_int32_t num)
{
    arr->el[arr->lenOfArray++] = num;
}

void pop(u_int1024_t *v)
{
    v->el[v->lenOfArray - 1] = 0;
    v->lenOfArray--;
}

int count(u_int1024_t *v)
{
    for (int h = 34; h >= 0; h--)
        if (v->el[h] != 0)
        {
            v->lenOfArray = h + 1;
            break;
    }
    return v->lenOfArray;
}
// ----------------------- Our "vector" (end) ---------------------------


// ----------------------- Functions of arithmetic (start) ---------------------------
u_int1024_t add_op(u_int1024_t x, u_int1024_t y)
{
    int flagOfСarryover = 0;
    for (int i = 0; i < 35; i++)
    {
        x.el[i] += flagOfСarryover + (i < count(&y) ? y.el[i] : 0);
        flagOfСarryover = x.el[i] >= base;
        if (flagOfСarryover && i == 34)
            {
                x.flagOfСarryover = 1;
                break;
            }
        if (flagOfСarryover)
            x.el[i] -= base;
    }
    return x;
}

u_int1024_t subtr_op(u_int1024_t x, u_int1024_t y)
{
    int flagOfСarryover = 0;
    for (size_t i = 0; i < 35; i++)
    {
        x.el[i] -= flagOfСarryover + (i < count(&y) ? y.el[i] : 0);
        flagOfСarryover = y.el[i] < 0;
        if (flagOfСarryover)
            y.el[i] += base;
    }
    return x;
}

u_int1024_t mult_op(u_int1024_t x, u_int1024_t y)
{
    u_int1024_t c;
    init(&c);
    for (size_t i = 0; i < 35; i++)
        for (int j = 0, flagOfСarryover = 0; i + j < 35; j++)
        {
            long long cur = c.el[i + j] + x.el[i] * 1ll * (j < count(&y) ? y.el[j] : 0) + flagOfСarryover;
            c.el[i + j] = (int)(cur % base);
            flagOfСarryover = (int)(cur / base);
            if (flagOfСarryover != 0 && i == 34)
            {
                c.flagOfСarryover = 1;
                break;
            }
        }
    int temp = count(&c);
    return c;
}
// ----------------------- Functions of arithmetic (end) ---------------------------


// ----------------------- Functions of input and output (start) ---------------------------
void printf_value(u_int1024_t x)
{
    int start = count(&x) - 1;
    if (x.flagOfСarryover == 1)
    {
        printf("Undefined Behavior\n");
        return;
    }
    else if ((int)x.el[start] > 0)
    {
        printf("%d", x.el[start]);
    }
    else if (x.el[start] == 0)
    {
        printf("0\n");
        return;
    }
    else
    {
        printf("Undefined Behavior\n");
        return;
    }
    for(int i = start - 1; i >= 0; --i)
        printf("%09d", x.el[i]);
    printf("\n");
}

void scanf_value(u_int1024_t *x)
{
    char *arr1;
    arr1 = (char *)(malloc(315 * sizeof(char)));
    scanf("%s", arr1);
    for (int j = 0; arr1[j]; j++)
        if (!isdigit(arr1[j]))
        {
            printf("Вввод может содержать только цифры!!!!");
            exit(1);
        }
    init(x);
    for (int i = (int)strlen(arr1); i > 0; i -= 9)
    {
        arr1[i] = 0;
        push_back(x, (u_int32_t)(atoi(i >= 9 ? arr1 + i - 9 : arr1)));
    }
}
// ----------------------- Functions of input and output (end) ---------------------------


// ----------------------- Function of generation from unsigned long int to u_int1024_t(start) ---------------------------
u_int1024_t from_uint(unsigned int x)
{
    u_int1024_t k;
    init(&k);
    k.el[0] = x % base;
    k.el[1] = x / base;
    int temp = count(&k);
    return k;
}
// ----------------------- Function of generation from unsigned long int to u_int1024_t(end) ---------------------------


int main()
{

    u_int1024_t num1;
    u_int1024_t num2;
    
    printf("\nВведите первое число: ");
    scanf_value(&num1);
    printf("Введите второе число: ");
    scanf_value(&num2);

    u_int1024_t num3 = add_op(num1, num2);    
    u_int1024_t num4 = subtr_op(num1, num2);
    u_int1024_t num5 = mult_op(num1, num2);

    printf("\nCложение : ");
    printf_value(num3);
    printf("Вычитание: ");
    printf_value(num4);
    printf("Умножение: ");
    printf_value(num5);
    printf("\n");


    return 0;
}
