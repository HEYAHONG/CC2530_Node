## 简介

这是一个CC2530节点程序的例子/模板,通过串口0(115200 8N1)查看程序输出

## 硬件

-  CC2530F256

## 操作系统

-  [contiki](!https://github.com/contiki-os/contiki.git)

## 网络协议栈

- RIME

## 编译器

- SDCC

**注意：SDCC的库应当支持huge-stack-auto，因此，当前直接使用SDCC官网下载/各linux发行版自带的二进制程序，可能无效。需要下载最新源码，写改源码并编译生成的SDCC程序才能正常使用。**

 1.修改incl.mk(最新版可能无需修改)
  + 文件路径： SDCC源码目录/device/lib/incl.mk
  + 修改操作，确保MODELS =后包含huge，修改后如下

```

      MODELS = small medium large huge
```
   2.修改 Makefile.in
   + 文件路径： SDCC源码目录/device/lib/Makefile.in
   + 修改操作，修改MCS51编译的TARGETS，修改后如下

```

    ifeq ($(OPT_DISABLE_MCS51), 0)
		    TARGETS        += models   model-mcs51-stack-auto
    endif
```

## 其他编译中需要使用到的工具

- make
- srec_cat
- python
- objcopy
- codeblocks(用于方便编写代码，可选)

对于使用ubuntu及其衍生版的用户，可直接安装以下软件包
- build-essential
- make
- python
- srecord
- codeblocks(可选)
