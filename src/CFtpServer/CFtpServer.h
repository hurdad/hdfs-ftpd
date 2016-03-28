/**@file
 * @brief      CFtpServer - Header
 *
 * @version    4.3 beta 1
 *
 * @date       February 2007
 *
 * @author     Julien Poumailloux (thebrowser@gmail.com)
 *
 * Copyright (C) 2007 Julien Poumailloux
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. If you like this software, a fee would be appreciated, particularly if
 *    you use this software in a commercial product, but is not required.
 */

#ifndef CFTPSERVER_H
#define CFTPSERVER_H

/// Disable warnings for deprecated C-style commands
#define _CRT_SECURE_NO_DEPRECATE

#include <ctime>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <iostream>

#ifdef CFTPSERVER_CONFIG_H_PATH
#include CFTPSERVER_CONFIG_H_PATH
#endif

#ifdef CFTPSERVER_ENABLE_ZLIB
#include CFTPSERVER_ZLIB_H_PATH
#endif

#ifdef __ECOS
#define USE_BSDSOCKETS
#define SOCKET int
#else
typedef int SOCKET;
#endif
#if defined(__CYGWIN__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#define __USE_FILE_OFFSET64
// Welcome to Cygwin 1.5.0: Starting to this version, off_t are automaticaly 64bits.
// *BSDs seem to natively use 64bits off_t.
#endif
typedef long long __int64;
#define MAX_PATH PATH_MAX
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#ifndef INADDR_NONE
#define INADDR_NONE	((unsigned long int) 0xffffffff)
#endif
#ifndef INADDR_ANY
#define INADDR_ANY	((unsigned long int) 0x00000000)
#endif

//HDFS Client Library Header
#include <hdfs.h>
#include <map>

/**
 * @brief CFtpServer class
 *
 * Runs a full featured ftp server with user control.
 * To run and stop the server use StartListening() and StopListening().
 *
 * Compiler specific settings can be set in CftpServerConfig.h
 */
class CFtpServer {
public:
	////////////////////////////////////////
	// [CONSTRUCTOR / DESTRUCTOR]
	////////////////////////////////////////

	CFtpServer();
	~CFtpServer();

	class CUserEntry;
	class CClientEntry;

	friend class CUserEntry;
	friend class CClientEntry;

private:
	class CEnumFileInfo;
	class CCriticialSection;

public:
	////////////////////////////////////////
	// START / STOP
	////////////////////////////////////////

	/**
	 * Ask the Server to Start Listening on the TCP-Port supplied by SetPort().
	 *
	 * @param  ulAddr  the Network Adress CFtpServer will listen on.
	 *		   Example:	INADDR_ANY for all local interfaces and inet_addr( "127.0.0.1" ) for the TCP Loopback interface.
	 *
	 * @param  usPort  the TCP-Port on which CFtpServer will listen.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error: the supplied Address or TCP-Port may not be valid.
	 */
	bool
	StartListening(unsigned long ulAddr, unsigned short int usPort);

	/**
	 * Ask the Server to Stop Listening.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	StopListening();

	/**
	 * Check if the Server is currently Listening.
	 *
	 * @return  "true"   if the Server is currently listening
	 *          "false"  otherwise.
	 */
	bool IsListening() const {
		return bIsListening;
	}

	/**
	 * Ask the Server to Start Accepting Clients.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	StartAccepting();

	/**
	 * Check if the Server is currently Accpeting Clients.
	 *
	 * @return  "true"   if the Server is currently accepting clients
	 *          "false"  otherwise.
	 */
	bool IsAccepting() const {
		return bIsAccepting;
	}

	////////////////////////////////////////
	// CONFIG
	////////////////////////////////////////

	/**
	 * Get the TCP Port on which CFtpServer will listen for incoming clients.
	 *
	 * @return  the TCP-Port.
	 */
	unsigned short GetListeningPort() const {
		return usListeningPort;
	}

