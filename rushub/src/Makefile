CXX = g++

CXXFLAGS = -O2 -Wall -Wextra -pipe
#CXXFLAGS = -g -Wall -Wextra -pipe

INCLUDE = -I/usr/include -I/usr/local/include

OBJS = $(CURDIR)/AdcProtocol.o $(CURDIR)/AntiFlood.o $(CURDIR)/CallList.o $(CURDIR)/Cli.o $(CURDIR)/Config.o $(CURDIR)/ConfigList.o $(CURDIR)/Conn.o $(CURDIR)/ConnChoose.o $(CURDIR)/ConnEpoll.o $(CURDIR)/ConnPoll.o $(CURDIR)/ConnSelect.o $(CURDIR)/DcConfig.o $(CURDIR)/DcConfigLoader.o $(CURDIR)/DcConn.o $(CURDIR)/DcIpList.o $(CURDIR)/NmdcParser.o $(CURDIR)/DcServer.o $(CURDIR)/DcTag.o $(CURDIR)/DcUser.o $(CURDIR)/Dir.o $(CURDIR)/dlfcn.o $(CURDIR)/Exception.o $(CURDIR)/main.o $(CURDIR)/MyInfo.o $(CURDIR)/NmdcProtocol.o $(CURDIR)/Obj.o $(CURDIR)/PluginList.o $(CURDIR)/PluginLoader.o $(CURDIR)/Protocol.o $(CURDIR)/Server.o $(CURDIR)/Service.o $(CURDIR)/StringToArg.o $(CURDIR)/stringutils.o $(CURDIR)/Times.o $(CURDIR)/UserList.o $(CURDIR)/WebConn.o $(CURDIR)/WebParser.o $(CURDIR)/WebProtocol.o

RusHub: $(OBJS)
	$(CXX) -lrt -ldl -o RusHub $(OBJS) $(CURDIR)/tinyxml/tinyxml.a

$(CURDIR)/AdcProtocol.o: $(CURDIR)/AdcProtocol.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/AdcProtocol.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/AntiFlood.o: $(CURDIR)/AntiFlood.cpp $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/AntiFlood.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/CallList.o: $(CURDIR)/CallList.cpp $(CURDIR)/Obj.h $(CURDIR)/Plugin.h $(CURDIR)/CallList.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/PluginList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Cli.o: $(CURDIR)/Cli.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h $(CURDIR)/Cli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Config.o: $(CURDIR)/Config.cpp $(CURDIR)/Config.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConfigList.o: $(CURDIR)/ConfigList.cpp $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Conn.o: $(CURDIR)/Conn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnChoose.o: $(CURDIR)/ConnChoose.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnEpoll.o: $(CURDIR)/ConnEpoll.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnEpoll.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnPoll.o: $(CURDIR)/ConnPoll.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnPoll.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnSelect.o: $(CURDIR)/ConnSelect.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnSelect.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConfig.o: $(CURDIR)/DcConfig.cpp $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/DcConfig.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConfigLoader.o: $(CURDIR)/DcConfigLoader.cpp $(CURDIR)/Obj.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/stringutils.h $(CURDIR)/tinyxml/tinyxml.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConn.o: $(CURDIR)/DcConn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcIpList.o: $(CURDIR)/DcIpList.cpp $(CURDIR)/HashTable.h $(CURDIR)/ConnBase.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcIpList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/NmdcParser.o: $(CURDIR)/NmdcParser.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcServer.o: $(CURDIR)/DcServer.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcTag.o: $(CURDIR)/DcTag.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcUser.o: $(CURDIR)/DcUser.cpp $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/UserBase.h $(CURDIR)/Plugin.h $(CURDIR)/Protocol.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/ConnBase.h $(CURDIR)/Conn.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Dir.o: $(CURDIR)/Dir.cpp $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/dlfcn.o: $(CURDIR)/dlfcn.cpp $(CURDIR)/dlfcn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Exception.o: $(CURDIR)/Exception.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h $(CURDIR)/Exception.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/main.o: $(CURDIR)/main.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h $(CURDIR)/Service.h $(CURDIR)/Dir.h $(CURDIR)/Exception.h $(CURDIR)/Cli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/MyInfo.o: $(CURDIR)/MyInfo.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/NmdcProtocol.o: $(CURDIR)/NmdcProtocol.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Obj.o: $(CURDIR)/Obj.cpp $(CURDIR)/Obj.h $(CURDIR)/Times.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/PluginList.o: $(CURDIR)/PluginList.cpp $(CURDIR)/HashTable.h $(CURDIR)/Obj.h $(CURDIR)/HashMap.h $(CURDIR)/Plugin.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/dlfcn.h $(CURDIR)/PluginLoader.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/PluginLoader.o: $(CURDIR)/PluginLoader.cpp $(CURDIR)/Obj.h $(CURDIR)/dlfcn.h $(CURDIR)/PluginLoader.h $(CURDIR)/Plugin.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Protocol.o: $(CURDIR)/Protocol.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Server.o: $(CURDIR)/Server.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Service.o: $(CURDIR)/Service.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Service.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/DcServer.h $(CURDIR)/StringToArg.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/StringToArg.o: $(CURDIR)/StringToArg.cpp $(CURDIR)/StringToArg.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/stringutils.o: $(CURDIR)/stringutils.cpp $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Times.o: $(CURDIR)/Times.cpp $(CURDIR)/Times.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/UserList.o: $(CURDIR)/UserList.cpp $(CURDIR)/UserBase.h $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/Plugin.h $(CURDIR)/UserList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebConn.o: $(CURDIR)/WebConn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/WebConn.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/DcTag.h $(CURDIR)/MyInfo.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/HashMap.h $(CURDIR)/Config.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebParser.o: $(CURDIR)/WebParser.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/WebParser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebProtocol.o: $(CURDIR)/WebProtocol.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Conn.h $(CURDIR)/WebConn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) RusHub