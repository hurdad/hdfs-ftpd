dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(hdfs-ftpd, 1.0)

AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strncasecmp])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strstr]) 
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([sys/socket.h]) 
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADER_STDBOOL
AC_TYPE_OFF_T

AC_CHECK_LIB([config++], [main], [],[
         echo "libconfig++ library is required for this program"
         exit -1])
         
AC_CHECK_LIB([boost_program_options], [main], [],[
         echo "boost_program_options library is required for this program"
         exit -1])     

AC_CHECK_LIB([pthread], [main], [],[
         echo "pthread library is required for this program"
         exit -1])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CXX

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT
