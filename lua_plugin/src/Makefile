#  files for make:
#
#  cdcconnbase.h
#  cdcparserbase.h
#  cdcserverbase.h
#  cdcuserbase.h
#  cplugin.h
#  cpluginbase.h
#  cpluginlistbase.h
#  cwebparserbase.h
#  tinyxml/tinystr.h
#  tinyxml/tinyxml.h
#  tinyxml/tinyxml.a

CXX = g++

CXXFLAGS = -O2 -Wall -fpic -shared
#CXXFLAGS = -g -Wall -fpic -shared

INCLUDE = -I/usr/include -I/usr/local/include -I/usr/include/lua5.1 -I/usr/include/lua -I/usr/include/lua/5.1

OBJS = $(CURDIR)/api.o $(CURDIR)/cdir.o $(CURDIR)/clua.o $(CURDIR)/cluainterpreter.o $(CURDIR)/ctaskslist.o $(CURDIR)/ctimerlist.o

lib: $(OBJS)
	$(CXX) -llua5.1 -shared -fpic -lrt -o lua.so $(OBJS) $(CURDIR)/tinyxml/tinyxml.a

$(CURDIR)/api.o: $(CURDIR)/api.cpp $(CURDIR)/cdcserverbase.h $(CURDIR)/api.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/clist.h $(CURDIR)/ctimerlist.h $(CURDIR)/cluainterpreter.h $(CURDIR)/ctaskslist.h $(CURDIR)/clua.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdir.o: $(CURDIR)/cdir.cpp $(CURDIR)/cdir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/clua.o: $(CURDIR)/clua.cpp $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/api.h $(CURDIR)/clist.h $(CURDIR)/ctimerlist.h $(CURDIR)/cluainterpreter.h $(CURDIR)/ctaskslist.h $(CURDIR)/clua.h $(CURDIR)/cdir.h $(CURDIR)/tinyxml/tinystr.h $(CURDIR)/tinyxml/tinyxml.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cluainterpreter.o: $(CURDIR)/cluainterpreter.cpp $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/api.h $(CURDIR)/clist.h $(CURDIR)/ctimerlist.h $(CURDIR)/cluainterpreter.h $(CURDIR)/ctaskslist.h $(CURDIR)/clua.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ctaskslist.o: $(CURDIR)/ctaskslist.cpp $(CURDIR)/clist.h $(CURDIR)/ctaskslist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/api.h $(CURDIR)/ctimerlist.h $(CURDIR)/cluainterpreter.h $(CURDIR)/clua.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ctimerlist.o: $(CURDIR)/ctimerlist.cpp $(CURDIR)/clist.h $(CURDIR)/ctimerlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/api.h $(CURDIR)/cluainterpreter.h $(CURDIR)/ctaskslist.h $(CURDIR)/clua.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) lua.so
