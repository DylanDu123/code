---
layout: post
title:  "解决多人开发cocoapods版本不同冲突的问题"
subtitle: "cocoapods"
date:   2019-05-19 10:45:13 -0400

category:
- 移动端
tags: 
- ruby
- iOS
---
## CocoaPods 是什么
---
[CocoaPods](https://github.com/CocoaPods/CocoaPods) 是使用 ruby 编写的开发 iOS 项目的库管理工具。它拥有超过 55,000 个库，并在超过 300 万个应用程序中使用。通过 [CocoaPods](https://github.com/CocoaPods/CocoaPods) 可以帮助我们优雅地扩展项目，便捷的导入第三方开源库。


## 多人开发使用 CocoaPods 的问题
---
有时候大家由于安装 CocoaPods 的版本不同，会造成生成的 .xcodeproj 文件发生冲突。这时就需要开发者指定 CocoaPods 的版本来避免这一冲突。

首先安装 Bundle
```
gem install bundler
```
创建 gemfile 文件

```
bundle init
```
这样，工程目录中就会多出一个 Gemfile 文件
可以在 Gemfile 文件中添加如下代码 指定使用的 CocoaPods 版本

```
source "https://rubygems.org"
gem 'cocoapods', '1.5.3'
```
到这里，管理工具已经安装完毕。
以后使用 CocoaPods 的用法只需要 在之前的命令前加上 `bundle exec` 就可以使用 Gemfile 指定的 CocoaPods 版本了
```
bundle exec pod install
bundle exec pod update
```
