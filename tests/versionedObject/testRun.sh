#!/usr/bin/bash

echo "############# testRun.sh ################"

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters: Expected argument :: ${0} <PATH_TO_BINARY_TEST>"
fi


SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECTDIR="$( cd "$( dirname "${SCRIPTDIR}" )" && pwd )"

TESTEXE="${1}"
TESTNAME="$(basename "${1}")"

TODAY="$(date +"%d-%b-%Y")"


TESTLOG="${SCRIPTDIR}/testlog/${TESTNAME}.log"
${TESTEXE}  2>&1 | tee "${TESTLOG}"

#retVal=$?   # as tee got added
retVal=${PIPESTATUS[0]}
if [ $retVal -ne 0 ]; then
    exit $retVal
fi
#sleep 3



sed -i "s|:: file:\([^:]\+\):[0-9]\+$|:: file:\1:line-number|g"  "${TESTLOG}"
sed -i "s|${PROJECTDIR}|/VERSIONOBJECT_DIR|g"  "${TESTLOG}"
#  :: file:include/versionedObject/VersionedObjectBuilder.h:265

git status -s "${TESTLOG}"

# if output has changed from what was expected, we use 'git' to indentify and return the count of those changed files
exit $(git status -s "${TESTLOG}" | wc -l)

