CXX = g++

#CXXFLAGS = -O2 -Wall -Wextra -pipe -pedantic -Wno-long-long -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Wconversion -Wsign-conversion -Winit-self
CXXFLAGS = -O2 -Wall -Wextra -pipe
#CXXFLAGS = -g -Wall -Wextra -pipe

TARGET=RusHub
LDFLAGS=-lrt -lz -lpthread
LIBTINYXML=$(CURDIR)/tinyxml/libtinyxml.a
INCLUDE = -I/usr/include -I/usr/local/include

OBJS = $(CURDIR)/AdcParser.o $(CURDIR)/AdcProtocol.o $(CURDIR)/AntiFlood.o $(CURDIR)/CallList.o $(CURDIR)/Cli.o $(CURDIR)/ConfigItem.o $(CURDIR)/ConfigList.o $(CURDIR)/Conn.o $(CURDIR)/ConnChoose.o $(CURDIR)/ConnEpoll.o $(CURDIR)/ConnPoll.o $(CURDIR)/ConnSelect.o $(CURDIR)/DcCmd.o $(CURDIR)/DcConfig.o $(CURDIR)/DcConfigLoader.o $(CURDIR)/DcConn.o $(CURDIR)/DcIpList.o $(CURDIR)/DcProtocol.o $(CURDIR)/DcServer.o $(CURDIR)/DcUser.o $(CURDIR)/Dir.o $(CURDIR)/dlfuncs.o $(CURDIR)/Encoder.o $(CURDIR)/Exception.o $(CURDIR)/main.o $(CURDIR)/NmdcParser.o $(CURDIR)/NmdcProtocol.o $(CURDIR)/Obj.o $(CURDIR)/Param.o $(CURDIR)/PluginList.o $(CURDIR)/PluginLoader.o $(CURDIR)/Protocol.o $(CURDIR)/Server.o $(CURDIR)/Service.o $(CURDIR)/StringToArg.o $(CURDIR)/stringutils.o $(CURDIR)/Thread.o $(CURDIR)/TigerHash.o $(CURDIR)/Times.o $(CURDIR)/UserList.o $(CURDIR)/WebConn.o $(CURDIR)/WebParser.o $(CURDIR)/WebProtocol.o $(CURDIR)/ZlibFilter.o


all: $(TARGET)

freebsd: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBTINYXML) $(LDFLAGS)

solaris: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBTINYXML) $(LDFLAGS) -ldl -lsocket -lresolv

RusHub_shared: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) -ldl -lcap -ltinyxml

RusHub_caps: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBTINYXML) $(LDFLAGS) -ldl -lcap

RusHub: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBTINYXML) $(LDFLAGS) -ldl

