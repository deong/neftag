#include <stdio.h>

int main()
{
    int code;
    char str[1024];
    int i=0;
    
    do
    {
        scanf("%d", &code);
        str[i++] = code;
    } while(code != 0);

    printf("%s\n", str);
    return 0;
}
