AC_PREREQ([2.69])
AC_INIT([similar], [0.0.1], [pablo@sifflez.org])
AC_LANG(C++)
AC_CONFIG_SRCDIR([similar.cc])

AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

AC_CHECK_LIB(boost_system-gcc-mt, main, , [
  AC_CHECK_LIB(boost_system-mt, main, , [
    AC_CHECK_LIB(boost_system, main, , [
      AC_MSG_ERROR("Linking against boost::system library failed.")
    ])
  ])
])

AC_CHECK_LIB(boost_filesystem-gcc-mt, main, , [
  AC_CHECK_LIB(boost_filesystem-mt, main, , [
    AC_CHECK_LIB(boost_filesystem, main, , [
      AC_MSG_ERROR("Linking against boost::filesystem library failed.")
    ])
  ])
])

AC_CHECK_LIB(boost_graph-gcc-mt, main, , [
  AC_CHECK_LIB(boost_graph-mt, main, , [
    AC_CHECK_LIB(boost_graph, main, , [
      AC_MSG_ERROR("Linking against boost::graph library failed.")
    ])
  ])
])

AC_CHECK_LIB(xapian, main, , [
  AC_MSG_ERROR("Linking against xapian library failed.")])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

