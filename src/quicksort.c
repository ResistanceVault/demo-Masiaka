#include "genesis.h"
#include "quicksort.h"

static struct QSORT_ENTRY t;

//------------------------------------------------------
inline void    QSwap (struct QSORT_ENTRY *a, struct QSORT_ENTRY *b)
//------------------------------------------------------
{   
    // struct QSORT_ENTRY t = *a;
    t = *a;
    *a = *b;
    *b = t;
}

//----------------------------------------
// http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
inline void    QuickSort (u16 n, struct QSORT_ENTRY *a)
//----------------------------------------
{
    short i, j, p;
    if (n < 2)
        return;
    p = a[n >> 1].value;
    for (i = 0, j = n - 1;; i++, j--) {
        while (a[i].value < p)
            i++;
        while (p < a[j].value)
            j--;
        if (i >= j)
            break;

        QSwap(&a[i], &a[j]);
    }
    QuickSort(i, a);
    QuickSort(n - i, a + i);
}  