import random

# 对于一个正整数 n ，小于且与 n 互素的正整数的个数， 记为 φ(n)
# 对于一个素数 可知φ(n)=n-1
# 对于两个个素数 p 和 q ，他们的乘积满足 n=p*q 可知 φ(n)=(p-1)*(q-1)
if __name__ == "__main__":
    num = random.randint(0, 999999)
    num = 10
    if num <= 1:
        print('啥都没有');
    else :
        for item in range(2,num):
            if num%item != 0:
                print(item)
                pass
    pass

