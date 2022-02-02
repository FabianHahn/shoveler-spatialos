#!/bin/bash

set -e

cd "$(dirname "$0")"

INSPECTOR_DIR=".inspector"
INSPECTOR_VERSION="16.0.0-preview-2"

if [ "$(uname)" == "Darwin" ]; then
	PACKAGE_OS_NAME="inspector_x86_64-macos"
	BINARY_NAME="inspector"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	PACKAGE_OS_NAME="inspector_x86_64-linux"
	BINARY_NAME="inspector"
else
	PACKAGE_OS_NAME="inspector_x86_64-win32"
	BINARY_NAME="inspector.exe"
fi

mkdir -p "${INSPECTOR_DIR}"
pushd "${INSPECTOR_DIR}"
	if [[ ! -f "${BINARY_NAME}" ]]; then
		echo "Inspector binary not found, downloading..."
		spatial package get inspector "${PACKAGE_OS_NAME}" "${INSPECTOR_VERSION}" "./${BINARY_NAME}"
		chmod +x "./${BINARY_NAME}"
	fi
popd

"${INSPECTOR_DIR}/${BINARY_NAME}" --grpc_addr="localhost:7777" --http_addr="localhost:5006" --schema_bundle="build/schema/schema.sb"
