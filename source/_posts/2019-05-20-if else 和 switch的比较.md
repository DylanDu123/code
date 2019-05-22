---
layout: post
title:  "if else 和 switch的比较"
subtitle: "学习笔记"
date:   2019-05-19 10:45:13 -0400
background: '/img/posts/03.jpg'
category: 编译原理
---

# 区别
if语句每执行一次都要先判断条件表达式是true还是false，为true时执行相应语句，若为false则继续判断下一个表达式，直到最后一个else结束。线性执行。 

switch语句的方式就至少有三种，编译器会根据代码的实际情况，权衡时间效率和空间效率，去选择一个对当前代码而言综合效率最高的一种。

# 编译器如何实现switch语句？
1. 逐条件判断
2. 跳转表
3. 二分查找法


### 1. 逐条件判断法
逐条件判断法其实就是和if……else语句的汇编实现相同，编译器把switch语句中各个case条件逐个进行判断，直到找到正确的case语句块。这种方法适用于switch语句中case条件很少的情况，即使逐个条件判断也不会导致大量时间和空间的浪费，比如下面这段代码：

```
#include <algorithm>

int test_switch(){
    int i ;
    int a = std::rand();
    switch(a){
        case 0: i = 0;break;
        case 1: i = 1;break;
        case 2: i = 2;break;
        default: i = 3;break;
    }
    return i;
}
```
该代码对应的汇编代码如下:

```
	movl	-4(%rbp), %eax
	cmpl	$1, %eax
	je	.L3
	cmpl	$2, %eax
	je	.L4
	testl	%eax, %eax
	jne	.L8
	movl	$0, -8(%rbp)
	jmp	.L6
.L3:
	movl	$1, -8(%rbp)
	jmp	.L6
.L4:
	movl	$2, -8(%rbp)
	jmp	.L6
.L8:
	movl	$3, -8(%rbp)
	nop
```

eax 寄存器存储的是判断条件值（对应于C++代码中的a值），首先判断 a 是否等于 1 ，如果等于 1 则跳转到 .L3 执行 a == 1 对应的代码段，然后判断 a 是否等于 2，如果等于 2 则跳转到 .L4 执行 a == 2 对应的代码段……可能难理解的是第 6 行代码testl %eax, %eax，其实这只是编译器提高判断一个寄存器是否为 0 效率的一个小技巧，如果 eax 不等于 0 则跳转到 .L8 代码段，执行 default 代码段对应的代码，如果 eax 等于 0 则执行 a==0 对应的代码段。

### 2. 跳转表实现法
在编译器采用这种 switch 语句实现方式的时候，会在程序中生成一个跳转表，跳转表存放各个 case 语句指令块的地址，程序运行时，首先判断 switch 条件的值，然后把该条件值作为跳转表的偏移量去找到对应 case 语句的指令地址，然后执行。这种方法适用于 case 条件较多，但是 case 的值比较连续的情况，使用这种方法可以提高时间效率且不会显著降低空间效率，比如下面这段代码编译器就会采用跳转表这种实现方式：

```
#include <algorithm>

int test_switch(){
    int i ;
    int a = std::rand();
    switch(a){
        case 0: i = 0;break;
        case 1: i = 1;break;
        case 2: i = 2;break;
        case 3: i = 3;break;
        case 4: i = 4;break;
        case 5: i = 5;break;
        case 6: i = 6;break;
        case 7: i = 7;break;
        case 8: i = 8;break;
        case 9: i = 9;break;
        default: i = 10;break;
    }
    return i;
}
```
该代码对应的汇编代码如下：

```
	movl	-4(%rbp), %eax
	movq	.L4(,%rax,8), %rax
	jmp	*%rax
.L4:
	.quad	.L3
	.quad	.L5
	.quad	.L6
	.quad	.L7
	.quad	.L8
	.quad	.L9
	.quad	.L10
	.quad	.L11
	.quad	.L12
	.quad	.L13
	.text
.L3:
	movl	$0, -8(%rbp)
	jmp	.L14
.L5:
	movl	$1, -8(%rbp)
	jmp	.L14
......
```
在x64架构中，eax寄存器是rax寄存器的低32位，此处我们可以认为两者值相等，代码第一行是把判断条件（对应于C++代码中的a值）复制到eax寄存器中，第二行代码是把.L4段偏移rax寄存器值大小的地址赋值给rax寄存器，第三行代码则是取出rax中存放的地址并且跳转到该地址处。我们可以清楚的看到.L4代码段就是编译器为switch语句生成的存放于.text段的跳转表，每种case均对应于跳转表中一个地址值，我们通过判断条件的值即可计算出来其对应代码段地址存放的地址相对于.L4的偏移，从而实现高效的跳转。


#### 3. 二分查找法
如果case值较多且分布极其离散的话，如果采用逐条件判断的话，时间效率会很低，如果采用跳转表方法的话，跳转表占用的空间就会很大，前两种方法均会导致程序效率低。在这种情况下，编译器就会采用二分查找法实现switch语句，程序编译时，编译器先将所有case值排序后按照二分查找顺序写入汇编代码，在程序执行时则采二分查找的方法在各个case值中查找条件值，如果查找到则执行对应的case语句，如果最终没有查找到则执行default语句。对于如下C++代码编译器就会采用这种二分查找法实现switch语句：

```
#include <algorithm>

int test_switch(){
    int i ;
    int a = std::rand();
    switch(a){
        case 4: i = 4;break;
        case 10: i = 10;break;
        case 50: i = 50;break;
        case 100: i = 100;break;
        case 200: i = 200;break;
        case 500: i = 500;break;
        default: i = 0;break;
    }
    return i;
}
```
改代码段对应的汇编代码为：
```
	movl	-4(%rbp), %eax
	cmpl	$50, %eax
	je	.L3
	cmpl	$50, %eax
	jg	.L4
	cmpl	$4, %eax
	je	.L5
	cmpl	$10, %eax
	je	.L6
	jmp	.L2
.L4:
	cmpl	$200, %eax
	je	.L7
	cmpl	$500, %eax
	je	.L8
	cmpl	$100, %eax
	je	.L9
	jmp	.L2
```
代码第二行条件值首先与50比较，为什么是50而不是放在最前面的4？这是因为二分查找首先查找的是处于中间的值，所以这里先与50进行比较，如果eax等于50，则执行case 50对应代码，如果eax值大于50则跳转到.L4代码段，如果eax小于50则继续跟4比较……直至找到条件值或者查找完毕条件值不存在。可以看出二分查找法在保持了较高的查询效率的同时又节省了空间占用。

# 总结
何时应该使用if……else语句，何时应该使用switch……case语句？

通过上面的分析我们可以得出结论，在可能条件比较少的时候使用if……else和switch……case所对应的汇编代码是相同的，所以两者在性能上是没有区别的，使用哪一种取决于个人习惯。如果条件较多的话，显而易见switch……case的效率更高，无论是跳转表还是二分查找都比if……else的顺序查找效率更高，所以在这种情况下尽量选用switch语句来实现分支语句。当然如果我们知道哪种条件出现的概率最高，我们可以将这个条件放在if判断的第一个，使顺序查找提前结束，这时使用if……else语句也可以达到较高的运行效率。

switch语句也有他本身的局限性，即switch语句的值只能为整型，比如当我们需要对一个double型数据进行判断时，便无法使用switch语句，这时只能使用if……else语句来实现。




```
引用
http://irootlee.com/juicer_switch/
```