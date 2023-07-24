#!/bin/sh

TESTFILE="client.js operator.js admin.js clifw.js"
NEWER=
BUILD_DIRECTORY="./build"

# Test if one the file listed upper is newer than the public/index.js
for f in $TESTFILE
do
		[ "$f" -nt public/index.js ] && NEWER=1
done

echo "Generate index.js"
if [ ! -d "$BUILD_DIRECTORY" ]; then
	mkdir $BUILD_DIRECTORY
	chmod 0775 $BUILD_DIRECTORY
fi
npx browserify client.js -o public/index.js
[ $? != "0" ] && rm public/index.js && exit 1

if ([ ! -f public/css/main.css ] || [ public/css/main.less -nt public/css/main.css ]); then
	echo "Generate main.css"
	npx lessc public/css/main.less > public/css/main.css
	cp public/css/main.css build/public/css/main.css
	[ $? != "0" ] && public/css/main.css && exit 1
fi

