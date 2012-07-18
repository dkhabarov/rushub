
CXX = g++

CXXFLAGS = -O2 -Wall -Wextra
#CXXFLAGS = -g -Wall -Wextra

OBJS = $(CURDIR)/tinystr.o $(CURDIR)/tinyxml.o $(CURDIR)/tinyxmlerror.o $(CURDIR)/tinyxmlparser.o
  
libtinyxml.a: $(OBJS)
	$(AR) $(ARFLAGS) tinyxml.a $(OBJS)

$(CURDIR)/tinystr.o: $(CURDIR)/tinystr.cpp $(CURDIR)/tinystr.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CURDIR)/tinyxml.o: $(CURDIR)/tinyxml.cpp $(CURDIR)/tinyxml.h $(CURDIR)/tinystr.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CURDIR)/tinyxmlerror.o: $(CURDIR)/tinyxmlerror.cpp $(CURDIR)/tinyxml.h $(CURDIR)/tinystr.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CURDIR)/tinyxmlparser.o: $(CURDIR)/tinyxmlparser.cpp $(CURDIR)/tinyxml.h $(CURDIR)/tinystr.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	-rm -f $(OBJS) libtinyxml.a
