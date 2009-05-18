#!/bin/sh

DBHOST="localhost"
DBNAME="gluxi"
DBUSER="gluxi"
DBPASS="gluxi"
DBTYPE="$1"
QUERYCMD=""
INLINE_SUFFIX=""

if [ "$DBTYPE" = "mysql" ]; then
	echo "MySQL support is currently out of date"
	exit 1
fi

if [ -f "dbupdate.cfg" ]; then
	. ./dbupdate.cfg $*
fi

if [ -z "$QUERYCMD" ]; then
	if [ -z "$DBTYPE" ]; then
		echo "Usage: dbupdate.sh [dbtype] <params>"
		exit 1
	fi

	if [ "$DBTYPE" = "mysql" ]; then
		QUERYCMD="mysql -h $DBHOST -u $DBUSER -p${DBPASS} $DBNAME --skip-column-names -A -B"
		INLINE_SUFFIX="-e"
	fi
	
	if [ "$DBTYPE" = "psql" ] || [ "$DBTYPE" = "pgsql" ]; then
		export ON_ERROR_STOP="true"
		DBTYPE="pgsql"
		QUERYCMD="psql -U $DBUSER -h $DBHOST $DBNAME -A -t -q"
		INLINE_SUFFIX="--command"
	fi

	if [ -z "$QUERYCMD" ]; then
		echo "Unknown database type: $DBTYPE"
		exit 1
	fi
fi

CURRENT_VERSION=`$QUERYCMD $INLINE_SUFFIX "SELECT value FROM version WHERE name='dbversion'"`
if [ -z "$CURRENT_VERSION" ]; then
	echo "Unable to get current database version"
	exit 1
fi
echo "Current database version: $CURRENT_VERSION"
ALIGNED_VERSION=`printf "%05d\n" $CURRENT_VERSION`

for FILE in `ls $DBTYPE | sed -e 's/\\.sql$//g' | sort`; do
	if [ $FILE -gt $ALIGNED_VERSION ]; then
		echo "==> Applying database update: $FILE.sql"
		RESULT="`$QUERYCMD < $DBTYPE/$FILE.sql 2>&1 | grep -v 'NOTICE:'`"
		if [ "z$RESULT" != "z" ]; then
			echo "--> Something happens:"
			echo "$RESULT"
			exit 1
		fi
		UPDATED_VERSION="`echo $FILE | sed -e 's/0\\+//'`"
		echo "Writing info: $UPDATED_VERSION"
		RESULT="`$QUERYCMD $INLINE_SUFFIX "UPDATE version SET value='$UPDATED_VERSION' WHERE name='dbversion'"`"
		if [ "z$RESULT" != "z" ]; then
			echo "--> Something happens:"
			echo "$RESULT"
			exit 1
		fi
	fi
done

echo "Database successfuly updated"
