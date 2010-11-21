CXX = g++

CXXFLAGS = -O2 -Wall -Wextra -pipe
#CXXFLAGS = -g -Wall -Wextra -pipe

INCLUDE = -I/usr/include -I/usr/local/include

OBJS = $(CURDIR)/cantiflood.o $(CURDIR)/ccalllist.o $(CURDIR)/cconfig.o $(CURDIR)/cconfiglist.o $(CURDIR)/cconfigloader.o $(CURDIR)/cconn.o $(CURDIR)/cconnchoose.o $(CURDIR)/cconnpoll.o $(CURDIR)/cconnselect.o $(CURDIR)/cdcconfig.o $(CURDIR)/cdcconn.o $(CURDIR)/cdciplist.o $(CURDIR)/cdcparser.o $(CURDIR)/cdcprotocol.o $(CURDIR)/cdcserver.o $(CURDIR)/cdcuser.o $(CURDIR)/cdir.o $(CURDIR)/cmainpath.o $(CURDIR)/cobj.o $(CURDIR)/cpluginlist.o $(CURDIR)/cpluginloader.o $(CURDIR)/cprotocol.o $(CURDIR)/cprotocolcmd.o $(CURDIR)/cserver.o $(CURDIR)/cservice.o $(CURDIR)/ctime.o $(CURDIR)/cuserlist.o $(CURDIR)/cwebconn.o $(CURDIR)/cwebparser.o $(CURDIR)/cwebprotocol.o $(CURDIR)/dlfcn.o $(CURDIR)/main.o $(CURDIR)/stringutils.o $(CURDIR)/ccli.o

RusHub: $(OBJS)
	$(CXX) -lrt -ldl -o RusHub $(OBJS) $(CURDIR)/tinyxml/tinyxml.a

$(CURDIR)/cantiflood.o: $(CURDIR)/cantiflood.cpp $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/tchashtable.h $(CURDIR)/cantiflood.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ccalllist.o: $(CURDIR)/ccalllist.cpp $(CURDIR)/cobj.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconfig.o: $(CURDIR)/cconfig.cpp $(CURDIR)/cconfig.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconfiglist.o: $(CURDIR)/cconfiglist.cpp $(CURDIR)/cobj.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconfigloader.o: $(CURDIR)/cconfigloader.cpp $(CURDIR)/cobj.h $(CURDIR)/cconfigloader.h $(CURDIR)/stringutils.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/tinyxml/tinyxml.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconn.o: $(CURDIR)/cconn.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconnchoose.o: $(CURDIR)/cconnchoose.cpp $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/cobj.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconnpoll.o: $(CURDIR)/cconnpoll.cpp $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/cobj.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cconnpoll.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cconnselect.o: $(CURDIR)/cconnselect.cpp $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/cobj.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cconnselect.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcconfig.o: $(CURDIR)/cdcconfig.cpp $(CURDIR)/cobj.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cprotocol.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcconfig.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcconn.o: $(CURDIR)/cdcconn.cpp $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdciplist.o: $(CURDIR)/cdciplist.cpp $(CURDIR)/tchashtable.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cdciplist.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcparser.o: $(CURDIR)/cdcparser.cpp $(CURDIR)/cobj.h $(CURDIR)/cprotocol.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cdcparser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcprotocol.o: $(CURDIR)/cdcprotocol.cpp $(CURDIR)/cobj.h $(CURDIR)/cprotocol.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcserver.o: $(CURDIR)/cdcserver.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdcuser.o: $(CURDIR)/cdcuser.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cconn.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cdir.o: $(CURDIR)/cdir.cpp $(CURDIR)/cdir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ccli.o: $(CURDIR)/ccli.cpp $(CURDIR)/ccli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<
  
