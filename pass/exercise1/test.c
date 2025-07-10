#include <stdio.h>
int add (int a, int b) {
    return a+b;
}
int main(void) {
    int a = add(2,3);
    printf("%d", a);
    return 0;
}
