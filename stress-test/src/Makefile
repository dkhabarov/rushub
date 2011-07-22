#  files for make:
#
#  Conn.cpp Conn.h ConnBase.h ConnChoose.cpp ConnChoose.h ConnEpoll.cpp 
#  ConnEpoll.h ConnPoll.cpp ConnPoll.h ConnSelect.cpp ConnSelect.h HashTable.h
#  MeanFrequency.h Obj.cpp Obj.h Plugin.h Protocol.cpp Protocol.h Server.cpp 
#  Server.h Times.cpp Times.h

CXX = g++

CXXFLAGS = -O2 -Wall -Wextra -pipe
#CXXFLAGS = -g -Wall -Wextra -pipe

INCLUDE = -I/usr/include -I/usr/local/include

OBJS = $(CURDIR)/Conn.o $(CURDIR)/ConnChoose.o $(CURDIR)/ConnEpoll.o $(CURDIR)/ConnPoll.o $(CURDIR)/ConnSelect.o $(CURDIR)/main.o $(CURDIR)/Obj.o $(CURDIR)/Protocol.o $(CURDIR)/Server.o $(CURDIR)/Times.o

client: $(OBJS)
	$(CXX) -lrt -ldl -o client $(OBJS)

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

$(CURDIR)/main.o: $(CURDIR)/main.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Obj.o: $(CURDIR)/Obj.cpp $(CURDIR)/Obj.h $(CURDIR)/Times.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Protocol.o: $(CURDIR)/Protocol.cpp $(CURDIR)/Obj.h $(CURDIR)/Protocol.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Server.o: $(CURDIR)/Server.cpp $(CURDIR)/ConnBase.h $(CURDIR)/Times.h $(CURDIR)/HashTable.h $(CURDIR)/ConnChoose.h $(CURDIR)/Obj.h $(CURDIR)/MeanFrequency.h $(CURDIR)/ConnSelect.h $(CURDIR)/ConnEpoll.h $(CURDIR)/ConnPoll.h $(CURDIR)/Server.h $(CURDIR)/Protocol.h $(CURDIR)/Plugin.h $(CURDIR)/Conn.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

$(CURDIR)/Times.o: $(CURDIR)/Times.cpp $(CURDIR)/Times.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f $(OBJS) client