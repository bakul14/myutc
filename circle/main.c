#include <stdio.h>
#define BUF_SIZE 128 // 2^7
#define BUF_MASK (BUF_SIZE - 1)

int main()
{
    unsigned int circle[BUF_SIZE] = {0};
    unsigned int tail = 0, head = 0;

    for (int i = 0; i <= 100000; i++)
    {
        circle[head++] = i;
        head &= BUF_MASK;
        printf("%d\n\r", circle[tail++]);
        tail &= BUF_MASK;
    }

    return 0;
}