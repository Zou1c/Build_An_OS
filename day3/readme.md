## 纸娃娃(haribote)操作系统

### 启动区内存地址

### 段寄存器的扩展位数的作用

### Makefile中变量的声明
进一步地，让Makefile也变得方便了  
(尤其是toolpath，只用改一遍就好了，太好了!)  

得，还是有一小部分路径得自己重写(harib00a completed)

### 尝试
harib00b写了一个if语句  

harib00c, harib00d后，完成了启动盘的读数据(这个程序已经用从软盘读取的数据填满了内存0x08200～ 0x34fff的地方)  

### 开始开发OS
harib00e到f，调试程序，读读改改  

harib00g, 想要切换显示模式(windows那样的画面)，还是得看BIOS里的函数是怎么支持的;  
harib00g中想要把磁盘装载内容的结束地址告诉给haribote.sys  

harib00h, 从BIOS获取键盘状态; (画面)设置信息保存  
这次VRAM的值是0xa0000。这个值又是从哪儿得来的呢？还是得看BIOS支持网页  

### C语言登场
在harib00i中  
为了调用C语言写的程序，添加了100行左右的汇编代码。

#### 关于C与汇编语言
(这里我比较好奇C怎么嵌入到汇编里的，于是问了问Gemini 1.5 pro experiment081)  
```
你提供的 Makefile 文件展示了如何将 bootpack.c 和汇编代码整合到一起构建 Haribote 操作系统镜像的过程。
简要概述：
编译 bootpack.c： 首先，bootpack.c 通过一系列步骤被编译成汇编代码 (bootpack.gas)，然后转换为 NASM 汇编 (bootpack.nas)，再汇编成目标文件 (bootpack.obj)，最终生成可重定位的二进制模块 (bootpack.hrb)。
链接汇编代码： asmhead.bin (汇编写的引导头) 和 bootpack.hrb 被链接在一起，生成最终的操作系统文件 haribote.sys。
创建引导镜像： ipl10.bin (汇编写的 IPL 代码) 和 haribote.sys 被合并到一个磁盘镜像文件 haribote.img 中。
详细步骤：
bootpack.c 到 bootpack.hrb：
bootpack.gas: bootpack.c 使用 cc1.exe 编译器生成 GAS 汇编代码。
bootpack.nas: bootpack.gas 使用 gas2nask.exe 转换为 NASM 汇编代码。
bootpack.obj: bootpack.nas 使用 nask.exe 汇编器生成目标文件。
bootpack.bim: bootpack.obj 使用 obj2bim.exe 转换为可重定位的二进制模块。
bootpack.hrb: bootpack.bim 使用 bim2hrb.exe 转换为最终的可执行模块。
创建 haribote.sys：
asmhead.bin: asmhead.nas (包含保护模式切换等代码) 使用 nask.exe 汇编器生成二进制文件。
haribote.sys: asmhead.bin 和 bootpack.hrb 使用 copy 命令合并，生成最终的操作系统文件。
创建 haribote.img：
ipl10.bin: ipl10.nas (IPL 代码) 使用 nask.exe 汇编器生成二进制文件。
haribote.img: 使用 edimg.exe 创建磁盘镜像，将 ipl10.bin 放置在引导扇区，并将 haribote.sys 复制到镜像中的其他位置。
关键点：
bootpack.c 编译成机器码后，与汇编代码 asmhead.bin 链接在一起，形成操作系统文件 haribote.sys。
haribote.sys 和引导程序 ipl10.bin 最终被合并到一个磁盘镜像文件 haribote.img 中。
当启动虚拟机或真实硬件时，IPL 代码会被加载并执行，它会将 haribote.sys 加载到内存中，然后跳转到 bootpack.hrb 的入口地址，开始执行操作系统。
总结：
通过一系列的编译、汇编和链接步骤，bootpack.c 的代码最终与汇编代码整合在一起，形成了一个完整的操作系统镜像文件。这个镜像文件可以被加载到虚拟机或真实硬件中，从而启动 Haribote 操作系统。
```

后来看到作者给的内容，也能解释：  
- 首先，使用cc1.exe从bootpack.c生成bootpack.gas。
- 第二步，使用gas2nask.exe从bootpack.gas生成bootpack.nas。
- 第三步，使用nask.exe从bootpack.nas生成bootpack.obj。
- 第四步，使用obi2bim.exe从bootpack.obj生成bootpack.bim。
- 最后，使用bim2hrb.exe从bootpack.bim生成bootpack.hrb。
- 这样就做成了机器语言，再使用copy指令将asmhead.bin与bootpack.hrb单纯结合到起来，
就成了haribote.sys。

#### 工具的补充讲解
```
让我们来分别看看这些工具的作用：

cc1.exe:
它是一个 C 编译器的前端，负责将 C 代码转换成汇编代码。 cc1.exe 生成的是 GAS (GNU Assembler) 格式的汇编代码 (bootpack.gas)。

gas2nask.exe:
这并非编译器，而是一个 汇编代码转换工具。 它将 GAS 格式的汇编代码 (bootpack.gas) 转换为 NASM (Netwide Assembler) 格式的汇编代码 (bootpack.nas)。 作者选择 NASM 可能是因为它更简单易用，也更适合用于编写引导程序和操作系统内核。

nask.exe:
这是一个 汇编器，将 NASM 格式的汇编代码 (bootpack.nas, ipl10.nas, asmhead.nas) 转换成 目标文件 (bootpack.obj, ipl10.bin, asmhead.bin)。 目标文件包含机器码和符号信息，但还不是最终可执行的程序。

obj2bim.exe:
这也不是编译器，而是一个 目标文件转换工具。 它将目标文件 (bootpack.obj) 转换成 BIM (Binary Image) 格式 (bootpack.bim)。 BIM 格式是一种自定义的二进制格式，用于 Haribote OS 的链接过程。

bim2hrb.exe:
它是一个 链接器，将 BIM 格式的文件 (bootpack.bim) 转换成最终可执行的 HRB (Haribote Relocatable Binary) 格式 (bootpack.hrb)。 HRB 格式是作者为 Haribote OS 设计的二进制格式，包含了程序的代码、数据和重定位信息。

```  

为什么会有这么多层？(可能的解释)
1. 简化实现： 作者选择自己实现工具链，而不是直接使用现成的编译器和链接器，可能是为了简化实现，并让读者更容易理解整个编译和链接过程。
2. 教学目的： 书中讲解了每个工具的实现细节，这可以让读者学习到编译器、汇编器和链接器的基本原理。
3. 特定需求： 作者设计的 HRB 格式可能是为了满足 Haribote OS 的特定需求，例如内存布局、地址空间等。

按作者的原话：  
> 这次提供的编译器，是以能适应各种不同操作系统为前提而设计的，所以对内部没有任何隐藏，是特意像这样多生成一些中间文件的。

### 最后
写一个io_hlt，能够被C语言程序调用(j项目)  

由于我每次make run之后都会用make only_src清理掉所有生成的东西，所以保留下来的都是可以阅读的，有帮助的东西，并且读者可以自己用make指令编译运行  

最后，得到QEMU的黑屏界面，嗯，那就对了，看到的还是啥也没变(毕竟到目前为止还在做准备)