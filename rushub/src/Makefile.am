bin_PROGRAMS=RusHub
RusHub_SOURCES= AdcParser.cpp ConfigList.h DcConfig.cpp Dir.h NmdcParser.h Server.h UserList.h AdcParser.h \
		ConfigLoader.h DcConfig.h dlfuncs.cpp NmdcProtocol.cpp Service.cpp WebConn.cpp AdcProtocol.cpp \
		ConnBase.h DcConfigLoader.cpp dlfuncs.h NmdcProtocol.h Service.h WebConn.h AdcProtocol.h \
		ConnChoose.cpp DcConfigLoader.h Exception.cpp Obj.cpp StringToArg.cpp WebParser.cpp \
		AntiFlood.cpp ConnChoose.h DcConn.cpp Exception.h Obj.h StringToArg.h WebParser.h AntiFlood.h \
		Conn.cpp DcConn.h HashMap.h Plugin.h stringutils.cpp WebProtocol.cpp CallList.cpp ConnEpoll.cpp \
		DcIpList.cpp HashTable.h PluginList.cpp stringutils.h WebProtocol.h CallList.h ConnEpoll.h \
		DcIpList.h PluginList.h TimeOut.h Cli.cpp Conn.h DcServer.cpp main.cpp PluginLoader.cpp Times.cpp \
		Cli.h ConnPoll.cpp DcServer.h PluginLoader.h Times.h ConfigItem.cpp ConnPoll.h DcUser.cpp Protocol.cpp \
		ConfigItem.h ConnSelect.cpp DcUser.h MeanFrequency.h Protocol.h UserBase.h ConfigList.cpp ConnSelect.h \
		Dir.cpp NmdcParser.cpp Server.cpp UserList.cpp Param.h Param.cpp Any.h DcProtocol.h DcProtocol.cpp \
		stdinc.h Encoder.h Encoder.cpp TigerHash.h TigerHash.cpp ZlibFilter.cpp Thread.h Thread.cpp Mutex.h \
		DcCmd.h DcCmd.cpp Logger.h Logger.cpp Singleton.h
RusHub_LDFLAGS = $(RUSHUB_LIBS)

if STATIC_TINYXML
  RusHub_LDADD = 	$(top_builddir)/src/tinyxml/libtinyxml.a $(AM_LDFLAGS)
endif