	/**
	 * Set the TCP Port Range CFtpServer can use to Send and Receive Files or Data.
	 *
	 * @param  usStart  the First Port of the Range.
	 * @param  uiLen    the Number of Ports, including the First previously given.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	SetDataPortRange(unsigned short int usStart, unsigned short int uiLen);

	/**
	 * Get the TCP Port Range CFtpServer can use to Send and Receive Files or Data.
	 *
	 * @param  usStart  a ointer to the First Port.
	 * @param  usLen    a Pointer to the Number of Ports, including the First.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	GetDataPortRange(unsigned short int *usStart, unsigned short int *usLen);

	/**
	 * Set the time in which a user has to login.
	 *
	 * @param  ulSecond  the Timeout delay in Second. Set it to 0 if no timeout.
	 */
	void SetNoLoginTimeout(unsigned long int ulSecond) {
		ulNoLoginTimeout = ulSecond;
	}

	/**
	 * Get the no login Timeout.
	 *
	 * @return  the Timeout delay in Second.
	 */
	unsigned long int GetNoLoginTimeout() const {
		return ulNoLoginTimeout;
	}

	/**
	 * Set the no transfer ( list, download or upload ) Timeout.
	 *
	 * @param  ulSecond  the Timeout delay in Seconds. Set it to 0 if no timeout.
	 */
	void SetNoTransferTimeout(unsigned long int ulSecond) {
		ulNoTransferTimeout = ulSecond;
	}

	/**
	 * Get the no transfer ( list, download or upload ) Timeout.
	 *
	 * @return  the Timeout delay in Second.
	 */
	unsigned long int GetNoTransferTimeout() const {
		return ulNoTransferTimeout;
	}

	/**
	 * Set the delay the Server will wait when checking for the Client's pass.
	 *
	 * @param  ulMilliSecond  the Timeout delay in Milliseconds. Set it to 0 if no delay.
	 */
	void SetCheckPassDelay(unsigned int ulMilliSecond) {
		uiCheckPassDelay = ulMilliSecond;
	}

	/**
	 * Get the delay the Server will wait when checking for the Client's password.
	 *
	 * @return  the delay in millisecnds.
	 */
	unsigned int GetCheckPassDelay() const {
		return uiCheckPassDelay;
	}

	/**
	 * Set the max allowed password tries per client. After that, the client is disconnected.
	 *
	 * @param  uiMaxPassTries  the max allowed Password tries.
	 */
	void SetMaxPasswordTries(unsigned int uiMaxPassTries) {
		uiMaxPasswordTries = uiMaxPassTries;
	}

	/**
	 * Get the max allowed password tries per client.
	 *
	 * @return  the max allowed Password tries per client.
	 */
	unsigned int GetMaxPasswordTries() const {
		return uiMaxPasswordTries;
	}

	/**
	 * Enable or disable server-to-server transfer, better known as File eXchange Protocol.
	 * Disabling it will result in check when using the FTP PORT command.
	 * Enabling it make the server vulnerable to the FTP bounce attack.
	 * By default, FXP is disabled.
	 *
	 * @param  bEnable  true to enable, and false to disable.
	 */
	void EnableFXP(bool bEnable) {
		bEnableFXP = bEnable;
	}

	/**
	 * Check if server-to-server transfer is enabled.
	 *
	 * @return  "true"   if enabled,
	 *          "false"  otherwise.
	 */
	bool IsFXPEnabled() const {
		return bEnableFXP;
	}

	/**
	 * Set the size of the file transfer and directory listing buffer which
	 * will be allocated for each client.
	 * Default: (32 * 1024)
	 * Minimum: (MAX_PATH + 57)
	 * Maximum: Operating system dependent
	 *
	 * @param  uiSize   the transfer buffer size.
	 */
	void SetTransferBufferSize(unsigned int uiSize) {
		uiTransferBufferSize = uiSize;
	}

	/**
	 * Get the size of the file transfer and directory listing buffer which
	 * will be allocated for each client.
	 * By default, it will be equal to (32 * 1024).
	 *
	 * @return  the transfer buffer size.
	 */
	unsigned int GetTransferBufferSize() const {
		return uiTransferBufferSize;
	}

