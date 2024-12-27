#! /bin/python

file = open("calc2", "r");
# file = open("log/heap3.log", "r");
sum = 0
heap = 0

for line in file:
    s = line.split()
    rng = [int(i, 16) for i in s[0].split("-")]
    sum += rng[1] - rng[0]

print(sum)

file.close()


