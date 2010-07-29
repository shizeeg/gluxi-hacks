#!/bin/sh

VERSION_FILE="$1"

if [ -z "$VERSION_FILE" ]; then
	echo "No output file provided"
	exit 1
fi

GIT_VERSION=`git log -1 --date=iso --abbrev-commit | sed -e "s/commit //g" | head -n 1`
GIT_DATE=`git log -1 --date=iso | grep Date: | sed -e "s/Date:   //g"`

echo "Repository version: ${GIT_VERSION}"
 
cat > $VERSION_FILE <<EOF
// File is generated automatically on `date`. Don't edit.

#define GLUXI_VERSION "GIT $GIT_VERSION ($GIT_DATE)"

char* getGluxiVersion()
{
	return GLUXI_VERSION;
}

EOF