	/**
	 * Set the size of the file transfer and directory listing socket buffer which
	 * will be allocated for each client.
	 * Default: (64 * 1024)
	 * Minimum: It should be at least equal to the Transfer Buffer Size + 1.
	 * Maximum: Operating system dependent
	 *
	 * @param  uiSize   the transfer socket buffer size.
	 */
	void SetTransferSocketBufferSize(unsigned int uiSize) {
		uiTransferSocketBufferSize = uiSize;
	}

	/**
	 * Get the size of the file transfer and directory listing socket buffer which
	 * will be allocated for each client.
	 * By default, it will be equal to (64 * 1024).
	 *
	 * @return  the transfer socket buffer size.
	 */
	unsigned int GetTransferSocketBufferSize() const {
		return uiTransferSocketBufferSize;
	}

#ifdef CFTPSERVER_ENABLE_ZLIB
	/**
	 * Enable or disable data transfer compresion.
	 *
	 * @param  bEnable  true to enable, and false to disable.
	 */
	void EnableModeZ( bool bEnable )
	{	bEnableZlib = bEnable;}

	/**
	 * Check if data transfer compression is enabled.
	 *
	 * @return  "true"   if enabled,
	 *          "false"  otherwise.
	 */
	bool IsModeZEnabled() const
	{	return bEnableZlib;}
#endif

	////////////////////////////////////////
	// HDFS Connection Configuration
	////////////////////////////////////////

	/**
	 * Get/Set the HDFS NameNode Host
	 *
	 * @return  the hostname/ip address
	 */
	std::string GetHDFSNameNodeHost() const {
		return HDFSConfig.HDFSNameNodeHost;
	}
	void SetHDFSNameNodeHost(std::string NameNodeHost) {
		HDFSConfig.HDFSNameNodeHost = NameNodeHost;
	}

	/**
	 * Get/Set the HDFS NameNode Port
	 *
	 * @return  the port number
	 */
	int GetHDFSNameNodePort() const {
		return HDFSConfig.HDFSNameNodePort;
	}
	void SetHDFSNameNodePort(int NameNodePort)  {
		HDFSConfig.HDFSNameNodePort = NameNodePort;
	}

	/**
	 * Get/Set the HDFS BufferSize
	 *
	 * @return  count
	 */
	int GetHDFSBufferSize() const {
		return HDFSConfig.HDFSBufferSize;
	}
	void SetHDFSBufferSize(int BufferSize)  {
		HDFSConfig.HDFSBufferSize = BufferSize;
	}

	/**
	 * Get/Set the HDFS Replication
	 *
	 * @return  count
	 */
	unsigned short GetHDFSReplication() const {
		return HDFSConfig.HDFSReplication;
	}

	void SetHDFSReplication(unsigned short Replication ) {
		HDFSConfig.HDFSReplication = Replication;
	}

	/**
	 * Get/Set the HDFS Blocksize
	 *
	 * @return  count
	 */
	int GetHDFSBlockSize() const {
		return HDFSConfig.HDFSBlockSize;
	}
	void SetHDFSBlockSize(int BlockSize) {
		HDFSConfig.HDFSBlockSize = BlockSize;
	}

	////////////////////////////////////////
	// STATISTICS
	////////////////////////////////////////

	/**
	 * Get in real-time the number of Clients connected to the Server.
	 *
	 * @return  the current number of Clients connected to the Server.
	 */
	unsigned int GetNbClient() const {
		return uiNumberOfClient;
	}

	/**
	 * Get in real-time the number of existing Users.
	 *
	 * @return  the current number of existing Users.
	 */
	unsigned int GetNbUser() const {
		return uiNumberOfUser;
	}

	////////////////////////////////////////
	// EVENTS
	////////////////////////////////////////

