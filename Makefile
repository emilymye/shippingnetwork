CXXFLAGS = -Wall -g

OBJECTS = ActivityImpl.o Instance.o Engine.o Exception.o

default: verification experiment
verification: verification.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
experiment: experiment.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

ActivityImpl.o: ActivityImpl.h Activity.h
Instance.o: Instance.cpp Instance.h Engine.h Entity.h ActivityImpl.h
Engine.o: Engine.cpp Engine.h Instance.h Entity.h
Exception.o: Exception.cpp Exception.h


verification.o: verification.cpp Instance.h ActivityImpl.h
experiment.o: experiment.cpp Instance.h ActivityImpl.h

clean:
	rm -f verification experiment *.o $(OBJECTS) $(LIBS) *~
