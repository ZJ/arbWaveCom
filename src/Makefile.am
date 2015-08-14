SUBDIRS = defOptions genBinary .

bin_PROGRAMS = awgcom

awgcom_SOURCES = driver.c genBinary/genBinary.h defOptions/defOptions.h 
awgcom_LDADD = defOptions/libdefoptions.a genBinary/libgenbinary.a
awgcom_LDFLAGS = @mingwldflags@
