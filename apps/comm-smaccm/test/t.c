/*
 * Test type-aliased array type pointer dereference behavior.
 * 
 * Output:
 * 0x3A2ABAC0, 0x3A2ABAC0
 * 233
 *
 * Conclusion:
 *     struct has the same address as its component array;
 *     Use struct to wrap constant-sized array, and use pointers when passed as arguments.
*/

#include <stdio.h>

typedef int array[10];
typedef struct array_cont {
        array arr;
} array_cont;

void f(array_cont *a) {
        printf("%d\n", a->arr[0]);
}

int main() {
        array_cont a1;
        a1.arr[0] = 233;

        printf("%p, %p\n", &a1, a1.arr);

        array_cont *r2 = &a1;

        f(r2);

        return 0;
}
