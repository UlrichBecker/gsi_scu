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

## Building LM32-application ```scu_control.bin```
1. Change in directory ```gsi_scu/prj/scu-control/lm32-non-os_exe```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/prj/scu-control/lm32-non-os_exe/deploy_lm32/result/scu_control.bin```.
4. If the variable ```SCU_URL``` specified so you can upload the binary-file by typing ```make load```.

## Building the static library ```libscu_fg_feedback.a```
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/prj/scu-control/daq/linux/feedback/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```scu-control/daq/linux/feedback/deploy_x86_64/result/libscu_fg_feedback.a```

## Building the tool ```fg-feedback```
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/tools/C++/fg-feedback/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/fg-feedback/deploy_x86_64/result/fg-feedback```

## Building the tool ```lm32-logd```
Prerequisite is that the LM32-binary file is already built.
1. Change in directory ```gsi_scu/tools/C++/lm32-logd/```.
2. Build binary file by typing: ```make```.
3. If the binary was successful built it will be in this directory: ```gsi_scu/tools/C++/lm32-logd/deploy_x86_64/result/lm32-logd```

Further explanations will follow as soon as possible.
