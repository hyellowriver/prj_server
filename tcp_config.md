
1. 打开/etc/security/limits.conf，如果文件里没有这两行则加入，如果有参照下面这两行修改。

* soft nofile 65535
* hard nofile 65535
2. 修改/etc/pam.d/common-session如果没有下面这行则加入，如果有则取消注释。

session required pam_limits.so
3. 运行以下命令

echo "ulimit -SHn 65535" >> /etc/profile
然后重启服务器，重启后再运行ulimit -n 看是不是我们设置的65535.