$(CURDIR)/AdcParser.o: $(CURDIR)/AdcParser.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/AdcParser.h $(CURDIR)/Times.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/AdcProtocol.o: $(CURDIR)/AdcProtocol.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/DcProtocol.h $(CURDIR)/Plugin.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/TigerHash.h $(CURDIR)/Encoder.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/AntiFlood.o: $(CURDIR)/AntiFlood.cpp $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/AntiFlood.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/CallList.o: $(CURDIR)/CallList.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Plugin.h $(CURDIR)/CallList.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/PluginList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Cli.o: $(CURDIR)/Cli.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/Cli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConfigItem.o: $(CURDIR)/ConfigItem.cpp $(CURDIR)/stdinc.h $(CURDIR)/ConfigItem.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConfigList.o: $(CURDIR)/ConfigList.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Conn.o: $(CURDIR)/Conn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Thread.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnChoose.o: $(CURDIR)/ConnChoose.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnEpoll.o: $(CURDIR)/ConnEpoll.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnEpoll.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnPoll.o: $(CURDIR)/ConnPoll.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnPoll.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ConnSelect.o: $(CURDIR)/ConnSelect.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/ConnSelect.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcCmd.o: $(CURDIR)/DcCmd.cpp $(CURDIR)/Plugin.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/DcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/NmdcParser.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/DcCmd.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConfig.o: $(CURDIR)/DcConfig.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/DcConfig.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConfigLoader.o: $(CURDIR)/DcConfigLoader.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/stringutils.h $(CURDIR)/tinyxml/tinyxml.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcConn.o: $(CURDIR)/DcConn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/zlib/zlib.h $(CURDIR)/ZlibFilter.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcIpList.o: $(CURDIR)/DcIpList.cpp $(CURDIR)/HashTable.h $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcIpList.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcProtocol.o: $(CURDIR)/DcProtocol.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/DcProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcServer.o: $(CURDIR)/DcServer.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/DcCmd.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/DcUser.o: $(CURDIR)/DcUser.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/UserBase.h $(CURDIR)/Plugin.h $(CURDIR)/Protocol.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/ConnBase.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Dir.o: $(CURDIR)/Dir.cpp $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/dlfuncs.o: $(CURDIR)/dlfuncs.cpp $(CURDIR)/dlfuncs.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Encoder.o: $(CURDIR)/Encoder.cpp $(CURDIR)/stdinc.h $(CURDIR)/Encoder.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Exception.o: $(CURDIR)/Exception.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/Exception.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/main.o: $(CURDIR)/main.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/Service.h $(CURDIR)/Dir.h $(CURDIR)/Exception.h $(CURDIR)/Cli.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/NmdcParser.o: $(CURDIR)/NmdcParser.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/Times.h $(CURDIR)/UserBase.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/NmdcProtocol.o: $(CURDIR)/NmdcProtocol.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/DcProtocol.h $(CURDIR)/Plugin.h $(CURDIR)/NmdcParser.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/UserBase.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Obj.o: $(CURDIR)/Obj.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Mutex.h $(CURDIR)/Thread.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Param.o: $(CURDIR)/Param.cpp $(CURDIR)/Plugin.h $(CURDIR)/stdinc.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/UserBase.h $(CURDIR)/Protocol.h $(CURDIR)/NmdcParser.h $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/PluginList.o: $(CURDIR)/PluginList.cpp $(CURDIR)/HashTable.h $(CURDIR)/HashMap.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Plugin.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/dlfuncs.h $(CURDIR)/PluginLoader.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/PluginLoader.o: $(CURDIR)/PluginLoader.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/dlfuncs.h $(CURDIR)/PluginLoader.h $(CURDIR)/Plugin.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Protocol.o: $(CURDIR)/Protocol.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Server.o: $(CURDIR)/Server.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Service.o: $(CURDIR)/Service.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Service.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/WebConn.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h $(CURDIR)/StringToArg.h $(CURDIR)/Dir.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/StringToArg.o: $(CURDIR)/StringToArg.cpp $(CURDIR)/StringToArg.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/stringutils.o: $(CURDIR)/stringutils.cpp $(CURDIR)/stdinc.h $(CURDIR)/stringutils.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Thread.o: $(CURDIR)/Thread.cpp $(CURDIR)/Mutex.h $(CURDIR)/Thread.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/TigerHash.o: $(CURDIR)/TigerHash.cpp $(CURDIR)/stdinc.h $(CURDIR)/TigerHash.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Times.o: $(CURDIR)/Times.cpp $(CURDIR)/stdinc.h $(CURDIR)/Times.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/UserList.o: $(CURDIR)/UserList.cpp $(CURDIR)/UserBase.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/HashTable.h $(CURDIR)/Plugin.h $(CURDIR)/Mutex.h $(CURDIR)/UserList.h $(CURDIR)/Protocol.h $(CURDIR)/DcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/NmdcParser.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/DcCmd.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebConn.o: $(CURDIR)/WebConn.cpp $(CURDIR)/ConnBase.h $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Times.h $(CURDIR)/Protocol.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/Plugin.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/WebConn.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/UserBase.h $(CURDIR)/NmdcParser.h $(CURDIR)/stringutils.h $(CURDIR)/Any.h $(CURDIR)/Param.h $(CURDIR)/HashMap.h $(CURDIR)/DcUser.h $(CURDIR)/TimeOut.h $(CURDIR)/DcConn.h $(CURDIR)/DcProtocol.h $(CURDIR)/NmdcProtocol.h $(CURDIR)/AdcParser.h $(CURDIR)/AdcProtocol.h $(CURDIR)/ConfigItem.h $(CURDIR)/ConfigList.h $(CURDIR)/ConfigLoader.h $(CURDIR)/DcConfig.h $(CURDIR)/UserList.h $(CURDIR)/CallList.h $(CURDIR)/PluginList.h $(CURDIR)/DcConfigLoader.h $(CURDIR)/AntiFlood.h $(CURDIR)/DcIpList.h $(CURDIR)/Thread.h $(CURDIR)/DcServer.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebParser.o: $(CURDIR)/WebParser.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/WebParser.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/WebProtocol.o: $(CURDIR)/WebProtocol.cpp $(CURDIR)/stdinc.h $(CURDIR)/Obj.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/WebParser.h $(CURDIR)/WebProtocol.h $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Mutex.h $(CURDIR)/Conn.h $(CURDIR)/WebConn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/ZlibFilter.o: $(CURDIR)/ZlibFilter.cpp $(CURDIR)/stdinc.h $(CURDIR)/zlib/zlib.h $(CURDIR)/ZlibFilter.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) $(TARGET)
