SUBDIRS = lm32-non-os_exe lm32-rtos_exe

all clean rebuild doc showdoc deldoc:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
