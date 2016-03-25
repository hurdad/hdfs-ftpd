#define CFTPSERVER_CONFIG_H_PATH "CFtpServerConfig.h"
#include <boost/program_options.hpp>
#include "CFtpServer/CFtpServer.h"
#include "main.h"
#include "config.hpp"
#include "log.hpp"

using namespace ftpd_server;
using namespace std;
using namespace boost::program_options;

int main(int argc, char * argv[]) {

	string config;

	options_description desc("Options");
	desc.add_options()("help", "Options related to the program.")("config,c",
			value<string>(&config)->default_value("config/sample.cfg"), "Configuration File");

	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
		notify(vm);
	} catch (exception &e) {
		cout << e.what() << endl;
		return EXIT_FAILURE;
	}

	//print help
	if (vm.count("help")) {
		cout << desc << endl;
		return EXIT_SUCCESS;
	}

	//init config
	InitServerConfig();
	LoadServerConfig(config.c_str());

	//init ftp object
	CFtpServer FtpServer;

	//set logging call backs
	FtpServer.SetServerCallback(Log::OnServerEvent);
	FtpServer.SetUserCallback(Log::OnUserEvent);
	FtpServer.SetClientCallback(Log::OnClientEvent);

	//set hdfs connection
/*	FTPServer.SetHDFSNameNodeHost(server.HDFSNameNodeHost);
	FTPServer.SetHDFSNameNodePort(server.HDFSNameNodePort);

	//set hdfs options
	FTPServer.SetHDFSBufferSize(server.HDFSBufferSize);
	FTPServer.SetHDFSReplication(server.HDFSReplication);
	FTPServer.SetHDFSBlockSize(server.HDFSBlockSize);
*/

	//configuration
	FtpServer.SetMaxPasswordTries(server.MaxPasswordTries);
	FtpServer.SetNoLoginTimeout(server.NoLoginTimeout); // seconds
	FtpServer.SetNoTransferTimeout(server.NoTransferTimeout); // seconds
	FtpServer.SetDataPortRange(server.DataPortRange.usStart, server.DataPortRange.usLen); // data TCP-Port range = [100-999]
	FtpServer.SetCheckPassDelay(server.CheckPassDelay); // milliseconds. Bruteforcing protection.
	FtpServer.SetTransferBufferSize(server.TransferBufferSize); //buffer for directory listing
	FtpServer.SetTransferSocketBufferSize(server.TransferSocketBufferSize);
	FtpServer.EnableFXP(server.EnableFXP);

#ifdef CFTPSERVER_ENABLE_ZLIB
	FtpServer.EnableModeZ( true );
#endif

#ifdef CFTPSERVER_ENABLE_EXTRACMD // See "CFtpServer/config.h". not defined by default
	pUser->SetExtraCommand( CFtpServer::ExtraCmd_EXEC );
	// Security Warning ! Only here for example.
	// the last command allow the user to call the 'system()' C function!
#endif

	//Add Users From Config
	CFtpServer::CUserEntry *pUser;
	for (std::vector<User>::iterator it = server.Users.begin(); it != server.Users.end(); ++it) {
		User myUser = (*it);
		pUser = FtpServer.AddUser(myUser.Username.c_str(), myUser.Password.c_str(),
				myUser.HomePath.c_str());
		if (pUser)
			pUser->SetPrivileges(myUser.Privs);
	}

	//Start listening
	if (FtpServer.StartListening(inet_addr(server.ListeningIP.c_str()), server.ListeningPort)) {

		if (FtpServer.StartAccepting()) {
			printf("-Listening.\r\n");

			//loop
			for (;;)
				sleep(1);

		}

		//shutdown
		FtpServer.StopListening();

	} else {
		Log::OnServerEvent(CFtpServer::ERROR_LISTENING);
	}

	return 0;

}
