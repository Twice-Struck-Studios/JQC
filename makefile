JC = javac
JFLAGS = -sourcepath src -Xlint:all
SRC = ./src/
SP = com/twicestruck/jqc/
RM = rm -f
.SUFFIXES: .java .class
.PHONY: clean

VMCLASSES = $(patsubst $(SRC)$(SP)vm/%.java,$(SRC)$(SP)vm/%.class,$(wildcard $(SRC)$(SP)vm/*.java))

%.class: %.java
	$(JC) $(JFLAGS) $<

default: vm
	
vm: $(VMCLASSES)
	cd src && jar -cf ../JQC.jar $(SP)vm/*.class

	
clean:
	$(RM) src/com/twicestruck/jqc/vm/*.class
