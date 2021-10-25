.PHONY: clean stage1 stage2

all: stage1 stage2

stage1:
	$(MAKE) -C stage1

stage2:
	$(MAKE) -C stage2

clean:
	$(MAKE) -C stage1 clean
	$(MAKE) -C stage2 clean