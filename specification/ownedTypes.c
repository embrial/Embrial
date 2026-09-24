#include <stdlib.h>
#include <stdint.h>

int main() {
    // var x @i32 = Heap.create(0);
    int32_t *x = malloc(sizeof(int32_t)); // allocate memory
    if (!x) return 1;                     // panic
    *x = 0;                               // x.* = i32.create(0);

    // var y @i32 = x;
    int32_t *y = x; // copy address
    x = NULL;       // invalidate x

    free(y); // automatically inserted free
    return 0;
}
