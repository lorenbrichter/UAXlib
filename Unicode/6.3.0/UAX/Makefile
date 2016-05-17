default:
	make _Derived

clean:
	rm -rf UCDReader
	rm -rf UAXBidi-test
	rm -rf UAXNormalization-test
	rm -rf _Derived

test: UAXBidi-test UAXNormalization-test
	./UAXNormalization-test
	./UAXBidi-test

CPP = c++ -std=c++11
#-stdlib=libc++

_Derived: UCDReader
	mkdir -p _Derived
	./UCDReader ../UCD _Derived

COMMON = $(wildcard *.cpp) $(wildcard *.h)

UCDReader: UCDReader-main.cpp
	$(CPP) UCDReader-main.cpp -o UCDReader

UAXBidi-test: _Derived $(COMMON)
	$(CPP) -DUAX_BIDI_ENABLE_DEBUG_TRACE=1 UAXBidi-test.cpp UAXBidi.cpp UCDUtils.cpp -o UAXBidi-test

UAXNormalization-test: _Derived $(COMMON)
	$(CPP) UAXNormalization-test.cpp UAXNormalization.cpp UCDUtils.cpp -o UAXNormalization-test
