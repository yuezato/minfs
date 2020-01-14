# minfs
なんか書く

# Motivation
+ Journaling of Journal Is (Almost) Free
  + https://www.usenix.org/node/179874
+ File systems unfit as distributed storage backends: lessons from 10 years of Ceph evolution
  + https://dl.acm.org/doi/10.1145/3341301.3359656

# 使い方
## 準備
`/dev/sdb`が破壊されても良いブロックデバイスであること。
Vagrantなりなんなりで準備すると良い。

```
$ make
$ gcc mkfs.minfs.c -o mkfs.minfs
```

以下は`sudo`でやること

## load module
```
insmod minfs.ko
```

## create mount point
```
mkdir -p /mnt/minfs
```

## format partition
```
./mkfs.minfs /dev/sdb
```

## mount filesystem
```
mount -t minfs /dev/sdb /mnt/minfs
```

## change to minfs root folder
```
cd /mnt/minfs
```

## create new file
```
touch b.txt && echo "OK. File created." || echo "NOT OK. File creation failed."
```

## unmount filesystem
```
cd ..
umount /mnt/minfs

popd > /dev/null 2>&1
```

## mount filesystem
```
mount -t minfs /dev/sdb /mnt/minfs
```

## check whether b.txt is still there
```
ls /mnt/minfs | grep b.txt && echo "OK. File b.txt exists " || echo "NOT OK. File b.txt does not exist."
```

## unmount filesystem
```
umount /mnt/minfs
```

## unload module
```
rmmod minfs
```
