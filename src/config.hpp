#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include "libconfig.h++"
#include <iostream>
#include <string>

namespace ftpd_server {

using namespace std;
using namespace libconfig;

config server;

void InitServerConfig() {

	server.ListeningIP = "0.0.0.0"; // FTP Listening Interface
	server.ListeningPort = 2100; // By default, the FTP control port is 21

	server.HDFSNameNodeHost = "127.0.0.1"; //HDFS Namenode Hostname
	server.HDFSNameNodePort = 50010; //HDFS Namenode Port
	server.HDFSBufferSize = 0; //default
	server.HDFSReplication = 0; //default
	server.HDFSBlockSize = 0; //default

	server.CheckPassDelay = 500; // milliseconds. Bruteforcing protection.
	server.DataPortRange.usStart = 100; // TCP Ports [100;999].
	server.DataPortRange.usLen = 900;
	server.EnableFXP = true;
	server.MaxPasswordTries = 3;
	server.NoLoginTimeout = 0; // No timeout.
	server.NoTransferTimeout = 0; // No timeout.
	server.TransferSocketBufferSize = 64 * 1024; //64kB
	server.TransferBufferSize = 32 * 1024; //32kB

	server.LogDirectory = "logs";
	server.EnableUserLogging = false;
	server.EnableClientLogging = true;
	server.EnableServerLogging = true;

}

void LoadServerConfig(const char * configFile) {
	Config cfg;

	// Read the file. If there is an error, report it and exit.
	try {
		cfg.readFile(configFile);
	} catch (const FileIOException &fioex) {
		std::cerr << "Error while reading config file: " << configFile << std::endl;
		exit (EXIT_FAILURE);
	} catch (const ParseException &pex) {
		std::cerr << "Configuration parse error at " << pex.getFile() << ":" << pex.getLine()
				<< " - " << pex.getError() << std::endl;
		exit (EXIT_FAILURE);
	}

	//
	// Parse Config
	//
/*
	//HDFS
	cfg.lookupValue("HDFS.NameNodeHost", server.HDFSNameNodeHost);
	cfg.lookupValue("HDFS.NameNodePort", server.HDFSNameNodePort);
	cfg.lookupValue("HDFS.BufferSize", server.HDFSBufferSize);
	cfg.lookupValue("HDFS.Replication", server.HDFSReplication);
	cfg.lookupValue("HDFS.BlockSize", server.HDFSBlockSize);


	//QFS
	/*
	cfg.lookupValue("QFS.MetaServerHost", server.QFSMetaServerHost);
	int QFSMetaServerPort = server.QFSMetaServerPort;
	cfg.lookupValue("QFS.MetaServerPort", QFSMetaServerPort);
	server.QFSMetaServerPort = QFSMetaServerPort;

	cfg.lookupValue("QFS.MaxRetryPerOp", server.QFSMaxRetryPerOp);
	cfg.lookupValue("QFS.RetryDelay", server.QFSRetryDelay);
	cfg.lookupValue("QFS.QFSDefaultIOTimeout", server.QFSDefaultIOTimeout);

	cfg.lookupValue("QFS.Replication.StripeSize", server.QFSReplicationStripeSize);
	cfg.lookupValue("QFS.Replication.NumStripes", server.QFSReplicationNumStripes);
	cfg.lookupValue("QFS.Replication.NumRecoveryStripes", server.QFSReplicationNumRecoveryStripes);
	cfg.lookupValue("QFS.Replication.NumReplicas", server.QFSReplicationNumReplicas);

	cfg.lookupValue("QFS.SkipHoles", server.QFSSkipHoles);
	cfg.lookupValue("QFS.ReadBufferSize", server.QFSReadBufferSize);
	cfg.lookupValue("QFS.WriteBufferSize", server.QFSWriteBufferSize);
	cfg.lookupValue("QFS.ReadAheadBufferSize", server.QFSReadAheadBufferSize);
	*/

	//FTP
	cfg.lookupValue("ListeningIP", server.ListeningIP);
	int ListeningPort = server.ListeningPort;
	cfg.lookupValue("ListeningPort", ListeningPort);
	server.ListeningPort = ListeningPort;

	int Start = server.DataPortRange.usStart;
	int Len = server.DataPortRange.usLen;
	cfg.lookupValue("DataPortRange.Start", Start);
	cfg.lookupValue("DataPortRange.Len", Len);
	server.DataPortRange.usStart = Start;
	server.DataPortRange.usLen = Len;

	cfg.lookupValue("EnableFXP", server.EnableFXP);
	cfg.lookupValue("MaxPasswordTries", server.MaxPasswordTries);

	int NoLoginTimeout = server.NoLoginTimeout;
	int NoTransferTimeout = server.NoTransferTimeout;
	cfg.lookupValue("NoLoginTimeout", NoLoginTimeout);
	cfg.lookupValue("NoTransferTimeout", NoTransferTimeout);
	server.NoLoginTimeout = NoLoginTimeout;
	server.NoTransferTimeout = NoTransferTimeout;

	cfg.lookupValue("TransferBufferSize", server.TransferBufferSize);
	cfg.lookupValue("TransferSocketBufferSize", server.TransferSocketBufferSize);

	Setting& Users = cfg.lookup("Users");
	int UserCount = Users.getLength();
	for (int i = 0; i < UserCount; i++) {
		User newUser;
		newUser.Username = string((const char *)Users[i]["Username"]);
		newUser.Password = string((const char *)Users[i]["Password"]);
		newUser.HomePath = string((const char *)Users[i]["HomePath"]);
		int privs =  Users[i]["Privs"];
		newUser.Privs = (unsigned char) privs <= 63 ? privs : 0;
		server.Users.push_back((newUser));
    }
	cfg.lookupValue("LogDirectory", server.LogDirectory);
	cfg.lookupValue("EnableUserLogging", server.EnableUserLogging);
	cfg.lookupValue("EnableClientLogging", server.EnableClientLogging);
	cfg.lookupValue("EnableServerLogging", server.EnableServerLogging);

}

}
#endif /* CONFIG_HPP_ */
