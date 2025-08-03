print("Hello world!")

sum = 0
n = 10000
for i in range(n * 10):
    for j in range(n):
        sum += i * j

print(sum)