	/**
	 * Enum the events that can be send to the events callbacks.
	 */
	enum eEvents {
		// User events
		NEW_USER,
		DELETE_USER,
		// Client events
		NEW_CLIENT,
		DELETE_CLIENT,
		CLIENT_DISCONNECT,
		CLIENT_AUTH,
		CLIENT_UPLOAD,
		CLIENT_DOWNLOAD,
		CLIENT_LIST,
		CLIENT_CHANGE_DIR,
		RECVD_CMD_LINE,
		SEND_REPLY,
		TOO_MANY_PASS_TRIES,
		NO_LOGIN_TIMEOUT,
		NO_TRANSFER_TIMEOUT,
		CLIENT_SOCK_ERROR,
		CLIENT_SOFTWARE,
		// Server event
		START_LISTENING,
		STOP_LISTENING,
		ERROR_LISTENING,
		START_ACCEPTING,
		STOP_ACCEPTING,
		MEM_ERROR,
		THREAD_ERROR,
		ZLIB_VERSION_ERROR,
		ZLIB_STREAM_ERROR,
		HDFS_CONNECT_ERROR
	};

	typedef void
	(*OnServerEventCallback_t)(int Event);
	typedef void
	(*OnUserEventCallback_t)(int Event, CFtpServer::CUserEntry *pUser, void *pArg);
	typedef void
	(*OnClientEventCallback_t)(int Event, CFtpServer::CClientEntry *pClient, void *pArg);

	/**
	 * Set the Server event callback.
	 *
	 * @param  pCallback  the Callback function.
	 */
	void SetServerCallback(OnServerEventCallback_t pCallback) {
		_OnServerEventCb = pCallback;
	}

	/**
	 * Set the User event callback.
	 *
	 * @param  pCallback  the Callback function.
	 */
	void SetUserCallback(OnUserEventCallback_t pCallback) {
		_OnUserEventCb = pCallback;
	}

	/**
	 * Set the Client event callback.
	 *
	 * @param  pCallback  the Callback function.
	 */
	void SetClientCallback(OnClientEventCallback_t pCallback) {
		_OnClientEventCb = pCallback;
	}

	/**
	 * Call the Server event callback.
	 *
	 * @param  Event  the Callback arguments.
	 */
	void OnServerEventCb(int Event) {
		if (_OnServerEventCb)
			_OnServerEventCb(Event);
	}

	/**
	 * Call the User event callback.
	 *
	 * @param  Event    the Callback arguments.
	 * @param  pUser  a pointer to the User class.
	 * @param  pArg     a pointer to something that depends on Event.
	 */
	void OnUserEventCb(int Event, CFtpServer::CUserEntry *pUser, void *pArg =
	NULL) {
		if (_OnUserEventCb)
			_OnUserEventCb(Event, pUser, pArg);
	}

	/**
	 * Call the Client event callback.
	 *
	 * @param  Event    the Callback arguments.
	 * @param  pClient  a pointer to the Client class.
	 * @param  pArg     a pointer to something that depends on Event.
	 */
	void OnClientEventCb(int Event, CFtpServer::CClientEntry *pClient, void *pArg =
	NULL) {
		if (_OnClientEventCb)
			_OnClientEventCb(Event, pClient, pArg);
	}

	////////////////////////////////////////
	// USER
	////////////////////////////////////////

	/**
	 * Enumerate the different Privileges a User can get.
	 */
	enum {
		READFILE = 0x1,
		WRITEFILE = 0x2,
		DELETEFILE = 0x4,
		LIST = 0x8,
		CREATEDIR = 0x10,
		DELETEDIR = 0x20
	};
#ifdef CFTPSERVER_ENABLE_EXTRACMD
	enum
	{
		ExtraCmd_EXEC = 0x1,
	};
#endif

	/**
	 * Create a new User.
	 *
	 * @param  pszLogin     the User Name.
	 * @param  pszPass      the User Password. Can be NULL.
	 * @param  pszStartDir  the User Start directory.
	 *
	 * @return on success  a pointer to the newly created User;
	 *         on error    NULL.
	 */
	CUserEntry *
	AddUser(const char *pszLogin, const char *pszPass, const char *pszStartDir);

