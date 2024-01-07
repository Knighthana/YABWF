# YABWF - Yet Another Boa Webserver Fork

yabwf README docv1.0.0 [Knighthana](https://github.com/Knighthana) 2024/01/07

wish to talk about project?

contact at [Yet Another Boa Webserver Fork](https://github.com/Knighthana/YABWF)

## Sincere thanks to the Boa Webserver project contributors!

Thanks to Larry Doolittle and Jon Nelson and other generous good people for creating this project

[Boa Webserver](http://http://www.boa.org/)

[Boa SourceForge Page](https://sourceforge.net/projects/boa/)

## 关于YABWF项目

这是一个Boa Webserver的fork

由于原Boa项目已经不再更新，因此我将其最新版本0.94.14rc21`fork`出来进行修改并且另赋一个新项目名**YABWF**(**Y**et **A**nother **B**oa **W**ebserver **F**ork)以便与原项目、开发成员进行区分，进行版本号更迭等

## 免责声明

### 代码公开过程没有任何隐含的赞同、支持行为

本项目的源代码完全是基于原项目Boa进行的二次修改，原项目Boa已经基于GPL协议进行源代码公开

本项目源代码同样基于GPL协议(GPL原文请参考项目中提供的COPYING文件)公开

公开代码完全出于**遵守GPL**以及**学习交流**的目的

不保证任何对代码文件或文档文件无论未修改、或修改后投入生产环境的任何行为，不对造成的任何损失负责

不认同、不许可任何利用此项目代码或文档进行任何违德、非法、反人类活动的行为

### 已提前告知信息安全问题

请注意，已经有报告指出Boa服务器存在一些**安全漏洞**

> CVE-2023-7208
>
> CVE-2021-35395
>
> CVE-2019-9976
>
> CVE-2019-7384
>
> CVE-2016-9564
>
> CVE-2009-4496
>
> CVE-2007-4915
>
> CVE-2005-0864
>
> CVE-2000-0920

本项目目前**没有**对这些漏洞进行修复，因此请不要在公开服务器上使用本项目

对此已经提前告知，若有造成损失概不负责

*目前暂时还没有修复这些漏洞的计划，但是如果有时间的话会出于兴趣对照CVE进行漏洞修复工作*

*但这也不代表那时修复后的`yabwf`仍然是安全的，因此仍然不建议在公开服务器上使用本项目*

### 若继续则默认为赞同免责声明

若不同意免责声明，请停止使用本项目并删除所有本项目的文件

若继续，默认已经理解并赞同“免责声明”的内容

## 编译与预安装

正确的配置文件设定是不可或缺的

对于一般情况，完全可以在运行前通过`-c`选项手动指定配置文件`boa.conf`的位置

```shell
boa -c /usr/local/boa
```

注意这里指定的是所在路径而不是具体的文件名

编译与预安装流程已经经过优化，对于初学者，可以使用项目根目录下的`construct.sh`进行一键编译

此脚本使用参数传递指令：

`construct.sh build`为编译

`construct.sh clean`为清理

此脚本使用“环境变量”传递编译选项，各编译选项详细介绍如下：

### 编译选项

#### `RUNPREFIXDIR`

在运行环境中的安装路径前缀；一般来说，如果运行环境只预留了`/opt/userapp`用于安装应用层软件，那么`RUNPREFIXDIR`就是`/opt/userapp`

默认值为`/`

#### `BUILDMACHINE`

正在使用的编译设备的类型，对于一般的PC，此项设置为`amd64`

默认值为`amd64`

#### `HOST`

交叉编译器设定

若运行环境为amd64的PC设备，编译器为`x86_64-linux-gnu-gcc`，则`HOST`为`x86_64-linux-gnu`

作为参考，常见的使用GNU EABI的支持硬件浮点计算单元ARM设备上的Linux运行环境的交叉编译器为`arm-linux-gnueabihf-gcc`，则`HOST`为`arm-linux-gnueabihf`

默认设置为`x86_64-linux-gnu`，代表运行环境为PC

#### `TRANSFERDIR`

中转路径，即在编译机器上暂储的路径

例如，需要将`YABWF`安装在嵌入式设备的`/opt/userapp`目录中，于是指定了`RUNPREFIXDIR`为`/opt/userapp`

此时，假如在编译时指定了`TRANSFERDIR=/home/me/codes/buildout`

那么，construct完成后在编译机器的`/home/me/codes/buildout`目录中就可以看到`opt`文件夹

将之整体打包压缩即可移植到目标设备中

默认值为项目目录下的`out`中

#### `CPUTHREAD`

编译时使用的CPU线程数

默认值为自动获取到的最大值，假如实际情况不允许这么做，请手动指定其它数字

### 编译与预安装举例

#### 1. 背景假设

假如现在需要在一台amd64的公共服务器上编译此项目，并预备移植到一台安装运行了Linux的ARM嵌入式设备上

编译机上的交叉编译器安装在`/opt/crosscompilers/OEMSoCMark`中

其中，`gcc`的路径为`/opt/crosscompilers/OEMSoCMark/bin/arm-oem-linux-gnueabihf-gcc`

`ld`的路径为`/opt/crosscompilers/OEMSoCMark/bin/arm-oem-linux-gnueabihf-ld`

服务器上用于中转文件的目录为`/pack`

服务器没有限制可使用的CPU线程数

在ARM设备上预备分配的存储位置为`/opt/userapp`，其它位置对于应用层软件而言或是不可访问，或是非持久化分区

#### 2. 编译与预安装

然后以如下方式调用`construct.sh`

```shell
RUNPREFIXDIR=/opt/userapp BUILDMACHINE=amd64 HOST=/opt/crosscompilers/OEMSoCMark/bin/arm-oem-linux-gnueabihf TRANSFERDIR=/pack ./construct.sh
```

那么就可以在`/pack`下看到`opt`目录

如果编译成功，则分别在`/pack/opt/userapp/bin`中和`/pack/opt/userapp/lib`中可以看到`boa`和`boa_indexer`

#### 3. 移植(示例)

在编译服务器的`/pack`目录下执行一次
```shell
tar -czf applicationinstallresource.tar.gz ./*
```

打包所有需要移植的文件

假设在ARM设备的`/download/`目录获取了压缩包，执行

```shell
cd /download
mkdir -p /download/updateresource
rm -rf /download/updateresource/*
tar -xzf applicationinstallresource.tar.gz -C /download/updateresource
```

视实际情况确定使用的打包与解包程序和选项

> 例如某些环境中未安装`gzip`，于是`tar`的`-z`选项是不可用的，因此打包时使用`tar -cf`解包时使用`tar -xf`
>
> 或者打包时使用了`zip`，则解包时使用`unzip`
>
> 打包解包的方式、移植流程实际上与本项目无关

正确解包获取文件后，运行

```shell
cd /download/updateresource
cp -rf ./* /
```

就完成了二进制文件的移植流程

为了正常运行服务器软件，还需要正确移植配置文件

新建或进入路径`/opt/userapp/etc`

新建目录`boa`，放入语法正确的`boa.conf`文件，请注意文件的`DirectoryMaker`、`MimeTypes`、`ScriptAlias`等需要作对应修改，参考"配置文件"一节

返回`/opt/userapp/etc`，确保`mime.types`文件存在，若不存在，可以找`nginx`项目借一份`mime.types`文件放在此位置

### 运行服务器

通过shell执行

```shell
/opt/userapp/bin/boa -c /opt/userapp/etc/boa
```

运行服务器

可以通过连接到设备的HTTP浏览器来确认服务器按照预期运行

## 配置文件

项目的配置文件是`boa.conf`

请参考`examples/boa.conf`配置文件的写法，其中已经对关键的配置选项给出了说明

注意，`examples/boa.conf`中配置选项的内容不会根据`construct.sh`进行变化，请手动修改它们

### 配置文件中的易变设定

`ErrorLog` 错误日志的路径与文件名

`AccessLog` 访问日志的路径与文件名

若要关闭日志输出，简单地将之全部设定为`/dev/null`即可

不建议将日志路径与文件名设定为无法访问的路径与文件名，那种情况下服务器将无法正常启动

`DocumentRoot` 通过HTTP文件的路径

`DirectoryMaker` boa_indexer的路径，如果修改了`RUNPREFIXDIR`则需要手动做对应修改

`MimeTypes` MIME文件路径，若在其它位置需要手动修改

`CGIPath` 为CGI程序设定的`PATH`环境变量

`CGILdLibraryPath` 为CGI程序设定的`LD_LIBRARY_PATH`环境变量

`ScriptAlias` alias设定，默认出于安全考虑仅仅是将`/cgi-bin/`alias到`/srv/www/cgi-bin/`

功能上支持alias其它的目录

例如，若有一个cgi程序为`/var/undermonitor/viatcp/account.cgi`

可以为其设定`ScriptAlias /cgi-bin/ /var/undermonitor/viatcp/`

此时前端可以在网页JavaScript中通过`/cgi-bin/account.cgi`直接`POST` `GET` `PUT` `DELETE`到`account.cgi`

### 配置文件读取过程的自定义

项目二进制程序寻找配置文件使用的路径与文件名在`src/defines.h`中进行了定义

假如运行的环境不允许通过标准的参数传入参数对于配置文件的设置，或是出于特定原因需要修改默认的配置文件名称`boa.conf`

那么可以通过修改源代码对配置文件进行自定义

手动在项目的`src/defines.h`中修改这些内容以便达成目的

首先对

```c
#ifndef SERVER_ROOT
#define SERVER_ROOT "/etc/boa"
#endif
```
与
```c
#define DEFAULT_CONFIG_FILE "boa.conf" /* locate me in the server root */
```

的内容进行修改，前者指定了配置文件的存储路径，后者指定了配置文件的文件名

此外如有必要还可以通过修改`src/defines.h`的

```c
#define MIME_TYPES_DEFAULT "/etc/mime.types"
```

来指定MIME文件的位置，不过默认情况下完全可以在`boa.conf`中指定MIME的位置

## 常见问题

### 静默退出，shell得到返回值`1`

对于使用了默认`boa.conf`的用户来说，看不到报错是因为报错被放入了日志文件中

对于这种情况，日志（默认位于`/tmp/yabwferr.log`）中会写明原因

一种常见的原因是`boa`无法绑定TCP端口，日志中写作`unable to bind: ******`

`Permission denied`: Linux操作系统中，小于`1024`的TCP端口需要提供`root`级别的权限才能绑定，需要切换用户或者使用`sudo`一类的工具再启动`boa`

`Address already in use`: 端口被占用，有可能是之前启动的`yabwf`进程占用了端口，或者其他的什么进程占用了这个tcp端口，使用`lsof -i:端口号`或者`netstatu -tlp | grep 端口号`来查找

### Could not chdir to "/etc/boa": aborting

这是由于没有正确设置配置文件造成的

参考上文，解决方案有三种

1. 使用`-c`选项指定配置文件所在的路径；
2. 在`/etc`中新建`boa`目录，放入`boa.conf`文件夹；
3. 修改源代码，重新指定默认的配置文件

建议使用第一种方法解决

### log.c:53 (open_logs) - unable to open error log: Permission denied

这是日志没有正确配置导致的，请检查对应的硬盘读写权限、用户访问权限

如果想在屏幕上(stderr)获取日志，就删掉任何有关`Errorlog`的配置选项，根据默认设置它将输出到`stderr`

如果不想记录任何日志，就设定`ErrorLog`与`AccessLog`为`/dev/null`

# 以下是原Boa Webserver的README原文，用于参考

This is Boa, a high performance web server for Unix-alike computers,
covered by the Gnu General Public License.  This is version 0.94,
released January 2000.  It is well tested and appears to be of
at least "gamma" quality.

Boa was created in 1991 by Paul Phillips <paulp@go2net.com>.  It is now being
maintained and enhanced by Larry Doolittle <ldoolitt@boa.org>
and Jon Nelson <jnelson@boa.org>.

For more information (including installation instructions) examine
the file docs/boa.txt or docs/boa.dvi, point your web browser to docs/boa.html,
or visit the Boa homepage at

        http://www.boa.org/

