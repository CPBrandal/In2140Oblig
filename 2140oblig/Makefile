CFLAGS  = -std=gnu11 -g -Wall -Wextra

BIN =	create_fs_1 \
	create_fs_2 \
	create_fs_3 \
        load_fs \
	del_fs

#
# If you call "make VALGRIND=1 test" on the command line, all tests will be 
#
ifneq ("$(VALGRIND)", "")
  VALG = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x40 --free-fill=0x23
else
  VALG = 
endif

#
# Calling "make all" creates all of the programs listed in BIN
#
all: $(BIN)

create_fs_1: allocation.o inode.o create_fs_1.o
	gcc $(CFLAGS) $^ -o $@ -lm

create_fs_2: allocation.o inode.o create_fs_2.o
	gcc $(CFLAGS) $^ -o $@ -lm

create_fs_3: allocation.o inode.o create_fs_3.o
	gcc $(CFLAGS) $^ -o $@ -lm

load_fs: load_fs.o allocation.o inode.o
	gcc $(CFLAGS) $^ -o $@ -lm

del_fs: del_fs.o allocation.o inode.o
	gcc $(CFLAGS) $^ -o $@ -lm

%.o: %.c
	gcc $(CFLAGS) -c -I. $^ -o $@


#
# These are some tests. You can run all of them together by calling "make test".
# You can run all tests with Valgrind by calling "make VALGRIND=1 test".
# You can also run the individual tests with Valgrind, f.eks. by calling
# "make VALGRIND=1 test_create_fs_1".
#
test: test_load test_create test_del


#
# a load test may change the simulated disk; recreate it before load testing
#
prep_test_load:
	cp load_example1/master_file_table.bak load_example1/master_file_table
	cp load_example2/master_file_table.bak load_example2/master_file_table
	cp load_example3/master_file_table.bak load_example3/master_file_table
	cp load_example1/block_allocation_table.bak load_example1/block_allocation_table
	cp load_example2/block_allocation_table.bak load_example2/block_allocation_table
	cp load_example3/block_allocation_table.bak load_example3/block_allocation_table

test_load_fs_1: load_fs
	$(VALG) ./load_fs load_example1/master_file_table load_example1/block_allocation_table

test_load_fs_2: load_fs
	$(VALG) ./load_fs load_example2/master_file_table load_example2/block_allocation_table

test_load_fs_3: load_fs
	$(VALG) ./load_fs load_example3/master_file_table load_example3/block_allocation_table

test_load: prep_test_load test_load_fs_1 test_load_fs_2 test_load_fs_3


test_create_fs_1: create_fs_1
	$(VALG) ./create_fs_1 create_example1/master_file_table create_example1/block_allocation_table

test_create_fs_2: create_fs_2
	$(VALG) ./create_fs_2 create_example2/master_file_table create_example2/block_allocation_table

test_create_fs_3: create_fs_3
	$(VALG) ./create_fs_3 create_example3/master_file_table create_example3/block_allocation_table

test_create: test_create_fs_1 test_create_fs_2 test_create_fs_3


#
# every delete test changes the simulated disk; recreate it before load testing
#
prep_test_del:
	cp del_example1/master_file_table.bak del_example1/master_file_table
	cp del_example2/master_file_table.bak del_example2/master_file_table
	cp del_example3/master_file_table.bak del_example3/master_file_table
	cp del_example1/block_allocation_table.bak del_example1/block_allocation_table
	cp del_example2/block_allocation_table.bak del_example2/block_allocation_table
	cp del_example3/block_allocation_table.bak del_example3/block_allocation_table

test_del_fs_1: del_fs
	$(VALG) ./del_fs del_example1/master_file_table del_example1/block_allocation_table

test_del_fs_2: del_fs
	$(VALG) ./del_fs del_example2/master_file_table del_example2/block_allocation_table

test_del_fs_3: del_fs
	$(VALG) ./del_fs del_example3/master_file_table del_example3/block_allocation_table

test_del: prep_test_del test_del_fs_1 test_del_fs_2 test_del_fs_3


clean:
	rm -rf *.o
	rm -f $(BIN)

