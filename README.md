# hdfs-ftpd
========

HDFS Backed C++ FTP Daemon

FTP Daemon core using CFtpServer Library[http://sourceforge.net/projects/cftpserver/]

Gettting Started
========
### Requirements:

* hdfs cluster [namenode host + port]

### Shared Library Requirements:
* libhdfs + libjvm
* libboost_program_options
* libconfig++ 1.4.9=+

Quick Start with Cloudera CDH 5x + CentOS 7
========
```
sudo su root
cd /etc/yum.repo.d/
wget https://archive.cloudera.com/cdh5/redhat/7/x86_64/cdh/cloudera-cdh5.repo
yum update
yum install hadoop-libhdfs hadoop-libhdfs-devel boost-devel libconfig-devel java-1.8.0-openjdk-headless
echo '/usr/lib/jvm/java/jre/lib/amd64/server/' > /etc/ld.so.conf.d/java.conf 
echo '/usr/lib/hadoop/lib/native/' > /etc/ld.so.conf.d/hadoop.conf 
ldconfig
exit

cd ~
git clone https://github.com/hurdad/hdfs-ftpd.git
cd hdfs-ftpd
sh autogen.sh
./configure
make
sh setclasspath.sh
src/hdfsftpd -c config/sample.cfg
```
Configuration
========
```
$ src/hdfsftpd --help
Options:
  --help                                Options related to the program.
  -c [ --config ] arg (=config/sample.cfg)
                                        Configuration File
```
