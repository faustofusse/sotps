project 			 := consola

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

ARGS = 43434 ./test/archivo.txt

all: ${executable}

run: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} ${ARGS}

valgrind: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" valgrind --leak-check=full --track-origins=yes ${^} ${ARGS}

helgrind: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" valgrind --tool=helgrind ${^} ${ARGS}

${obj_dir}/%.o: ${src_dir}/%.c | ${obj_dir}
	${CC} -I${inc_dir} -I"${shared_lib}/include" ${CFLAGS} -c $< -o $@

${executable}: ${objects} | ${bin_dir}
	${CC} -L"${shared_lib}/build" ${objects} ${CFLAGS} ${LIBS} -o $@

${build_dirs}:
	mkdir -p ${@}

clean:
	rm -rf ${build_dir}

# PRUEBAS ===================================================================================

prueba_dir			:= ../pruebas
time_between_processes := 0.1

prueba-base: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 256 ${prueba_dir}/base/BASE_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 256 ${prueba_dir}/base/BASE_2 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 256 ${prueba_dir}/base/BASE_2 &

prueba-planificacion: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 512 ${prueba_dir}/planificacion/PLANI_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 512 ${prueba_dir}/planificacion/PLANI_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 512 ${prueba_dir}/planificacion/PLANI_2 &

prueba-suspension: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 1024 ${prueba_dir}/suspension/SUSPE_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 1024 ${prueba_dir}/suspension/SUSPE_2 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 1024 ${prueba_dir}/suspension/SUSPE_3 &

prueba-memoria: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 4096 ${prueba_dir}/memoria/MEMORIA_1

prueba-tlb: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/tlb/TLB_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/tlb/TLB_2 &

prueba-integral: ${executable}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/integral/INTEGRAL_1 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/integral/INTEGRAL_2 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/integral/INTEGRAL_3 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/integral/INTEGRAL_4 &
	sleep ${time_between_processes}
	@LD_LIBRARY_PATH="${shared_lib}/build/" ${^} 2048 ${prueba_dir}/integral/INTEGRAL_5 &

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
