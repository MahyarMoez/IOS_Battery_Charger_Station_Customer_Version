#!/bin/sh
VERSION_MAJOR=0
VERSION_MINOR=0
VERSION_PATCH=0
VERSION_BUILD=0

if command -v git >/dev/null 2>&1 && [ -d ".git" ]; then
    DESCRIBE=$(git describe --tags --long --match "v[0-9]*.[0-9]*.[0-9]*" 2>/dev/null)
    if [ -n "$DESCRIBE" ]; then
        # parse v1.2.3-45-gabcde
        VERSION_MAJOR=$(echo $DESCRIBE | sed -E 's/^v?([0-9]+)\..*/\1/')
        VERSION_MINOR=$(echo $DESCRIBE | sed -E 's/^v?[0-9]+\.([0-9]+)\..*/\1/')
        VERSION_PATCH=$(echo $DESCRIBE | sed -E 's/^v?[0-9]+\.[0-9]+\.([0-9]+).*/\1/')
        VERSION_BUILD=$(echo $DESCRIBE | sed -E 's/^v?[0-9]+\.[0-9]+\.[0-9]+-([0-9]+).*/\1/')
    else
        VERSION_BUILD=$(git rev-list --count HEAD)
    fi
fi

echo "#define APP_VERSION \"$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_BUILD\""