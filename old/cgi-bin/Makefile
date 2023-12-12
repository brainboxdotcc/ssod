# Insp Main Makefile
#
# (C) ChatSpike development team
# Makefile by <Craig@ChatSpike.net>
# Small modifications by <brain@ChatSpike.net>
#

CXX = g++

all: ssod.cgi ssod-admin.cgi

ssod.cgi: Main.cpp
	$(CXX) -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -lgd -Wno-deprecated -Wno-write-strings $^ -o $@
	chmod ug+rx ssod.cgi
ssod-admin.cgi: ssod-admin.cpp
	$(CXX) -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -lgd -Wno-deprecated -Wno-write-strings $^ -o admin/$@
	chmod ug+rx admin/ssod-admin.cgi

