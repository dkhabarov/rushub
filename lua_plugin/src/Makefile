#  files for make:
#
#  Plugin.h
#  tinyxml/tinystr.h
#  tinyxml/tinyxml.h
#  tinyxml/tinyxml.a

CXX = g++

CXXFLAGS = -O2 -Wall -Wextra -pipe
#CXXFLAGS = -g -Wall -Wextra -pipe

INCLUDE = -I/usr/include -I/usr/local/include -I/usr/include/lua5.1 -I/usr/include/lua -I/usr/include/lua/5.1

OBJS = $(CURDIR)/api.o $(CURDIR)/Dir.o $(CURDIR)/HubConfig.o $(CURDIR)/LuaInterpreter.o $(CURDIR)/LuaPlugin.o $(CURDIR)/LuaUtils.o $(CURDIR)/TaskList.o $(CURDIR)/TimerList.o $(CURDIR)/Uid.o

all: debian

linux-static: freebsd

freebsd: $(OBJS)
	$(CXX) -shared -fpic -lrt -o lua.so $(OBJS) $(CURDIR)/tinyxml/tinyxml.a /usr/local/lib/liblua.a

debian: $(OBJS)
	$(CXX) -llua5.1 -shared -fpic -lrt -o lua.so $(OBJS) $(CURDIR)/tinyxml/tinyxml.a

linux: $(OBJS)
	$(CXX) -llua -shared -fpic -lrt -o lua.so $(OBJS) $(CURDIR)/tinyxml/tinyxml.a

$(CURDIR)/api.o: $(CURDIR)/api.cpp $(CURDIR)/api.h $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/TaskList.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h $(CURDIR)/Uid.h $(CURDIR)/LuaUtils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Dir.o: $(CURDIR)/Dir.cpp $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/HubConfig.o: $(CURDIR)/HubConfig.cpp $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/HubConfig.h $(CURDIR)/TaskList.h $(CURDIR)/LuaPlugin.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/LuaInterpreter.o: $(CURDIR)/LuaInterpreter.cpp $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/TaskList.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h $(CURDIR)/Uid.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/LuaPlugin.o: $(CURDIR)/LuaPlugin.cpp $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/TaskList.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/LuaUtils.o: $(CURDIR)/LuaUtils.cpp $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/TaskList.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h $(CURDIR)/LuaUtils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/TaskList.o: $(CURDIR)/TaskList.cpp $(CURDIR)/List.h $(CURDIR)/TaskList.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/TimerList.o: $(CURDIR)/TimerList.cpp $(CURDIR)/List.h $(CURDIR)/TimerList.h $(CURDIR)/api.h $(CURDIR)/LuaInterpreter.h $(CURDIR)/TaskList.h $(CURDIR)/HubConfig.h $(CURDIR)/LuaPlugin.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Uid.o: $(CURDIR)/Uid.cpp $(CURDIR)/api.h $(CURDIR)/Uid.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) lua.so