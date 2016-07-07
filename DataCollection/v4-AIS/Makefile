# define the programs to make, and their objects and libraries

PROGS = test-AIS-sensor rt-capture
LIBS =
TEST = 

rt-capture_OBJS = rt-capture.o comms.o sample.o hw.o bcm2835.o
rt-capture_LIBS = 

test-AIS-sensor_OBJS = test-AIS-sensor.o comms.o sample.o hw.o bcm2835.o
test-AIS-sensor_LIBS =

# define additional things to install

INSTALL_EXTRA = 

# define any sub directories

LIBDIR = ./
SUBDIRS = 

# define additional CFLAGS and LDFLAGS

#CFLAGS += -I./ `pkg-config --cflags opencv` -D__DEBUG__ -D__VER_2_CIRCUIT__
#LDFLAGS = -L${LIBDIR} `pkg-config --libs opencv`
CFLAGS += -I./ -D__DEBUG__ -D__VER_4_AIS_CIRCUIT__
LDFLAGS = -L${LIBDIR} 

# the following will be generic

all_SUBDIRS := \
	$(foreach dir, ${SUBDIRS}, ${LIBDIR}/.subdir/${dir})

all_OBJS := \
	$(foreach prog, ${PROGS}, ${${prog}_OBJS}) \
	$(foreach lib, ${LIBS}, ${${lib}_OBJS}) \
	$(foreach test, ${TEST}, ${${test}_OBJS})

install_PROGS := \
	$(foreach prog, ${PROGS}, ${PREFIX}/${prog}) 

install_LIBS := \
	$(foreach lib, ${LIBS}, ${PREFIX}/${lib}) 

install_SUBDIRS := \
	$(foreach dir, ${SUBDIRS}, ${PREFIX}/.libs/.subdir/${dir})


.PHONY: all test install ${SUBDIRS}

all: ${PROGS} ${LIBS} ${all_SUBDIRS}

test: ${TEST}

%.o : %.c
	$(CC) $(CFLAGS) ${$@_CFLAGS} -c $< -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) ${$@_CFLAGS} -c $< -o $@

${all_SUBDIRS}:
	mkdir -p ${@}
	LIBDIR=${LIBDIR} make -w -C $(notdir $@)
	
${PROGS}: ${all_OBJS} ${all_SUBDIRS}
	${CC} -o $@ ${$@_OBJS} -Wl,-rpath,${LIBDIR} ${LDFLAGS} ${$@_LIBS} 

${TEST}: ${all_OBJS} ${all_SUBDIRS}
	${CC} -o $@ ${$@_OBJS} -Wl,-rpath,${LIBDIR} ${LDFLAGS} ${$@_LIBS}

${LIBS}: ${all_OBJS} ${all_SUBDIRS}
	${CC} ${LDFLAGS} -shared -Wl,-soname,$@.so.0 -o $@.so.0.0 ${$@_OBJS}
	/sbin/ldconfig -n .
	ln -sf $@.so.0 $@.so
	cp $@.so* ${LIBDIR}/

${install_SUBDIRS}:
	mkdir -p ${@}
	LIBDIR=${LIBDIR} make -w -C $(notdir @) install

${install_PROGS}:
	${CC} -Wl,-rpath,${PREFIX}/.libs ${LDFLAGS} ${$(notdir $@)_LIBS} -o ${@} ${$(notdir $@)_OBJS}

${install_LIBS}:
	cp $(notdir $@).so* ${PREFIX}/.libs

install: ${install_SUBDIRS} ${install_PROGS} ${install_LIBS}
	$(foreach ex, ${INSTALL_EXTRA}, cp ${ex} ${PREFIX} ;) 
	rm -fR ${install_SUBDIRS}
	
uninstall:
	$(foreach dir, ${SUBDIRS}, make -w -C ${dir} uninstall ;)
	rm -fR ${install_PROGS} ${install_LIBS} $(foreach x, ${INSTALL_EXTRA}, ${PREFIX}/${x})

clean:
	$(foreach dir, ${SUBDIRS}, make -w -C ${dir} clean ;)
	rm -fR ${all_SUBDIRS} ${install_SUBDIRS}
	rm -f *~ *.o ${PROGS} ${TEST} $(foreach lib, ${LIBS}, ${lib}.so* )
	rm -f .\#*

deps:
	$(foreach dir, ${SUBDIRS}, CHECKDEPS=../${CHECKDEPS} CHECKDEPS_SUBDIR=${dir} LIBDIR=${LIBDIR} make -w -C ${dir} deps ;)
	CHECKDEPS_TARGET="${PROGS} ${LIBS}" CHECKDEPS_SOURCE="${all_OBJS}" LIBDIR=${LIBDIR} ${CHECKDEPS}

deps-clean:
	rm -fR ${LIBDIR}/.subdir/${CHECKDEPS_SUBDIR}
	${foreach lib, ${LIBS}, rm -f ${LIBDIR}/${lib}.so*}

