#!/bin/sh

if [ "$OS_TYPE" = rhel_8 ]; then
	OS_FAMILY=REDHAT
	OSV_VERSION=$(echo $OS_VERSION | cut -f1 -d '.')
	OSV_PATCH=$(echo $OS_VERSION | cut -f2 -d '.')
	OSV_PATCH_MINOR="0"
elif [ "$OS_TYPE" = sles ]; then
	OS_FAMILY=SUSE
	OSV_VERSION=$(echo $OS_VERSION | cut -c1-2)
	OSV_PATCH=$(echo $OS_VERSION | cut -c5)
	OSV_PATCH_MINOR="0"
elif [ "$(echo $OS_TYPE | cut -c1-6)" = ubuntu ]; then
	OS_FAMILY=UBUNTU
	OS_VERSION_STRING=$(echo $OS_TYPE | cut -f2 -d '_')
	OSV_VERSION=$(echo $OS_VERSION_STRING | cut -f1 -d '.')
	OSV_PATCH=$(echo $OS_VERSION_STRING | cut -f2 -d '.')
	OSV_PATCH_MINOR="0"
else
	>&2 echo "Failed to recognize version.\nPlease set correct OS_TYPE and/or OS_VERSION.\n\nGot:\nOS_TYPE=$OS_TYPE\nOS_VERSION=$OS_VERSION\n"
	exit 1
fi

echo "#define OSV_VERSION $OSV_VERSION"
echo "#define OSV_PATCH $OSV_PATCH"
echo "#define OSV_PATCH_MINOR $OSV_PATCH_MINOR"
echo "#define OS_FAMILY_$OS_FAMILY"
