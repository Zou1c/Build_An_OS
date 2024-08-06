## 让helloos更加伟大

### 读懂汇编 & 重新编译
helloos3就是帮助理解这个汇编语言的;  

helloos4的编译流程：  
![alt text](pictures/batch.svg)

### Makefile制作
> 诶呀呀，一看到这个流程就会觉得，“天哪，我接着开发下去的话，每次想要得到最后的结果都要输入好几条指令，每条指令还有自己的参数，然后才能得到最终想要的文件”

于是，make工具就被GNU开发了出来，Makefile文件也就相当于直接替代了所有你要执行的命令  

helloos5可以直接去看对应的Makefile文件，这里就不介绍语法了，可以先看看原文这里的描述，通俗易懂  

最终，你能得到：  
在命令行输入make img, 会从ipl.nas开始一直编译得到helloos.img  
(还有make clean, make run等方便的命令; 虽然helloos程序还是没变，但是对于开发来说确实方便了不少)

### 有一点需要注意的
(我比较疑惑，为什么双击make.bat，就能直接在helloos5下面得到结果文件，make.exe是怎么知道的？)  
> 双击执行 make.bat 并不会直接告诉 make.exe 你的当前路径，但会将 make.exe 的执行路径设置为 make.bat 所在的路径，从而间接地告诉 make.exe 在哪里查找 Makefile 文件。

我后来试了一下，简单来说，就是Windows下“双击make.bat”会呼唤出控制台，直接锁定到make.bat的路径，之后的一切就顺其自然了  

(你可以试一试写一个a.bat，里面的内容如下，看看直接通过GUI执行它会发生什么)
```
pause
```