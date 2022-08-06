project 			 := memoria

inc_dir 			 := ./include
src_dir 			 := ./src
sources 			 := $(wildcard ${src_dir}/*.c)

build_dir   		 := ./build
obj_dir 			 := ${build_dir}/obj
bin_dir 			 := ${build_dir}/bin
executable 		 := ${bin_dir}/${project}
build_dirs 		 := ${obj_dir} ${bin_dir}
objects 			 := $(subst .c,.o,$(subst ${src_dir},${obj_dir},${sources}))

shared_lib 		 := ../shared

LIBS = -lcommons -lshared
CC = gcc
CFLAGS = -g -Wall

ARGS = $(if $(prueba), ../pruebas/$(prueba)/$(project).cfg, config.cfg)

all: ${executable}

run: ${executable}
	@echo ${ARGS}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} ${ARGS}

valgrind: ${executable}
	@echo ${ARGS}
	@LD_LIBRARY_PATH="${shared_lib}/build/" valgrind --leak-check=full --track-origins=yes ${^} ${ARGS}

helgrind: ${executable}
	@echo ${ARGS}
	@LD_LIBRARY_PATH="${shared_lib}/build/" valgrind --tool=helgrind ${^} ${ARGS}

${obj_dir}/%.o: ${src_dir}/%.c | ${obj_dir}
	${CC} -I${inc_dir} -I"${shared_lib}/include" ${CFLAGS} -c $< -o $@

${executable}: ${objects} | ${bin_dir}
	${CC} -L"${shared_lib}/build" ${objects} ${CFLAGS} ${LIBS} -o $@

${build_dirs}:
	mkdir -p ${@}

clean:
	rm -rf ${build_dir}

# TESTS ===================================================================================

test_dir 			:= ./test
test_sources 		:= $(wildcard ${test_dir}/*.c)
test_build_dir 	:= ${build_dir}/test
test_obj_dir 		:= ${test_build_dir}/obj
test_bin_dir 		:= ${test_build_dir}/bin
test_objects 		:= $(subst .c,.o,$(subst ${test_dir},${test_obj_dir},${test_sources}))
test_dirs			:= ${test_obj_dir} ${test_bin_dir}

${test_dirs}:
	mkdir -p ${@}

${test_obj_dir}/%.o: ${test_dir}/%.c | ${test_dirs}
	${CC} -I${inc_dir} -I"${shared_lib}/src" ${CFLAGS} -c $< -o $@

build-tests: ${test_objects} | ${test_dirs}
	${CC} -L"${shared_lib}/build" ${test_objects} ${CFLAGS} ${LIBS} -o ${test_bin_dir}/tests

run-tests: ${test_dirs}
	LD_LIBRARY_PATH="${shared_lib}/build/" ${test_bin_dir}/tests

test: build-tests run-tests
