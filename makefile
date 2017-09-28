#makefile文件编译指令
#如果make文件名是makefile，直接使用make就可以编译
#如果make文件名不是makefile，比如test.txt，那么使用make -f test.txt

#————————————编译子模块————————————#
#方法一：(make -C subdir) 
#方法二：(cd subdir && make)
#加-w 可以查看执行指令时，当前工作目录信息

#————————————编译Exist————————————#

PROTOCL_DIR=./cpp/protocl
SDK_DIR=./cpp/sdk
GSRC_DIR=./cpp/gsrc
GMAP_DIR=./cpp/gmap

all: 
#	@echo "Create bin dir"
#	tar -zxvpf bin.tar.gz

	@echo "Complie protocl"
	(make -C $(PROTOCL_DIR) -w)
	@echo ""
	@echo "protocl Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie sdk"
	(make -C $(SDK_DIR) -w)
	@echo ""
	@echo "sdk Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie gsrc"
	(make -C $(GSRC_DIR) -w)
	@echo ""
	@echo "gsrc Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "Complie gmap"
	(make -C $(GMAP_DIR) -w)
	@echo ""
	@echo "gmap Complie finished"
	@echo ""
	@echo ""
	@echo ""
	@echo ""


clean:
	(make -C $(PROTOCL_DIR) -w clean)
	(make -C $(SDK_DIR) -w clean)
	(make -C $(GSRC_DIR) -w clean)
	(make -C $(GMAP_DIR) -w clean)