$(CURDIR)/cmainpath.o: $(CURDIR)/cmainpath.cpp $(CURDIR)/cobj.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cmainpath.h $(CURDIR)/cdir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cobj.o: $(CURDIR)/cobj.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cdir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cpluginlist.o: $(CURDIR)/cpluginlist.cpp $(CURDIR)/tchashtable.h $(CURDIR)/cobj.h $(CURDIR)/tchashmap.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cdir.h $(CURDIR)/dlfcn.h $(CURDIR)/cpluginloader.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cpluginloader.o: $(CURDIR)/cpluginloader.cpp $(CURDIR)/cobj.h $(CURDIR)/dlfcn.h $(CURDIR)/cpluginloader.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/cpluginbase.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cprotocol.o: $(CURDIR)/cprotocol.cpp $(CURDIR)/cobj.h $(CURDIR)/cprotocol.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cprotocolcmd.o: $(CURDIR)/cprotocolcmd.cpp $(CURDIR)/cprotocolcmd.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cserver.o: $(CURDIR)/cserver.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cservice.o: $(CURDIR)/cservice.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cservice.h $(CURDIR)/cdir.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ctime.o: $(CURDIR)/ctime.cpp $(CURDIR)/conndef.h $(CURDIR)/ctime.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cuserlist.o: $(CURDIR)/cuserlist.cpp $(CURDIR)/cuserbase.h $(CURDIR)/cobj.h $(CURDIR)/tchashtable.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cuserlist.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cwebconn.o: $(CURDIR)/cwebconn.cpp $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cwebparserbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cwebconn.h $(CURDIR)/cwebparser.h $(CURDIR)/cwebprotocol.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cplugin.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cwebparser.o: $(CURDIR)/cwebparser.cpp $(CURDIR)/cobj.h $(CURDIR)/cprotocol.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cwebparser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/cwebprotocol.o: $(CURDIR)/cwebprotocol.cpp $(CURDIR)/cobj.h $(CURDIR)/cprotocol.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cwebparser.h $(CURDIR)/cwebprotocol.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cwebconn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/dlfcn.o: $(CURDIR)/dlfcn.cpp $(CURDIR)/dlfcn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/main.o: $(CURDIR)/main.cpp $(CURDIR)/cobj.h $(CURDIR)/conndef.h $(CURDIR)/ctime.h $(CURDIR)/cconnbase.h $(CURDIR)/tchashtable.h $(CURDIR)/tchashmap.h $(CURDIR)/cconnchoose.h $(CURDIR)/cmeanfrequency.h $(CURDIR)/cconnselect.h $(CURDIR)/cconnpoll.h $(CURDIR)/cserver.h $(CURDIR)/cdcserverbase.h $(CURDIR)/cdcuserbase.h $(CURDIR)/cdcconnbase.h $(CURDIR)/cprotocol.h $(CURDIR)/cdcparserbase.h $(CURDIR)/cconn.h $(CURDIR)/cuserbase.h $(CURDIR)/cdcuser.h $(CURDIR)/ctimeout.h $(CURDIR)/cdcconn.h $(CURDIR)/cprotocolcmd.h $(CURDIR)/cdcparser.h $(CURDIR)/cdcprotocol.h $(CURDIR)/cconfig.h $(CURDIR)/cconfiglist.h $(CURDIR)/cdcconfig.h $(CURDIR)/cuserlist.h $(CURDIR)/cpluginlistbase.h $(CURDIR)/ccalllist.h $(CURDIR)/cpluginbase.h $(CURDIR)/cwebparserbase.h $(CURDIR)/cplugin.h $(CURDIR)/cwebconn.h $(CURDIR)/cpluginlist.h $(CURDIR)/stringutils.h $(CURDIR)/cconfigloader.h $(CURDIR)/cantiflood.h $(CURDIR)/cdciplist.h $(CURDIR)/cmainpath.h $(CURDIR)/cdcserver.h $(CURDIR)/cdir.h $(CURDIR)/cservice.h $(CURDIR)/ccli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/stringutils.o: $(CURDIR)/stringutils.cpp $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) RusHub