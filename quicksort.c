void quicksort(int *a, int n) {
    if (n < 2)
        return;
    int p = a[n / 2];
    int *l = a;
    int *r = a + n - 1;
    while (l <= r) {
        if (*l < p) {
            l++;
            continue;
        }
        if (*r > p) {
            r--;
            continue; // we need to check the condition (l <= r) every time we change the value of l or r
        }
        int t = *l;
        *l++ = *r;
        *r-- = t;
    }
    quicksort(a, r - a + 1);
    quicksort(l, a + n - l);
}

int main () {
    int i;
    int a[] = {4, 65, 2, -31, 0, 99, 2, 83, 782, 1};
    int n = sizeof a / sizeof a[0];
    quicksort(a, n);
    
    for (i = 0; i < n; i++){
        printf("%d ", a[i]);
    }
    return 0;
}