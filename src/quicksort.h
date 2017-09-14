#include "genesis.h"

// Sorting generic structure
struct  QSORT_ENTRY
{
    fix16   value;
    u16    index;
};

void    QSwap (struct QSORT_ENTRY *a, struct QSORT_ENTRY *b);
void    QuickSort (u16 n, struct QSORT_ENTRY *a);
