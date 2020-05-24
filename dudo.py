d = 2 * 2

def s(arr):
    n, r = arr
    if r != 1:
        return 5 * n + (n//2) + r - 7
    if r == 1 and n <= d//2:
        return 11 * n - 6
    if r == 1 and n > d//2:
        return 5 * d + n - 1

p = [(n, r) for n in range(1, d+1) for r in range(1, 7)]
p.sort(key=s)
print(p)
