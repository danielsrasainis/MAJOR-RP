
// MAJOR RP: the prebuilt closed-source net.dll / net.so carry MTA's own nightly
// version string (e.g. 1.7.0-7.27002.0), which never equals this fork's version.
// Server and network library are always shipped together from the same build,
// so skip the string comparison (supported switch, see Server/core/CDynamicLibrary.cpp).
#define MTASA_SKIP_VERSION_CHECKS
