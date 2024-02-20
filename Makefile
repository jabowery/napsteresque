#
# $Id: Makefile,v 1.55.4.10.2.24 2001/08/23 18:29:26 dbrumleve Exp $
#

NAPD_BASE	= ..

########################
### OS Configuration ###
########################

# solaris

# DEFINES	=
# OS_LIBS	= -lnsl -lsocket
# OS_FLAGS	= -DSOLARIS
# OS_INCLUDES	=

# linux

DEFINES		= -DLINUX -D_GNU_SOURCE -DUSE_THREADS \
		  -D_REENTRANT -DREENTRANT -D__USE_MALLOC=1 -DDEBUG \
		  -DPARANOIA=0 -DALL_FILES_SHAREABLE -DDWS
OS_LIBS		= -lpthread
OS_FLAGS	=
OS_INCLUDES	=


#########################
### SQL Configuration ###
#########################

SQL_BASE		= $(ORACLE_HOME)
SQL_INCLUDE_PATH	= -I$(SQL_BASE)/network/public \
			  -I$(SQL_BASE)/rdbms/public \
			  -I$(SQL_BASE)/rdbms/demo
SQL_LIB_PATH		= -L$(SQL_BASE)/lib 
#SQL_LIB			= -lclntsh 

SQLPP_BASE		=  /usr/local/dbora
SQLPP_INCLUDE_PATH	= -I$(SQLPP_BASE)
SQLPP_LIB_PATH		= -L$(SQLPP_BASE)
#SQLPP_LIB		= -ldbora

SQL_LIBS		= $(SQL_LIB_PATH) $(SQLPP_LIB_PATH) $(SQL_LIB) $(SQLPP_LIB) 
SQL_INCLUDES		= $(SQL_INCLUDE_PATH) $(SQLPP_INCLUDE_PATH)


#############################################
### Sleepycat (Berkeley) DB Configuration ###
#############################################

BDB_BASE		= /usr/local/BerkeleyDB.3.2
BDB_INCLUDES		= -I$(BDB_BASE)/include
BDB_LIB_PATH		= -L$(BDB_BASE)/lib 
BDB_LIB			= -ldb_cxx 
#BDB_LIBS		= $(BDB_LIB_PATH) $(BDB_LIB)


############################ 
### Secude Configuration ### 
############################

#SECUDE_BASE		= /usr/local/secude
#SECUDE_INCLUDES		= -I$(SECUDE_BASE)/include
#SECUDE_LIB_PATH		= -L$(SECUDE_BASE)
#SECUDE_LIBS		= $(SECUDE_LIB_PATH) -Wl,-rpath $(SECUDE_BASE) -lsecude

#############################
### General Configuration ###
#############################

CXX		= g++
PURIFY		= /usr/local/bin/purify -best-effort -g++

LIBS		= $(SQL_LIBS) $(LOG_LIBS) $(BDB_LIBS) $(OS_LIBS) $(SECUDE_LIBS) -L/usr/local/ssl/lib -lcrypto
#INCLUDES	= $(SQL_INCLUDES) $(BDB_INCLUDES) $(SECUDE_INCLUDES) \
#		  $(OS_INCLUDES) $(LOG_INCLUDES) -I/usr/local/ssl/include -I.
INCLUDES	= -I/usr/local/ssl/include

WARNING_FLAGS	= -Wall -Wno-switch -Wformat
CXXFLAGS	= $(DEFINES) $(WARNING_FLAGS) $(OS_FLAGS) \
		  -march=pentium -g -fexceptions


SRC	= channel.cc client.cc command.cc user.cc codes.cc pollset.cc \
	  file.cc functions.cc ignore.cc \
	  notify.cc remote.cc socket.cc address.cc \
	  scheduled.cc event.cc scheduler.cc search.cc server.cc ternary.cc \
	  db.cc cidclient.cc filecert.cc \
	  auth.cc elite.cc info.cc share.cc transfer.cc \
	  security.cc thread.cc mutex.cc data.cc time.cc

OBJ	= channel.o client.o command.o user.o codes.o pollset.o \
	  file.o functions.o ignore.o \
	  notify.o remote.o socket.o address.o \
	  scheduled.o event.o scheduler.o search.o server.o ternary.o \
	  db.o cidclient.o filecert.o \
	  auth.o elite.o info.o share.o transfer.o \
	  security.o thread.o mutex.o data.o time.o


HDR	= ban.hh build.hh channel.hh client.hh command.hh pollset.hh \
	  codes.hh db.hh defines.hh file.hh filecert.hh \
	  filedb.hh functions.hh ignore.hh \
	  notify.hh remote.hh event.hh scheduled.hh \
	  scheduler.hh search.hh server.hh tdp.hh \
	  ternary.hh security.hh thread.hh mutex.hh time.hh

SRCDIR	= .
BINDIR	= .

##################
### Make Rules ###
##################

all: napd napc

test:		napd
		@ ./napd -g -T
		@ echo OK

napd:		$(OBJ) napd.o
		$(CXX) $(CXXFLAGS) $(OBJ) napd.o $(LIBS) -o $@

napc:		$(OBJ) napc.o
		$(CXX) $(CXXFLAGS) $(OBJ) napc.o $(LIBS) -o $@

.cc.o:  
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.depends:
		@$(CXX) $(INCLUDES) -M $(SRC) > .depends

build.hh:
		@echo \#ifndef __NAP_BUILD_HH > .build
		@echo \#define __NAP_BUILD_HH 1 >> .build
		@echo \#define NAP_COMPILE_USER \"`whoami`\" >> .build
		@echo \#define NAP_COMPILE_HOST \"`hostname`\" >> .build
		@echo \#define NAP_COMPILE_TIME \"`date +%T`\" >> .build
		@echo \#define NAP_COMPILE_DATE \"`date +%x`\" >> .build
		@echo \#endif >> .build
		@mv -f .build build.hh

clean:
		rm -f $(OBJ) napd.o napc.o napd napc core build.hh *~

####################
### Dependencies ###
####################

-include .depends
