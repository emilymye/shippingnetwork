CXXFLAGS = -Wall -g

OBJECTS = Reactors.o ActivityImpl.o Instance.o Engine.o Exception.o Reactors.o

default: verification experiment
verification: verification.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
experiment: experiment.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

ActivityImpl.o: ActivityImpl.h Activity.h
Instance.o: Instance.cpp Instance.h ActivityImpl.h Engine.h Entity.h Reactors.h
Engine.o: Engine.cpp Engine.h Instance.h Entity.h Activity.h
Exception.o: Exception.cpp Exception.h
Reactors.o: Reactors.cpp Reactors.h ActivityImpl.h Engine.h Entity.h 

verification.o: verification.cpp Exception.h Instance.h ActivityImpl.h
experiment.o: experiment.cpp Exception.h Instance.h ActivityImpl.h

clean:
	rm -f verification experiment *.o $(OBJECTS) $(LIBS) *~
