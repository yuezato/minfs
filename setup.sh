make clean
make
make mkfs.minfs
make direct
./mkfs.minfs /dev/sdb
insmod minfs.ko
mount -t minfs /dev/sdb wks/
./lm.sh | grep minfs
rm -f /home/vagrant/shared/minfs.ko
cp minfs.ko /home/vagrant/shared
cp minfs.c /home/vagrant/shared
