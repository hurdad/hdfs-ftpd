hdfs-ftpd
========

Hadoop HDFS Backed C++ FTP Daemon

FTP Daemon core using CFtpServer Library [http://sourceforge.net/projects/cftpserver/]

Gettting Started
========
### Requirements:

* Hadoop HDFS Cluster

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
yum install hadoop-libhdfs hadoop-libhdfs-devel boost-devel libconfig-devel java-1.8.0-openjdk-devel gcc-c++ automake git
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
####################################################
# sample config file for hdfs-ftpd v0.1
# 
#####################################################

HDFS: //HDFS Connection and Configuration
{        
    NameNodeHost 	= "default"; //Default [uses fs.default.name]
    NameNodePort 	= 0;		//Default (8020) 
    BufferSize 		= 0; 		//Default
    Replication		= 0;		//Default
    BlockSize		= 0;		//Default 
};

//FTP
ListeningIP 		= "127.0.0.1";  // FTP Listening Interface
ListeningPort 		= 2100; 		// By default, the FTP control port is 21
CheckPassDelay 		= 500; 			// milliseconds. Bruteforcing protection.
EnableFXP 			= true; 		// Server to Server transfers
MaxPasswordTries 	= 3;
NoLoginTimeout 		= 45; 			// Seconds
NoTransferTimeout 	= 90; 			// Seconds

//FTP Users
Users:(
	{
		Username : "admin";
		Password : "admin";
		HomePath : "/";
		Privs    : 63; // READFILE | WRITEFILE | DELETEFILE | LIST | CREATEDIR | DELETEDIR
	},
	{
		Username : "anonymous";
		Password : "";
		HomePath : "/";
		Privs    : 9; // READFILE | LIST 
	}
);

//TCP Port ranges for transfers
DataPortRange: 
{ 
	Start = 10000;
	Length = 900;
};

//TCP Socket Buffer
TransferSocketBufferSize = 65536; //64k

//Dir List Buffer Size
TransferBufferSize = 32768; //32kB

//Logging
LogDirectory 		= "logs";
EnableUserLogging 	= true; //Add & Removing Users
EnableClientLogging = true; //Client Operations
EnableServerLogging = true; //General Server 
```
