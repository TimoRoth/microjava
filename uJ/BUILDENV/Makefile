#USAGE:  make -C <PATH_TO_THIS_FILES_PARENT_DIR> CLASS=<MY_JAVA_FILE_NAME_WITHOUT_JAVA_EXTENSION> OUT=<OUTPUT_FILE_NAME>


SELF	:= $(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
CLASS	?= PLEASE_SPECIFY_CLASS_PARAM
DIR	:= $(PWD)
CCVT	?= $(SELF)/../../classCvt/classCvt
TOBIN	?= $(SELF)/../../classCvt/tobin
OUTPUT	?= 

ifeq ($(BUILD), just_filelist)
	CMD = echo
else
	ifeq ($(BUILD), unopt)

		CMD = $(TOBIN)
	else
		CMD = $(TOBIN) -c $(CCVT)
	endif
endif

ifeq ($(OUTPUT), )
	OUT = &1
else
	OUT = $(DIR)/$(OUTPUT)
endif

ifeq ($(SKIPSELF), yes)
	MYCLASS = 
else
	MYCLASS = $(DIR)/$(CLASS).class
endif

all:
	CLASSPATH=$(SELF)/RT/real:$(SELF)/RT/fake:$(DIR) javac $(DIR)/$(CLASS).java
	find $(SELF)/RT/real -iname "*.class" -type f > $(SELF)/files
	ls $(DIR)/*.class | grep -v "$(CLASS)\.class" >> $(SELF)/files || true
	cat $(SELF)/files | xargs $(CMD) $(MYCLASS) >$(OUT)
	rm -f $(SELF)/files

runtime:
	find $(SELF)/RT -name "*.java" -type f | xargs javac

rtclean:	clean
	find $(SELF)/RT -name "*.class" -type f | xargs rm -f

clean:
	rm -f $(DIR)/*.class
