CXXFLAGS = -Wall -g

OBJECTS = Instance.o Engine.o
LIBS = fwk/BaseCollection.o fwk/BaseNotifiee.o fwk/Exception.o

default: test1 example

fwk/BaseCollection.o: fwk/BaseCollection.cpp fwk/BaseCollection.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
fwk/BaseNotifiee.o: fwk/BaseNotifiee.cpp fwk/BaseNotifiee.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
fwk/Exception.o: fwk/Exception.cpp fwk/Exception.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Instance.o: Instance.cpp Instance.h Engine.h Entity.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $<
Engine.o: Engine.cpp Engine.h Instance.h Entity.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test1.o: test1.cpp Instance.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
example.o: example.cpp Instance.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test1: test1.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

example: example.o $(OBJECTS) $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f test1 example *.o $(OBJECTS) $(LIBS) *~



