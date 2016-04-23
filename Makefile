.PHONY: all run clean

all:
	@ninja

run: all
	@./build/wt

clean:
	@ninja -t clean
