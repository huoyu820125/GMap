#makefile文件编译指令
#如果make文件名是makefile，直接使用make就可以编译
#如果make文件名不是makefile，比如test.txt，那么使用make -f test.txt

#————————————编译子模块————————————#
#方法一：(make -C subdir) 
#方法二：(cd subdir && make)
#加-w 可以查看执行指令时，当前工作目录信息

#————————————编译Exist————————————#
Output_lib_dir=./lib
Output_bin_dir=./bin
$(shell mkdir $(Output_lib_dir))
$(shell mkdir $(Output_bin_dir))

COMMON_DIR=./common
MDK_EX_DIR=./mdk_ex
MDK_DIR=./mdk

all: 
#	@echo "Create bin dir"
#	tar -zxvpf bin.tar.gz

	@echo "Complie common"
	(make -C $(COMMON_DIR) -w)
	@echo ""
	@echo "common Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie mdk_ex"
	(make -C $(MDK_EX_DIR) -w)
	@echo ""
	@echo "mdk_ex Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie mdk"
	(make -C $(MDK_DIR)/mdk_static -w)
	@echo ""
	@echo "mdk Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""


clean:
	(make -C $(COMMON_DIR) -w clean)
	(make -C $(MDK_EX_DIR) -w clean)
	(make -C $(MDK_DIR)/mdk_static -w clean)

