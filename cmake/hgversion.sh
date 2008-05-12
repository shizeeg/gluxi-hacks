#!/bin/sh

VERSION_FILE="$1"

if [ -z "$VERSION_FILE" ]; then
	echo "No output file provided"
	exit 1
fi

HG_VERSION=`hg tip --template "r{rev}:{node|short} ({date|isodate})\n"`
echo "Repository version: ${HG_VERSION}"

cat > $VERSION_FILE <<EOF
// File is generated automatically on `date`. Don't edit.

#define GLUXI_VERSION "HG $HG_VERSION"

char* getGluxiVersion()
{
	return GLUXI_VERSION;
}

EOF
