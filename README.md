# GSI SCU Utilities
By the way, for people who don't work at GSI, this repository is completely worthless.

This repository includes some sub-repositories, therefore don't forget the option "--recursive" if you'll clone this repository.

```git clone --recursive https://github.com/UlrichBecker/gsi_scu.git```

Add following to the environment variable ```LD_LIBRARY_PATH``` if not already done:<br />
```export LD_LIBRARY_PATH=/common/usr/cscofe/lib:/common/usr/fesa/lib:$LD_LIBRARY_PATH```

## Compiling
To compiling some of the including projects you need a LM32-cross-compiler.
On ASL you can find it here: ```/common/usr/cscofe/opt/compiler/lm32/```
Before you use it, you have to actualize the environment variable ```PATH```.<br />
E.g.: ```export PATH=/common/usr/cscofe/opt/compiler/lm32/bin:$PATH```<br />
If you work on a local machine, so you can [obtain and build the LM32-cross-compiler by this repository](https://github.com/UlrichBecker/gcc-toolchain-builder) as well.

Prerequisite to compile even of some linux-tools, that the LM32-application ```scu3_control.bin``` has already be compiled,
because the building of scu_control generates some headder files which will used by some linux-tools and libraries.

## Using the tool [cppcheck](https://cppcheck.sourceforge.io) as option
If you work on a local linux machine so it's meaningful to verify the C/C++ code by the additional tool<br />
[cppcheck](https://cppcheck.sourceforge.io) to make the sourcecode as clean as possible.<br />
The [cppcheck](https://cppcheck.sourceforge.io) code analysis can still find errors and warnings that the compiler misses.<br />
Unfortunately this tool isn't installed on the ASL- clusters at the moment.

**Note: For a release variant, only one number of warnings are permitted: zero!**

## Some Makefile targets
### Common targets
Prerequisite is that you has changed in a directory which includes a "Makefile".
* ```make```           Builds the corresponding binary file.
* ```make clean```     Deletes the binary file and all build artifacts.
* ```make rebuild```   Compiles the entire project new doesn't matter a source-file has changed or not.
* ```make ldep```      Prints the dependencies, that means all source and headder files which are involved at the corresponding project.
* ```make lsrc```      Prints all source code files belonging to the current project.
* ```make incdirs```   Prints all include directories belonging to the current project.
* ```make devargs```   Prints all definitions of the preprocessor which had been made in the makefile.
* ```make doc```       Generates a [doxygen](https://doxygen.nl) documentation in HTML of the corresponding project.
* ```make showdoc```   Generates a [doxygen](https://doxygen.nl) documentation in HTML of the corresponding project and invokes the firefox-browser.
* ```make deldoc```    Cleans the complete documentation.
* ```make asm```       Translates all source code modules in assembler.
* ```make objdump```   Builds the corresponding ```.elf``` - file and disassemble it.
* ```make check```     Makes C/C++ code analysis of the current project by the tool [cppcheck](https://cppcheck.sourceforge.io) which is more accuracy as the compilers code analysis.
* ```make V=1```       Builds the project in the verbosity mode.

### Makefile targets especially for LM32 projects
Prerequisite is that the environment variable ```SCU_URL``` exist which includes the name of the target-SCU.
E.g.: ```export SCU_URL=scuxl4711```
* ```make load```      Builds the LM32 binary file and loads it up to the corresponding SCU specified in ```SCU_URL```.
* ```make reset```     Makes a CPU-reset of the LM32.
* ```make info```      Prints the build-ID.

### Makefile targets especially for linux projects
For test and debug purposes the variable ```CALL_ARGS``` can set with commandline parameters.
* ```make run```       Builds the binary-file and invoke it.
* ```make dbg```       Builds a debugable binary-file and invokes the KDE-debugger frontend [```kdbg```](https://www.kdbg.org) if the makefile-variable ```DEBUG=1```. **Yes KDE!** That isn't exotic!
* ```make public```    Builds the binary-file and copy it in the directory ```/common/usr/cscofe/bin/```

## Some of the most important common makefile variables
* ```REPOSITORY_DIR```    This variable is the most important which shall contain the path who this GIT-repository is installed.<br />
                          The following both codelines shall be include at the bottom of each mekefile:<br />
                          For LM32-applications:<br />
                          ```REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)```<br />
                          ```include $(REPOSITORY_DIR)/makefiles/makefile.scu```<br />
                          For Linux-applications:<br />
                          ```REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)```<br />
                          ```include $(REPOSITORY_DIR)/makefiles/makefile.scun```
* ```MIAN_MODULE```       By default the name of the sourcefile which contains the function ```main()``` this is also the name of the binary file. E.g.:<br />
                          ```MIAN_MODULE = scu_control_os.c```
* ```SOURCE```            List of additional source files, which can be C-, cplusplus- or assembler- files. E.g.:<br />
                          ```SOURCE += mprintf.c```<br />
                          ```SOURCE += scu_task_daq.c```
* ```DEFINES```           List of predefined macros. E.g.:<br />
                          ```DEFINES += CONFIG_USE_TEMPERATURE_WATCHER```<br />
                          ```DEFINES += MAX_LM32_INTERRUPTS=2```
* ```INCLUDE_DIRS```      List of additional directories of header files. E.g.:<br />
                          ```INCLUDE_DIRS += $(RTOS_SRC_DIR)/include```
* ```CFLAGS```            Compiler flags. E.g.:<br />
                          ```CFLAGS += -Wall```<br />
                          ```CFLAGS += -Wfatal-errors```
* ```LD_FLAGS```          Linker flags. E.g.:<br />
                          ```LD_FLAGS += -Wl,--gc-sections```
* ```CODE_OPTIMIZATION``` Code optimization level. E.g.:<br />
                          ```CODE_OPTIMIZATION = s```
* ```NO_LTO```            By default the link time optimizer is active. If this is not desired then this variable has to be initialized by one. E.g.:<br />
                          ```NO_LTO = 1```
* ```SCU_URL```           Name of the developer-SCU. E.g.:<br />
                          ```SCU_URL = scuxl4711.acc.gsi.de```

## Makefile variables which concerns the the building of LM32 applications only
* ```USRCPUCLK```         CPU-clock in kHz. by default its 125000.
* ```RAM_SIZE```          Size of LM32-RAM in bytes. By default on SCU3: 147456 bytes.
* ```USE_RTOS```          The base source files of FreeRTOS will added to the project.
* ```RTOS_USING_HEAP```   Number of heap-model for FreeRTOS, this can be 1, 2, 3, 4 or 5. By default the heap-model 1 will used.

## Makefile variables which concerns the building of Linux applications only
* ```LIBS```              List of additional libraries. E.g.:<br />
                          ```LIBS += stdc++```
* ```LIB_DIRS```          Path to additional libraries.
* ```IS_LIBRARY```        Binary will build as library if its value is 1.
* ```STATIC_LIBRARY```    Binary will build as static-library if its value is 1.
* ```FOR_SCU```           Binary shall run on SCU only.
* ```FOR_SCU_AND_ACC```   Binary can run on SCU and ASL-cluster.
* ```DEBUG```             Binary will build with debug-infos. In this case the code optimization is 0,
                          doesn't matter how the value of ```CODE_OPTIMIZATION``` is. And the target ```make dbg``` becomes active.
* ```CALL_ARGS```         List of command line parameters for binary to test. This variable will used by the target ```make run``` and ```make dbg```.

## Building the classical "Hello world" example application for LM32
In this example you can see how the makefile has to look for a simple LM32 application.
1. Change in directory ```gsi_scu/demo-and-test/lm32/non-os/Hello_World```
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/demo-and-test/lm32/non-os/Hello_World/deploy_lm32/result/Hello_World.bin```
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```. Of course it is possible to define the variable ```SCU_URL``` in the makefile as well, this will overwrite a possible environment variable.
In the directory ```gsi_scu/demo-and-test/lm32/``` you can find further examples for LM32 with and without using [FreeRTOS](https://www.freertos.org).
**Note:** For LM32-applications using [FreeRTOS](https://www.freertos.org) the variable ```USE_RTOS``` has to be defined in the makefile.
In this manner the source files of [FreeRTOS](https://www.freertos.org) will added automatically to the project and the startup code [```crt0ScuLm32.S```](https://github.com/UlrichBecker/gsi_scu/blob/master/srclib/lm32/sys/crt0ScuLm32.S) becomes correct customized:  ```USE_RTOS = 1```
Further the heap-model using by [FreeRTOS](https://www.freertos.org) has to be choose by the makefile variable ```RTOS_USING_HEAP```. E.g.: ```RTOS_USING_HEAP = 4```


## Building the LM32-application ```scu3_control.bin```
This binary becomes in the future deprecated use the preemptive multitasking variant ```scu3_control_os.bin``` by implementation of [FreeRTOS](https://www.freertos.org) instead.
1. Change in directory ```gsi_scu/prj/scu-control/lm32-non-os_exe/SCU3/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/prj/scu-control/lm32-non-os_exe/SCU3/deploy_lm32/result/scu3_control.bin```.
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```.

## Building the [FreeRTOS](https://www.freertos.org) LM32-application ```scu3_control_os.bin```
1. Change in directory ```gsi_scu/prj/scu-control/lm32-rtos_exe/SCU3/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/prj/scu-control/lm32-rtos_exe/SCU3/deploy_lm32/result/scu3_control_os.bin```.
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```.

## Building the static library ```libscu_fg_feedback.a```
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/prj/scu-control/daq/linux/feedback/```.
2. Set some environment variables for building via YOCTO-SDK:<br />```unset LD_LIBRARY_PATH; source /common/usr/embedded/yocto/fesa/sdk/environment-setup-core2-64-ffos-linux```
3. Build binary file by typing: ```make```.
4. If the binary was successful built it will be in this directory: ```scu-control/daq/linux/feedback/deploy_x86_64_sdk_4.1.1/result/libscu_fg_feedback.a```

## Building the tool ```fg-feedback```. Monitoring and test tool for MIL and SCU- bus DAQs
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/tools/C++/fg-feedback/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/fg-feedback/deploy_x86_64/result/fg-feedback```

## Building the tool ```lm32-logd```. Linux demon for log-messages from a LM32 application
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/tools/C++/lm32-logd/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/lm32-logd/deploy_x86_64/result/lm32-logd```

## Building the tool ```fg-wave```. Visualization tool for ramp-files via [Gnuplot](http://www.gnuplot.info) without additional hardware
1. Change in directory ```gsi_scu/tools/C++/fg-wave```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/fg-wave/deploy_x86_64/result/fg-wave```.

## Building the tool ```mem-mon```. Visualizing and partitioner for memory- partitions in DDR3 for SCU3, resp. in SRAM for SCU4
1. Change in directory ```gsi_scu/tools/C++/mem-mon```
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/mem-mon/deploy_x86_64/result/mem-mon```


# [FreeRTOS](https://www.freertos.org) port especially for the LM32 of the GSI-SCU
The [FreeRTOS](https://www.freertos.org) port, respectively the hardware depended backend, for the LM32 is not a officially port
of [FreeRTOS](https://www.freertos.org). The source code can be find in the following directory:
[```gsi_scu/srclib/lm32/sys/FreeRTOS_LM32_SCU/port```](https://github.com/UlrichBecker/gsi_scu/tree/master/srclib/lm32/sys/FreeRTOS_LM32_SCU/port)


# Coding style
The coding style as far as possible to the [Eric Allman style](https://de.wikipedia.org/wiki/Einr%C3%BCckungsstil#Allman_/_BSD).<br />
Keep the code clearly arranged and in such a way that it is easy to read and least straining on the eyes.

Avoid squeezing as much as possible into one line of code, because that's not ingenious, it's just cluttered.<br />
This will neither make the binary smaller nor improve performance.

For example: Curly braces - open or closed - belong in a separate line of code if possible.

Bad:
```c
   if(a==b){
      b=1;
      c=2;
   }else if(x){
      b=2;
      c=3;
   }
```
Good:
```c
   if( a == b )
   {
      b = 1;
      c = 2;
   }
   else if( x != 0 )
   {
      b = 2;
      c = 3;
   }
```

Avoid tabulators except in Makefiles

Design comments so that they are recognizable as comments and not as possibly commented out source code.

Bad:
```C
// This is a bad comment.
/* This is also a
bad comment. */
```
Good:
```c
/*
 * This ist a good comment.
 */
```
Keep the comments [doxygen](https://doxygen.nl)- conform if possible.

Keep the implementation of finite state machines [DOCFSM](https://github.com/UlrichBecker/DocFsm)- conform.

At the head of a source code file belongs at least:
* The name of this file.
* A short description about the function of this file.
* The name of the author.
* The date of creation.

Compiler switches for conditional compilation should be written in capital letters, starting with the prefix ```CONFIG_```.

Macros should be written in capital letters if possible.

Self created data types should be in capital letters if possible and should end with the sufix ```_T```. 

C++ class names should start with a capital letter.

C++ member variables should start with the prefix ```m_```.

C++ class variables should start with the prefix ```c_```.

Global variables should start with the prefix ```g_```.

Global variables that are only visible in one compilation unit should start with the prefix ```mg_```.

Put a collection of global variables in a structure if possible. E.g.:
```c
typedef struct
{
   int a;
   int b;
   int c;
} MY_GLOBAL_T;
MY_GLOBAL_T g_myGlobals;
```

Local static variables should start with the prefix ```s_```. E.g.:
```c
void foo( void )
{
   static int s_callCount = 0;
   s_callCount++;
   printf( "Function %s() has been called for %d times.\n", __func__, s_callCount );
}
```
Non C++ functions without parameters has to be declared and defined by the keyword ```void```. See above.<br />
Otherwise it would be an open parameter-list in C, which makes code analysis difficult.<br />
But in C++ it's a good practice to do that as well.

In C++ use the C++ keyword ```nullptr``` instead of ```NULL```.

Pointers should begin with the lowercase letter ```p```.

C++ references should begin with the lowrecase letter ```r```.

Callback functions, that means functions which are called by pointer, or virtual functions in C++, shall begin with the prefix ```on```if possible.

Further explanations will follow as soon as possible.