	/**
	 * Delete a User, and by the way all the Clients connected to this User.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	DeleteUser(CFtpServer::CUserEntry *pUser);

private:
	////////////////////////////////////////
	// EVENTS
	////////////////////////////////////////

	OnServerEventCallback_t _OnServerEventCb;
	OnUserEventCallback_t _OnUserEventCb;
	OnClientEventCallback_t _OnClientEventCb;

	////////////////////////////////////////
	// CLASS CCriticialSection
	////////////////////////////////////////

	class CCriticialSection {
	public:
		bool Initialize() {
			pthread_mutex_init(&m_CS, NULL);
			return true;
		}
		bool Enter() {
			pthread_mutex_lock(&m_CS);
			return true;
		}
		bool Leave() {
			pthread_mutex_unlock(&m_CS);
			return true;
		}
		bool Destroy() {
			pthread_mutex_destroy(&m_CS);
			return true;
		}
	private:
		pthread_mutex_t m_CS;

	} FtpServerLock;

	////////////////////////////////////////
	// USER
	////////////////////////////////////////

	enum {
		MaxLoginLen = 16, MaxPasswordLen = 16, MaxRootPathLen = MAX_PATH
	};

	class CCriticialSection UserListLock;
	class CUserEntry *pFirstUser, *pLastUser;

	/**
	 * @warning  MUST lock the UserListLock before calling this function.
	 */
	CUserEntry *
	SearchUserFromLogin(const char *pszName);

	////////////////////////////////////////
	// CLIENT
	////////////////////////////////////////

	/**
	 * Add a new Client.
	 *
	 * @return  on success  a pointer to the new CClientEntry class,
	 *          on error    NULL.
	 */
	CFtpServer::CClientEntry *
	AddClient(SOCKET Sock, struct sockaddr_in *Sin);

	class CCriticialSection ClientListLock;
	class CClientEntry *pFirstClient, *pLastClient;

	////////////////////////////////////////
	// Network
	////////////////////////////////////////

	volatile SOCKET ListeningSock;

	struct {
		unsigned short int usLen, usStart;
	} DataPortRange;

	bool bIsListening;
	bool bIsAccepting;
	unsigned short int usListeningPort;

	pthread_t AcceptingThreadID;
	static void *
	StartAcceptingEx(void *pvParam);
	pthread_attr_t m_pattrServer;
	pthread_attr_t m_pattrClient;
	pthread_attr_t m_pattrTransfer;

	////////////////////////////////////////
	// FILE
	////////////////////////////////////////

	/**
	 * Simplify a Path.
	 */
	static bool
	SimplifyPath(char *pszPath);

	////////////////////////////////////////
	// STATISTIC
	////////////////////////////////////////

	unsigned int uiNumberOfUser;
	unsigned int uiNumberOfClient;

	////////////////////////////////////////
	// CONFIG
	////////////////////////////////////////

	unsigned int uiMaxPasswordTries;
	unsigned int uiCheckPassDelay;
	unsigned long int ulNoTransferTimeout, ulNoLoginTimeout;
	unsigned int uiTransferBufferSize, uiTransferSocketBufferSize;

#ifdef CFTPSERVER_ENABLE_ZLIB
	bool bEnableZlib;
#endif
	bool bEnableFXP;

	// HDFS Configuration
	struct {
		std::string HDFSNameNodeHost;
		unsigned short int HDFSNameNodePort;
		int HDFSBufferSize;
		unsigned short HDFSReplication;
		int HDFSBlockSize;
	} HDFSConfig;

};

//////////////////////////////////////////////////////////////////////
// CFtpServer::CUserEntry CLASS
//////////////////////////////////////////////////////////////////////
/**
 * @brief CFtpServer::CUserEntry class
 *
 * One instance of this class will be allocated for each user.
 */
class CFtpServer::CUserEntry {
public:

	CUserEntry();
	~CUserEntry() {
	}

	friend class CFtpServer;
	friend class CFtpServer::CClientEntry;

	/**
	 * Set the Privileges of a User.
	 *
	 * @param  ucPriv  the user's privileges separated by the bitwise inclusive binary operator "|".
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	SetPrivileges(unsigned char ucPriv);

	/**
	 * Get a User's privileges
	 *
	 * @return  The user's privileges concatenated with the bitwise inclusive binary operator "|".
	 */
	unsigned char GetPrivileges() const {
		return ucPrivileges;
	}

