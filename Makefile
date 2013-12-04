CXXFLAGS = -Wall -g

OBJECTS = Instance.o Engine.o
LIBS = Exception.o

default: test1 example
test1: test1.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^
example: example.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

Exception.o: Exception.cpp Exception.h
Instance.o: Instance.cpp Instance.h Engine.h Entity.h
Engine.o: Engine.cpp Engine.h Instance.h Entity.h Activity.h

test1.o: test1.cpp Instance.h
example.o: example.cpp Instance.h

clean:
	rm -f test1 example *.o $(OBJECTS) $(LIBS) *~



