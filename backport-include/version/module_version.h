#define xstr(a) str(a)
#define str(a) #a

#ifndef DISTRO_NAME
#error "You need a DISTRO_NAME defined"
#endif

#ifndef KERNEL_VERSION_NAME
#error "You need a KERNEL_VERSION_NAME defined"
#endif

#ifndef CSE_BACKPORT_VERSION
#error "You need a CSE_BACKPORT_VERSION defined"
#endif

#define BACKPORT_MODULE_VER "backported to " xstr(DISTRO_NAME) " " xstr(KERNEL_VERSION) " using backports version " xstr(CSE_BACKPORT_VERSION)