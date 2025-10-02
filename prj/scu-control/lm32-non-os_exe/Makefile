SUBDIRS = SCU3

all clean rebuild load doc showdoc deldoc:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
