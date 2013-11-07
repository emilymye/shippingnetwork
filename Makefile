CXXFLAGS = -Wall -g

OBJECTS = Instance.o Engine.o
LIBS = fwk/BaseCollection.o fwk/BaseNotifiee.o fwk/Exception.o

default: test1 example

fwk/BaseCollection.o: fwk/BaseCollection.cpp fwk/BaseCollection.h
	$(CXX) -c -o $@ $^

fwk/BaseNotifiee.o: fwk/BaseNotifiee.cpp fwk/BaseNotifiee.h
	$(CXX) -c -o $@ $^

fwk/Exception.o: fwk/Exception.cpp fwk/Exception.h
	$(CXX) -c -o $@ $^

test1:	test1.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

example:	example.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f test1 test1.o $(OBJECTS) $(LIBS) *~



Instance.o: Instance.cpp Instance.h Engine.h Entity.h 
Engine.o: Engine.cpp Engine.h Instance.h Entity.h
test1.o: test1.cpp Instance.h
