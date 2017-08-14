做为一个开发人员，你肯定是遇到了too many open files这样的错误，这是由于系统同时打开文件的句柄数量超过了Linux系统所设置的最高值，系统默认为1024。我们需要修改这个参数来达到可同时打开更多的句柄。

如果你直接来到了这里，那么恭喜你，按照下面的方法操作即可。
如果你是看了别的文件又来到这里，可能你搜的方法不管用。那么按照下面的方法操作即可。

想要查看当前系统的最大可打开的句柄数可以这查看，运行以下命令：

ulimit -n
我们用以下方法来修改这个值。

1. 打开/etc/security/limits.conf，如果文件里没有这两行则加入，如果有参照下面这两行修改。

* soft nofile 65535
* hard nofile 65535
2. 修改/etc/pam.d/common-session如果没有下面这行则加入，如果有则取消注释。

session required pam_limits.so
3. 运行以下命令

echo "ulimit -SHn 65535" >> /etc/profile
然后重启服务器，重启后再运行ulimit -n 看是不是我们设置的65535.