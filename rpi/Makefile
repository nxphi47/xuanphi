# define the programs to make, and their objects and libraries

PROGS = record-sensors
LIBS =
TEST = test-rpigpio


record-sensors_OBJS = rpi-gpio.o rpi-cam.o record-sensors.o
record-sensors_LIBS =  -lopencv_core -lopencv_highgui

test-rpigpio_OBJS = rpi-gpio.o rpi-cam.o test-rpigpio.o
test-rpigpio_LIBS =  -lopencv_core -lopencv_highgui


# define additional things to install

INSTALL_EXTRA = 

# define any sub directories

LIBDIR = ./
SUBDIRS = 

# define additional CFLAGS and LDFLAGS

CFLAGS += -I./ `pkg-config opencv --cflags` -D__DEBUG__ -D__VER_2_CIRCUIT__
LDFLAGS = -L${LIBDIR} `pkg-config opencv --libs`
#CFLAGS += -I./ -D__DEBUG__ -D__VER_2_CIRCUIT__
#LDFLAGS = -L${LIBDIR} 

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
	${CC} -Wl,-rpath,${LIBDIR} ${LDFLAGS} ${$@_LIBS} -o $@ ${$@_OBJS} 

${TEST}: ${all_OBJS} ${all_SUBDIRS}
	${CC} -Wl,-rpath,${LIBDIR} ${LDFLAGS} ${$@_LIBS} -o $@ ${$@_OBJS} 

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
	rm -f *~ *.o ${PROGS} $(foreach lib, ${LIBS}, ${lib}.so* )
	rm -f .\#*

deps:
	$(foreach dir, ${SUBDIRS}, CHECKDEPS=../${CHECKDEPS} CHECKDEPS_SUBDIR=${dir} LIBDIR=${LIBDIR} make -w -C ${dir} deps ;)
	CHECKDEPS_TARGET="${PROGS} ${LIBS}" CHECKDEPS_SOURCE="${all_OBJS}" LIBDIR=${LIBDIR} ${CHECKDEPS}

deps-clean:
	rm -fR ${LIBDIR}/.subdir/${CHECKDEPS_SUBDIR}
	${foreach lib, ${LIBS}, rm -f ${LIBDIR}/${lib}.so*}

