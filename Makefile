CXXFLAGS = -Wall -g

OBJECTS = Instance.o Engine.o
LIBS = fwk/BaseCollection.o fwk/BaseNotifiee.o fwk/Exception.o

default: test1 example
test1: test1.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^
example: example.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

fwk/BaseCollection.o: fwk/BaseCollection.cpp fwk/BaseCollection.h
fwk/BaseNotifiee.o: fwk/BaseNotifiee.cpp fwk/BaseNotifiee.h
fwk/Exception.o: fwk/Exception.cpp fwk/Exception.h

Instance.o: Instance.cpp Instance.h Engine.h Entity.h 
Engine.o: Engine.cpp Engine.h Instance.h Entity.h
test1.o: test1.cpp Instance.h
example.o: example.cpp Instance.h

clean:
	rm -f test1 example *.o $(OBJECTS) $(LIBS) *~



