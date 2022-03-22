linux_source=$1

if [ -z $linux_source ]; then
  echo "Enter path to Linux kernel source as argument"
else
  make -C $linux_source ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$PWD modules
fi
