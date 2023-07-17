all: main

main: src/main.cpp
	$(CXXCOMPILER) $(CXXFLAGS) $(LDFLAGS) -o main src/main.cpp -I./include

clean:
	rm -f main