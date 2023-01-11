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

if [ "$OS_FAMILY" = "REDHAT" ]; then
	echo "4.18"
	exit 0
elif [ "$OS_FAMILY" = "SUSE" ]; then
	echo "5.14"
	exit 0
elif [ "$OS_FAMILY" = "UBUNTU" ]; then
	if [ "$OSV_VERSION" = "20" ]; then
		echo "5.14"
		exit 0
	elif [ "$OSV_VERSION" = "22" ]; then
		echo "5.{15,17}"
		exit 0
	fi
fi


>&2 echo "Failed to assign proper kernel pattern for installation."
exit 1