	/**
	 * Get the number of Clients logged-in as the User.
	 *
	 * @return  The number of Clients.
	 */
	unsigned int GetNumberOfClient() const {
		return uiNumberOfClient;
	}

	/**
	 * Set the maximum number of Clients which can be logged in as the User at the same time.
	 *
	 * @param  uiMax  the number of clients.
	 */
	void SetMaxNumberOfClient(unsigned int uiMax) {
		uiMaxNumberOfClient = uiMax;
	}

	/**
	 * Get the maximum number of Clients which can be logged in as the User at the same time.
	 *
	 * @return  The number of clients.
	 */
	unsigned int GetMaxClient() const {
		return uiMaxNumberOfClient;
	}

	/**
	 * Get a pointer to the User's Name.
	 *
	 * @return  A pointer to the User's Name.
	 */
	const char *
	GetLogin() const {
		return szLogin;
	}

	/**
	 * Get a pointer to the User's Password.
	 *
	 * @return	A pointer to the User's Password.
	 */
	const char *
	GetPassword() const {
		return szPassword;
	}

	/**
	 * Get a pointer to the User's Start Directory.
	 *
	 * @return  A pointer to the User's Start Directory.
	 */
	const char *
	GetStartDirectory() const {
		return szStartDirectory;
	}

#ifdef CFTPSERVER_ENABLE_EXTRACMD
	/**
	 * Set the supported Extra-Commands of a User.
	 *
	 * @param  dExtraCmd  the user's Extra-Commands concatenated with the bitwise inclusive binary operator "|".
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool SetExtraCommand( unsigned char dExtraCmd );

	/**
	 * Get the supported Extra-Commands of a User.
	 *
	 * @return  The user's Extra-Commands concatenated with the bitwise inclusive binary operator "|".
	 */
	unsigned char GetExtraCommand() const
	{	return ucExtraCommand;}
#endif

private:

#ifdef CFTPSERVER_ENABLE_EXTRACMD
	unsigned char ucExtraCommand;
#endif
	class CUserEntry *pPrevUser, *pNextUser;
	bool bDelete;
	bool bIsEnabled;
	unsigned char ucPrivileges;
	char szLogin[MaxLoginLen + 1];
	char szPassword[MaxPasswordLen + 1];
	char szStartDirectory[MaxRootPathLen + 1];
	unsigned int uiNumberOfClient, uiMaxNumberOfClient;
};

//////////////////////////////////////////////////////////////////////
// CFtpServer::CClientEntry CLASS
//////////////////////////////////////////////////////////////////////
/**
 * @brief CFtpServer::CClientEntry class
 *
 * One instance of this class will be allocated for each client.
 */
class CFtpServer::CClientEntry {
public:

	CClientEntry();
	~CClientEntry();

	friend class CFtpServer;

	/**
	 * Get a pointer to a in_addr structure representing the Client's IP.
	 *
	 * @return  A pointer to a in_addr structure.
	 */
	struct in_addr *
	GetIP() const {
		return (struct in_addr*) &ulClientIP;
	}

	/**
	 * Get a pointer to a in_addr structure representing the Server's IP the client is connected to.
	 *
	 * @return  A pointer to a in_addr structure.
	 */
	struct in_addr *
	GetServerIP() const {
		return (struct in_addr*) &ulServerIP;
	}

	/**
	 * Check if the client is logged-in.
	 *
	 * @return  "true"   if the client is logged in,
	 *          "false"  otherwise.
	 */
	bool IsLogged() const {
		return bIsLogged;
	}

	/**
	 * Check that the client has a or several privileges.
	 *
	 * @param  ucPriv  One or serveral privileges to check.
	 *
	 * @return  "true"   if the client has all the supplied privileges,
	 *          "false"  otherwise.
	 */
	bool
	CheckPrivileges(unsigned char ucPriv) const;

