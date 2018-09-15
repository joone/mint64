# file		makefile
# date		2008/11/12
# author	kkamagui 
# brief		OS 이미지를 빌드하기 위한 make 파일

# 기본적으로 빌드를 수행할 목록
all: BootLoader Kernel32 Kernel64 Utility Disk.img

# 부트 로더 빌드를 위해 부트 로더 디렉터리에서 make 실행
BootLoader:
	@echo 
	@echo ============== Build Boot Loader ===============
	@echo 
	
	make -C 00.BootLoader

	@echo 
	@echo =============== Build Complete ===============
	@echo 
	
# 보호 모드 커널 이미지를 빌드하기 위해 보호 모드 디렉터리에서 make 실행
Kernel32:
	@echo 
	@echo ============== Build 32bit Kernel ===============
	@echo 
	
	make -C 01.Kernel32

	@echo 
	@echo =============== Build Complete ===============
	@echo 

# IA-32e 모드 커널 이미지를 빌드하기 위해 IA-32e 모드 디렉터리에서 make 실행
Kernel64:
	@echo 
	@echo ============== Build 64bit Kernel ===============
	@echo 
	
	make -C 02.Kernel64

	@echo 
	@echo =============== Build Complete ===============
	@echo 

	
# OS 이미지 생성
Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo 
	@echo =========== Disk Image Build Start ===========
	@echo 

	./04.Utility/00.ImageMaker/ImageMaker $^
	chmod 777 Disk.img

	@echo 
	@echo ============= All Build Complete =============
	@echo 
	
# 유틸리티 빌드
Utility:
	@echo 
	@echo =========== Utility Build Start ===========
	@echo 

	make -C 04.Utility

	@echo 
	@echo =========== Utility Build Complete ===========
	@echo 
	
	
# 소스 파일을 제외한 나머지 파일 정리	
clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	make -C 04.Utility clean
	rm -f Disk.img
