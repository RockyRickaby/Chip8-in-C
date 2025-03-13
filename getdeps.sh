#!/bin/sh

includedir=include
libdir=lib
tmp=temporary
raylib=raylib-5.5_linux_amd64
raylibzip=${raylib}.tar.gz

getdependencies()
{
	echo "Downloading dependencies..."
	if [ ! -e $tmp ]; then
		mkdir $tmp
	fi
	if [ ! -e $includedir ]; then
		mkdir $includedir
	fi
	if [ ! -e $libdir ]; then
		mkdir $libdir
	fi
	wget -vc -P $tmp https://github.com/raysan5/raylib/releases/download/5.5/${raylibzip}
	curl -o ${tmp}/rprand.h https://raw.githubusercontent.com/raysan5/raylib/refs/heads/master/src/external/rprand.h
	
	tar -xzvf ${tmp}/${raylibzip} -C ${tmp}

	mv ${tmp}/${raylib}/lib/* $libdir
	mv ${tmp}/${raylib}/include/* $includedir

	mkdir ${includedir}/external
	mv ${tmp}/rprand.h ${includedir}/external

	rm -rf $tmp
	
	echo "Done downloading dependencies"
}

# check if files already exist. If not, just download everything at once.

if [ ! -e ${includedir}/libraylib.a ]; then
	getdependencies 
elif [ ! -e ${includedir}/libraylib.so.5.5.0 ]; then
	getdependencies 
elif [ ! -e ${includedir}/libraylib.so.550 ]; then
	getdependencies 
elif [ ! -e ${libdir}/rprand.h ]; then
	getdependencies 
elif [ ! -e ${libdir}/raylib.h ]; then
	getdependencies 
elif [ ! -e ${libdir}/raymath.h ]; then
	getdependencies 
elif [ ! -e ${libdir}/rlgl.h ]; then
	getdependencies 
fi

echo "Done!"
