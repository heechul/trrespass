gbFile="/sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages"

mkdir /mnt/huge
mount -t hugetlbfs  -o pagesize=1G none /mnt/huge
su -c 'echo 1 >'$gbFile 
cat $gbFile