	/**
	 * Get a pointer to the Client's User.
	 *
	 * @return  a pointer to the client's user if the client is logged-in,
	 *          NULL otherwise.
	 */
	CUserEntry *
	GetUser() const {
		return (bIsLogged ? pUser : NULL);
	}

	/**
	 * Get the CWD ( Current Working Directory ) of the Client.
	 */
	char *
	GetWorkingDirectory() {
		return szWorkingDir;
	}

	/**
	 * Kick the Client. This is a non-blocking function.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error.
	 */
	bool
	InitDelete();

private:

	////////////////////////////////////////
	// SHELL
	////////////////////////////////////////

	volatile SOCKET CtrlSock;

	bool bIsLogged;

	void
	LogIn();
	void
	LogOut();

	void ResetTimeout() {
		time(&tTimeoutTime);
	}

	/// Enum the different Status a Client can have.
	enum Status_t {
		WAITING, LISTING, UPLOADING, DOWNLOADING
	}volatile eStatus;

	unsigned long ulDataIp;
	unsigned short usDataPort;

	char szWorkingDir[ MAX_PATH + 3 + 1];
	char szRenameFromPath[ MAX_PATH + 1];

	time_t tTimeoutTime;

	enum {
		MAX_COMMAND_LINE_LEN = MAX_PATH + 32
	};
	char sCmdBuffer[MAX_COMMAND_LINE_LEN + 1];
	char *pszCmdArg;
	char *psNextCmd;
	int iCmdLen;
	int iCmdRecvdLen;
	int nRemainingCharToParse;

	int
	ParseLine();
	bool
	ReceiveLine();

	unsigned int nPasswordTries;

	class CCriticialSection ClientLock;

	pthread_t ClientThreadID;
	static void* Shell(void *pvParam);

	enum {
		CMD_NONE = -1,
		CMD_QUIT,
		CMD_USER,
		CMD_PASS,
		CMD_NOOP,
		CMD_ALLO,
		CMD_SITE,
		CMD_HELP,
		CMD_SYST,
		CMD_STRU,
		CMD_MODE,
		CMD_TYPE,
		CMD_CLNT,
		CMD_PORT,
		CMD_PASV,
		CMD_LIST,
		CMD_NLST,
		CMD_CWD,
		CMD_XCWD,
		CMD_FEAT,
		CMD_MDTM,
		CMD_PWD,
		CMD_XPWD,
		CMD_CDUP,
		CMD_XCUP,
		CMD_STAT,
		CMD_ABOR,
		CMD_REST,
		CMD_RETR,
		CMD_STOR,
		CMD_APPE,
		CMD_STOU,
		CMD_SIZE,
		CMD_DELE,
		CMD_RNFR,
		CMD_RNTO,
		CMD_MKD,
		CMD_XMKD,
		CMD_RMD,
		CMD_XRMD,
		CMD_OPTS
	};

	////////////////////////////////////////
	// TRANSFER
	////////////////////////////////////////

	volatile SOCKET DataSock;

	int iZlibLevel;

	/// Enum the different Modes of Transfer.
	enum DataMode_t {
		STREAM, ZLIB
	} eDataMode;

	/// Enum the different Type of Transfer.
	enum DataType_t {
		ASCII, BINARY, ///< equals to IMAGE
		EBCDIC
	} eDataType;

	/// Enum the different Modes of Connection for transferring Data.
	enum DataConnection_t {
		NONE, PASV, PORT
	} eDataConnection;

	struct DataTransfer_t {
		CFtpServer::CClientEntry *pClient;
#ifdef __USE_FILE_OFFSET64
		__int64 RestartAt;
#else
		int RestartAt;
#endif
		hdfsFileInfo *fileAttr;
		int nCmd;
		SOCKET SockList;
		bool opt_a, opt_d, opt_F, opt_l;
#ifdef CFTPSERVER_ENABLE_ZLIB
		int iZlibLevel;
		z_stream zStream;
#endif
		DataMode_t eDataMode;
		DataType_t eDataType;

