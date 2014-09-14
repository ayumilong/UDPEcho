include Make.defines

PROGS =	 client server 

OPTIONS = -DUNIX  -DANSI


COBJECTS =	DieWithMessage.o AddressUtility.o
CSOURCES =	DieWithMessage.c  AddressUtility.c

CPLUSOBJECTS = 

COMMONSOURCES =

CPLUSSOURCES =

all:	${PROGS}


client:		UDPEchoClient.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(COMMONSOURCES) $(SOURCES)
		${CC} ${LINKOPTIONS}  $@ UDPEchoClient.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(LINKFLAGS) 


server:		UDPEchoServer.o $(CPLUSOBJECTS) $(COBJECTS)
		${CC} ${LINKOPTIONS} $@ UDPEchoServer.o $(CPLUSOBJECTS) $(COBJECTS) $(LIBS) $(LINKFLAGS)



.cc.o:	$(HEADERS)
	$(CPLUS) $(CPLUSFLAGS) $(OPTIONS) $<

.c.o:	$(HEADERS)
	$(CC) $(CFLAGS) $(OPTIONS) $<



backup:
	rm -f newUDPEcho.tar
	rm -f newUDPEcho.tar.gz
	tar -cf newUDPEcho.tar *
	gzip -f newUDPEcho.tar

clean:
		rm -f ${PROGS} ${CLEANFILES}

depend:
		makedepend UDPEchoClient.c UDPEchoServer.c $(CFLAGS) $(HEADERS) $(SOURCES) $(COMMONSOURCES) $(CSOURCES)
#		mkdep $(CFLAGS) $(HEADERS) $(SOURCES) $(COMMONSOURCES) $(CSOURCES)

# DO NOT DELETE

UDPEchoClient-Timeout.o: /usr/include/stdio.h /usr/include/features.h
UDPEchoClient-Timeout.o: /usr/include/libio.h /usr/include/_G_config.h
UDPEchoClient-Timeout.o: /usr/include/wchar.h /usr/include/xlocale.h
UDPEchoClient-Timeout.o: /usr/include/stdlib.h /usr/include/alloca.h
UDPEchoClient-Timeout.o: /usr/include/string.h /usr/include/unistd.h
UDPEchoClient-Timeout.o: /usr/include/getopt.h /usr/include/errno.h
UDPEchoClient-Timeout.o: /usr/include/signal.h /usr/include/time.h
UDPEchoClient-Timeout.o: /usr/include/netinet/in.h /usr/include/stdint.h
UDPEchoClient-Timeout.o: /usr/include/endian.h /usr/include/netdb.h
UDPEchoClient-Timeout.o: /usr/include/rpc/netdb.h Practical.h
UDPEchoServer-SIGIO.o: /usr/include/stdio.h /usr/include/features.h
UDPEchoServer-SIGIO.o: /usr/include/libio.h /usr/include/_G_config.h
UDPEchoServer-SIGIO.o: /usr/include/wchar.h /usr/include/xlocale.h
UDPEchoServer-SIGIO.o: /usr/include/stdlib.h /usr/include/alloca.h
UDPEchoServer-SIGIO.o: /usr/include/string.h /usr/include/unistd.h
UDPEchoServer-SIGIO.o: /usr/include/getopt.h /usr/include/fcntl.h
UDPEchoServer-SIGIO.o: /usr/include/time.h /usr/include/signal.h
UDPEchoServer-SIGIO.o: /usr/include/errno.h /usr/include/netdb.h
UDPEchoServer-SIGIO.o: /usr/include/netinet/in.h /usr/include/stdint.h
UDPEchoServer-SIGIO.o: /usr/include/endian.h /usr/include/rpc/netdb.h
UDPEchoServer-SIGIO.o: Practical.h
DieWithMessage.o: /usr/include/stdio.h /usr/include/features.h
DieWithMessage.o: /usr/include/libio.h /usr/include/_G_config.h
DieWithMessage.o: /usr/include/wchar.h /usr/include/xlocale.h
DieWithMessage.o: /usr/include/stdlib.h /usr/include/alloca.h
AddressUtility.o: /usr/include/stdio.h /usr/include/features.h
AddressUtility.o: /usr/include/libio.h /usr/include/_G_config.h
AddressUtility.o: /usr/include/wchar.h /usr/include/xlocale.h
AddressUtility.o: /usr/include/string.h /usr/include/arpa/inet.h
AddressUtility.o: /usr/include/netinet/in.h /usr/include/stdint.h
AddressUtility.o: /usr/include/endian.h
