# GSI SCU Utilities

This repository includes some sub-repositories, therefore don't forget the option "--recursive" if you'll clone this repository.

```git clone --recursive https://github.com/UlrichBecker/gsi_scu.git```

By the way, for people who don't work at GSI, this repository is completely worthless.

## Compiling
To compiling some of the including projects you need a LM32-cross-compiler.
On ASL you can find it here: ```/common/usr/cscofe/opt/compiler/lm32/```
Before you use it, you have to actualize the environment variable ```PATH``` by invoking the script: ```set-gcc-path```
E.g.: ```. /common/usr/cscofe/opt/compiler/lm32/set-gcc-path```
Don't forget the dot at the beginning.
If you work on a local machine, so you can [obtain and build the LM32-cross-compiler by this repository](https://github.com/UlrichBecker/gcc-toolchain-builder) as well.

Prerequisite to compile even of some linux-tools, that the LM32-application ```scu_control.bin``` has already be compiled,
because the building of scu_control generates some headder files which will used by some linux-tools and libraries.

## Some Makefile targets
### Common targets
Prerequisite is that you has changed in a directory which includes a "Makefile".
* ```make```           Builds the corresponding binary file.
* ```make clean```     Deletes the binary file and all build artifacts.
* ```make rebuild```   Compiles the entire project new doesn't matter a source-file has changed or not.
* ```make ldep```      Prints the dependencies, that means all source and headder files which are involved at the corresponding project.
* ```make doc```       Generates a [doxygen](https://doxygen.nl) documentation in HTML of the corresponding project.
* ```make showdoc```   Generates a [doxygen](https://doxygen.nl) documentation in HTML of the corresponding project and invokes the firefox-browser.
* ```make deldoc```    Cleans the complete documentation.
* ```make asm```       Translates all source code modules in assembler.
* ```make objdump```   Builds the corresponding ```.elf``` - file and disassemble it.
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
* ```make public       Builds the binary-file and copy it in the directory ```/common/usr/cscofe/bin/```

## Some of the most important makefile variables
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


## Building the LM32-application ```scu_control.bin```
This binary becomes in the future deprecated use the preemptive multitasking variant ```scu_control_os.bin``` by implementation of [FreeRTOS](https://www.freertos.org) instead.
1. Change in directory ```gsi_scu/prj/scu-control/lm32-non-os_exe```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/prj/scu-control/lm32-non-os_exe/deploy_lm32/result/scu_control.bin```.
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```.

## Building the [FreeRTOS](https://www.freertos.org) LM32-application ```scu_control_os.bin```
1. Change in directory ```gsi_scu/prj/scu-control/lm32-rtos_exe```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/prj/scu-control/lm32-rtos_exe/deploy_lm32/result/scu_control.bin```.
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```.

## Building the static library ```libscu_fg_feedback.a```
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/prj/scu-control/daq/linux/feedback/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```scu-control/daq/linux/feedback/deploy_x86_64/result/libscu_fg_feedback.a```

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


### [FreeRTOS](https://www.freertos.org) port especially for the LM32 of the GSI-SCU
The [FreeRTOS](https://www.freertos.org) port, respectively the hardware depended backend, for the LM32 is not a officially port
of [FreeRTOS](https://www.freertos.org). The source code can be find in the following directory:
[```gsi_scu/srclib/lm32/sys/FreeRTOS_LM32_SCU/port```](https://github.com/UlrichBecker/gsi_scu/tree/master/srclib/lm32/sys/FreeRTOS_LM32_SCU/port)

Further explanations will follow as soon as possible.
