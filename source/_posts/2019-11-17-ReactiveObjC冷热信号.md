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

### 冷信号

