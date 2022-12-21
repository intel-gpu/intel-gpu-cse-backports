#include <generated_osv_version.h>

#if OSV_VERSION > 0
#	define OSV_RELEASE_CODE (((OSV_VERSION) << 16) + ((OSV_PATCH) << 8) + (OSV_PATCH_MINOR))
#else
#	define OSV_RELEASE_CODE 0
#endif

#ifdef OS_FAMILY_SUSE
#	define SUSE_RELEASE_CODE OSV_RELEASE_CODE
#	define SUSE_RELEASE_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#	define SUSE_RELEASE_VERSION_GEQ(a,b,c) SUSE_RELEASE_CODE >= SUSE_RELEASE_VERSION(a,b,c)
#else
#	define SUSE_RELEASE_CODE 0
#	define SUSE_RELEASE_VERSION(a,b,c) 0
#	define SUSE_RELEASE_VERSION_GEQ(a,b,c) 0
#endif

#ifdef OS_FAMILY_REDHAT
#	define REDHAT_RELEASE_CODE OSV_RELEASE_CODE
#	define REDHAT_RELEASE_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#else
#	define REDHAT_RELEASE_CODE 0
#	define REDHAT_RELEASE_VERSION(a,b,c) 0
#endif

#ifdef OS_FAMILY_UBUNTU
#	define UBUNTU_RELEASE_CODE OSV_RELEASE_CODE
#	define UBUNTU_RELEASE_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#else
#	define UBUNTU_RELEASE_CODE 0
#	define UBUNTU_RELEASE_VERSION(a,b,c) 0
#endif