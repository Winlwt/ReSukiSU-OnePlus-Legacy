# parashim.ko —— 必须在目标内核实树的同一个 out/ 目录里编译，
# 才能拿到匹配的 Module.symvers（否则 __versions 的 CRC 全是 0）。
#
# 本工作流里由 .github/workflows/parashim.yml 调用：
#   WORK=$(ls -d .work/*/ | head -1)
#   make -C $WORK/kernel/msm-5.4 O=$WORK/obj ARCH=arm64 ... M=$PWD/parashim modules

obj-m += parashim.o
