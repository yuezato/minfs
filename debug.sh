echo ttyS1,115200 | tee /sys/module/kgdboc/parameters/kgdboc
echo g | tee /proc/sysrq-trigger