		char szPath[ MAX_PATH + 1];
		pthread_t TransferThreadID;

	} CurrentTransfer;

#ifdef CFTPSERVER_ENABLE_ZLIB
	bool InitZlib( DataTransfer_t *pTransfer );
#endif

	bool
	OpenDataConnection(int nCmd); ///< Open the Data Channel in order to transmit data.
	bool
	ResetDataConnection(bool bSyncWait = true); ///< Close the Data Channel.

	static void*
	RetrieveThread(void *pvParem);
	static void*
	StoreThread(void *pvParam);
	static void*
	ListThread(void *pvParam);

	////////////////////////////////////////
	// FILE
	////////////////////////////////////////

	bool
	SafeWrite(hdfsFile hFile, char *pBuffer, int nLen);

	/**
	 * Build a Full-Path using:
	 *
	 * @li  the Client's User Start Directory,
	 * @li  the Client's Working Directory,
	 * @li  the Client Command.
	 *
	 * @note  Call interally BuildVirtualPath() and store a pointer to the virtual path in pszVirtualPath.
	 */
	char*
	BuildPath(char* szAskedPath, char **pszVirtualPath = NULL);

	/**
	 * Build a Virtual-Path using:
	 *
	 * @li  the Client's Working Directory,
	 * @li  the Client Command.
	 */
	char*
	BuildVirtualPath(char* szAskedPath);

	/**
	 * Build a list line.
	 *
	 * @warning  psLine must be at least CFTPSERVER_LIST_MAX_LINE_LEN chars long.
	 */
	int
	GetFileListLine(char* psLine, hdfsFileInfo* attr, const char* pszName, bool opt_F);

	/**
	 * Copy the list line to a buffer, and send it to the client when full.
	 */
	bool
	AddToListBuffer(DataTransfer_t *pTransfer, char *pszListLine, int nLineLen, char *pBuffer,
			unsigned int *nBufferPos, unsigned int uiBufferSize);

	////////////////////////////////////////
	// USER LINK
	////////////////////////////////////////

	CFtpServer::CUserEntry *pUser;

	////////////////////////////////////////
	// LINKED LIST
	////////////////////////////////////////

	class CClientEntry *pPrevClient, *pNextClient;

	////////////////////////////////////////
	// OTHER
	////////////////////////////////////////

	unsigned long ulServerIP;
	unsigned long ulClientIP;

	bool bIsCtrlCanalOpen;
	class CFtpServer *pFtpServer;

	hdfsFS fs; //HDFS Connection

	typedef std::map<int, std::string> UserNames;
	typedef std::map<int, std::string> GroupNames;

	UserNames mUserNames; //QFS Users
	GroupNames mGroupNames; //QFS Groups

	/**
	 * Send a reply to the Client.
	 *
	 * @return  "true"   on success,
	 *          "false"  on error: The Socket may be invalid or the Connection may have been interrupted.
	 */
	bool
	SendReply(const char *pszReply, bool bNoNeedToAlloc = 0);

	/**
	 * Send a custom reply to the Client.
	 *
	 * @return  "true"   on success;
	 *          "false"  on error: The Socket may be invalid or the Connection may have been interrupted.
	 *
	 * @note  Call interally SendReply().
	 */
	bool
	SendReply2(const char *pszList, ...);
};

//////////////////////////////////////////////////////////////////////
// CFtpServer::CEnumFileInfo CLASS
//////////////////////////////////////////////////////////////////////

/**
 * Layer of abstraction used to list file on several Operating Systems.
 */
class CFtpServer::CEnumFileInfo {
public:
	CEnumFileInfo() {
		memset(this, 0x0, sizeof(CEnumFileInfo));
	}
	bool
	FindFirst(const char *pszPath);
	bool
	FindNext();
	bool
	FindClose();

	char *pszName;
	char szDirPath[ MAX_PATH + 1];
	char szFullPath[ MAX_PATH + 1];

	hdfsFS fs; //HDFS Connection
	hdfsFileInfo *fileAttr; //HDFS File Attributes
	int numAttr; //Number of Returned Attributes
	int indexAttr;
};

#endif // #ifdef CFTPSERVER_H
