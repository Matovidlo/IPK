##################################
##### Martin Vaško xvasko12  #####
####  FIT VUTBR 2.BIT->2.BIB  ####
### IPK -> 1.project, RESTapi  ###
## Date of creation = 24.2.2016 ##
# Makefile usage = make,make all #
##################################

CPP=g++
CPPFLAG=-std=c++11 -pedantic -Wall -g
EXECUTABLE=ftrest ftrestd

.PHONY: all clean xvasko12.tgz

all: ftrestd ftrest

#ftrestd.h
ftrest: ftrest.cc
	$(CPP) $(CPPFLAG) $^ -o $@

#ftrestd.h
ftrestd: ftrestd.cc
	$(CPP) $(CPPFLAG) $^ -o $@

clean:
	$(RM) $(EXECUTABLE) xvasko12.tgz readme.html

markdown:
	markdown -o readme.html readme.md

xvasko12.tgz: $(wildcard *.cc) $(wildcard *.h) Makefile readme.md
	tar cvzf $@ $^
