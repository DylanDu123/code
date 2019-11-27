---
layout: post
title: 2019-11-17-ReactiveObjC冷热信号
subtitle: "RunLoop学习笔记"
date: 2019-11-17 17:20:20
category:
- 移动端
tags: 
- iOS
---

### 冷信号和热信号的概念
冷热信号的概念是源自于源于.NET框架中的 Hot Observable 和 Cold Observable

* `Hot Observable` 是主动的，尽管你并没有订阅事件，但是它会时刻推送。而 `Cold Observable` 是被动的，只有当你订阅的时候，它才会发布消息。
* `Hot Observable` 可以有多个订阅者，是一对多，集合可以与订阅者共享信息；而 `Cold Observable` 只能一对一，当有不同的订阅者，消息是重新完整发送。

### RAC冷热信号的体现
RAC 中冷热信号的理解可以参考 Github 上 [Framework概述](https://github.com/ReactiveCocoa/ReactiveObjC/blob/master/Documentation/FrameworkOverview.md) 文档 

其中关键性描述是在`Connections`下

>Signals are cold by default, meaning that they start doing work each time a new subscription is added. This behavior is usually desirable, because it means that data will be freshly recalculated for each subscriber, but it can be problematic if the signal has side effects or the work is expensive (for example, sending a network request).
>A connection is created through the -publish or -multicast: methods on RACSignal, and ensures that only one underlying subscription is created, no matter how many times the connection is subscribed to. Once connected, the connection's signal is said to be hot, and the underlying subscription will remain active until all subscriptions to the connection are disposed.

这段英语的翻译基本意思是

>默认情况下，信号是冷的，这意味着每次添加新订阅时它们都开始工作。它将为每个订阅者重新计算数据，但是如果信号有副作用或工作成本很高(例如，发送网络请求)，则可能会出现问题。
>通过RACSignal上的-publish或-multicast:方法创建连接，并确保只创建一个底层订阅，不管订阅了多少次连接。连接后，连接的信号被认为是热的，基础订阅将保持活动状态，直到对连接的所有订阅都被释放。

现在通过代码来展示下冷热信号的互相转换。
```
RACSignal *signal = [RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
    [subscriber sendNext:@1];
    [subscriber sendNext:@2];
    [subscriber sendNext:@3];
    [subscriber sendCompleted];
    return nil;
}];
NSLog(@"Signal was created.");
[[RACScheduler mainThreadScheduler] afterDelay:0.1 schedule:^{
    [signal subscribeNext:^(id x) {
        NSLog(@"Subscriber 1 recveive: %@", x);
    }];
}];

[[RACScheduler mainThreadScheduler] afterDelay:1 schedule:^{
    [signal subscribeNext:^(id x) {
        NSLog(@"Subscriber 2 recveive: %@", x);
    }];
}];  
```
运行输出如下
```
Signal was created.
Subscriber 1 recveive: 1
Subscriber 1 recveive: 2
Subscriber 1 recveive: 3
Subscriber 2 recveive: 1
Subscriber 2 recveive: 2
Subscriber 2 recveive: 3

```
可以看到我们在0.1秒和1秒后分别订阅了信号。两次订阅分别给两个订阅者发送了完整的消息。这完全符合我们对于冷信号的定义。
接下来。我们参考文档。看一下热信号是如何做出响应的。 [代码示例来源-美团技术团队](https://tech.meituan.com/2015/09/08/talk-about-reactivecocoas-cold-signal-and-hot-signal-part-1.html)
```
   RACMulticastConnection *connection = [[RACSignal createSignal:^RACDisposable *(id<RACSubscriber> subscriber) {
        [[RACScheduler mainThreadScheduler] afterDelay:1 schedule:^{
            [subscriber sendNext:@1];
        }];

        [[RACScheduler mainThreadScheduler] afterDelay:2 schedule:^{
            [subscriber sendNext:@2];
        }];
        
        [[RACScheduler mainThreadScheduler] afterDelay:3 schedule:^{
            [subscriber sendNext:@3];
        }];
        
        [[RACScheduler mainThreadScheduler] afterDelay:4 schedule:^{
            [subscriber sendCompleted];
        }];
        return nil;
    }] publish];
    [connection connect];
    RACSignal *signal = connection.signal;
    
    NSLog(@"Signal was created.");
    [[RACScheduler mainThreadScheduler] afterDelay:1.1 schedule:^{
        [signal subscribeNext:^(id x) {
            NSLog(@"Subscriber 1 recveive: %@", x);
        }];
    }];
    
    [[RACScheduler mainThreadScheduler] afterDelay:2.1 schedule:^{
        [signal subscribeNext:^(id x) {
            NSLog(@"Subscriber 2 recveive: %@", x);
        }];
    }];
```
这段代码作了如下的事情
1. 创建了一个`RACSignal`。并在 1、2、3秒的时候分别输出log
2. 将其`publish`。得到`RACMulticastConnection`。
3. 让`connection`进行连接操作
4. 分别在1.1秒和2.1秒订阅获得的信号。

运行这段代码可以看到出书结果为 
```
11:07:49.943 RACDemos[9418:1186344] Signal was created.
11:07:52.088 RACDemos[9418:1186344] Subscriber 1 recveive: 2
11:07:53.044 RACDemos[9418:1186344] Subscriber 1 recveive: 3
11:07:53.044 RACDemos[9418:1186344] Subscriber 2 recveive: 3
```
我们再来关注下输出结果的一些细节：
* 信号在11:07:49.943被创建
* 11:07:52.088时订阅者1才收到2这个值，说明1这个值没有接收到，时间间隔是2秒多
* 11:07:53.044时订阅者1和订阅者2同时收到3这个值，时间间隔是3秒多

通过上面的文档、和日志我们可以得出结论是 `[RACSignal publish]` `[RACMulticastConnection connect]` `[RACMulticastConnection signal]`这几个操作生成了一个热信号。 

### RACSubject
上面我们有讲解到冷信号是如何转变成为热信号的。但是，如果每一次使用热心好都需要这么转换的话就太麻烦了。因此框架提供了`RACSubject`。`RACSubject`继承与`RACSignal`它是一个天然的热信号。同时实现了管道协议和订阅者协议。使得它既可以被订阅。又可以用来发送数据。下面使用代码来演示下如何使用.

```
    RACSubject *subject = [RACSubject subject];

    // Subscriber 1
    [subject subscribeNext:^(id  _Nullable x) {
        NSLog(@"subscribe1 Sub: %@", x);
    }];
    [subject sendNext:@1];

    // Subscriber 2
    [subject subscribeNext:^(id  _Nullable x) {
        NSLog(@"subscribe2 Sub: %@", x);
    }];
    [subject sendNext:@2];

    // Subscriber 3
    [subject subscribeNext:^(id  _Nullable x) {
        NSLog(@"subscribe3 Sub: %@", x);
    }];
    [subject sendNext:@3];
    [subject sendCompleted];
```
控制台输出为
```
21:07:31.342468+0800 RACDemo[43162:5625969] subscribe1 Sub: 1
21:07:31.342687+0800 RACDemo[43162:5625969] subscribe1 Sub: 2
21:07:31.342820+0800 RACDemo[43162:5625969] subscribe2 Sub: 2
21:07:31.342943+0800 RACDemo[43162:5625969] subscribe1 Sub: 3
21:07:31.343061+0800 RACDemo[43162:5625969] subscribe2 Sub: 3
21:07:31.343169+0800 RACDemo[43162:5625969] subscribe3 Sub: 3
```

### RACSubject 实现原理
```
- (instancetype)init {
	self = [super init];
	if (self == nil) return nil;

	_disposable = [RACCompoundDisposable compoundDisposable];
	_subscribers = [[NSMutableArray alloc] initWithCapacity:1];
	
	return self;
}
```
首先 `RACSubject` 在初始化的时候创建了一个`_subscribers`数组。用于维护订阅者。
```
- (RACDisposable *)subscribe:(id<RACSubscriber>)subscriber {
	NSCParameterAssert(subscriber != nil);

	RACCompoundDisposable *disposable = [RACCompoundDisposable compoundDisposable];
	subscriber = [[RACPassthroughSubscriber alloc] initWithSubscriber:subscriber signal:self disposable:disposable];

	NSMutableArray *subscribers = self.subscribers;
	@synchronized (subscribers) {
		[subscribers addObject:subscriber];
	}
	
	[disposable addDisposable:[RACDisposable disposableWithBlock:^{
		@synchronized (subscribers) {
			// Since newer subscribers are generally shorter-lived, search
			// starting from the end of the list.
			NSUInteger index = [subscribers indexOfObjectWithOptions:NSEnumerationReverse passingTest:^ BOOL (id<RACSubscriber> obj, NSUInteger index, BOOL *stop) {
				return obj == subscriber;
			}];

			if (index != NSNotFound) [subscribers removeObjectAtIndex:index];
		}
	}]];

	return disposable;
}
```
在被订阅的时候。将传入的新订阅者保存在`subscribers`中。
```
- (void)sendNext:(id)value {
	[self enumerateSubscribersUsingBlock:^(id<RACSubscriber> subscriber) {
		[subscriber sendNext:value];
	}];
}

- (void)sendError:(NSError *)error {
	[self.disposable dispose];
	
	[self enumerateSubscribersUsingBlock:^(id<RACSubscriber> subscriber) {
		[subscriber sendError:error];
	}];
}

- (void)sendCompleted {
	[self.disposable dispose];
	
	[self enumerateSubscribersUsingBlock:^(id<RACSubscriber> subscriber) {
		[subscriber sendCompleted];
	}];
}
```
由于`RACSubject`遵守了 `RACSubscriber`协议。使得`RACSubject`也能主动发送信号。在发送信号的时候。通过数组遍历所有的订阅者并将信号传递出去。