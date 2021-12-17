#!/bin/bash

set -e

cd "$(dirname "$0")"

RUNTIME_DIR=".runtime"
RUNTIME_VERSION="16.0.0-preview-1"

if [ "$(uname)" == "Darwin" ]; then
	PACKAGE_OS_NAME="x86_64-macos"
	BINARY_NAME="runtime"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	PACKAGE_OS_NAME="x86_64-linux"
	BINARY_NAME="runtime"
else
	PACKAGE_OS_NAME="x86_64-win32"
	BINARY_NAME="runtime.exe"
fi

mkdir -p "${RUNTIME_DIR}"
pushd "${RUNTIME_DIR}"
	if [[ ! -f "${BINARY_NAME}" ]]; then
		echo "Runtime binary not found, downloading..."
		spatial package get runtime "${PACKAGE_OS_NAME}" "${RUNTIME_VERSION}" "runtime.zip"
		unzip "runtime.zip"
	fi
popd

"${RUNTIME_DIR}/${BINARY_NAME}" --config tiles_standalone.json --snapshot build/seeders/tiles.snapshot --schema-bundle build/schema/schema.sb
