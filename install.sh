#!/bin/bash

#
# mupen64plus install script
#

if [ -x /usr/bin/ginstall ]; then
	INSTALL=/usr/bin/ginstall
elif [ -x /bin/ginstall ]; then
	INSTALL=/bin/ginstall
elif [ -x /usr/bin/install ]; then
	INSTALL=/usr/bin/install
elif [ -x /bin/install ]; then
	INSTALL=/bin/install
else
	INSTALL=install
fi

usage()
{
	echo "usage: $(basename $0) [PREFIX] [SHAREDIR] [BINDIR] [LIBDIR] [MANDIR]"
	echo -e "\tPREFIX   - installation directories prefix (default: /usr/local)"
	echo -e "\tSHAREDIR - path to Mupen64Plus shared data files (default: \$PREFIX/share/mupen64plus)"
	echo -e "\tBINDIR   - path to Mupen64Plus binary program files (default: \$PREFIX/bin)"
	echo -e "\tLIBDIR   - path to Mupen64Plus plugins (default: \$SHAREDIR/plugins)"
	echo -e "\tMANDIR   - path to manual files (default: \$PREFIX/man/man1)"
	echo "To install KDE4 version, use: GUI=KDE4 $(basename $0)"
}

if [ $# -gt 5 ]; then
	usage
	exit 1
fi

if [ $# -gt 0 ]; then
	PREFIX=$1
else
	PREFIX=/usr/local
fi

if [ $# -gt 1 ]; then
	SHAREDIR=$2
else
	SHAREDIR=${PREFIX}/share/mupen64plus
fi

if [ $# -gt 2 ]; then
	BINDIR=$3
else
	BINDIR=${PREFIX}/bin
fi

if [ $# -gt 3 ]; then
	LIBDIR=$4
else
	LIBDIR=${SHAREDIR}/plugins
fi

if [ $# -gt 4 ]; then
	MANDIR=$5
else
	MANDIR=${PREFIX}/man/man1
fi

echo "Installing Mupen64Plus to $PREFIX"
$INSTALL -d -v "${SHAREDIR}" || exit $?
$INSTALL -d -v "${SHAREDIR}/config" || exit $?
$INSTALL -m 0644 config/* "${SHAREDIR}/config" || exit $?
$INSTALL -m 0644 mupen64plus.ini "${SHAREDIR}/" || exit $?
$INSTALL -d -v "${SHAREDIR}/doc" || exit $?
$INSTALL -m 0644 doc/HiRezTexture.txt "${SHAREDIR}/doc" || exit $?
$INSTALL -d -v "${SHAREDIR}/fonts" || exit $?
$INSTALL -m 0644 fonts/* "${SHAREDIR}/fonts" || exit $?
$INSTALL -d -v "${SHAREDIR}/icons" || exit $?
$INSTALL -m 0644 icons/*.png "${SHAREDIR}/icons" || exit $?
$INSTALL -m 0644 icons/*.xpm "${SHAREDIR}/icons" || exit $?
$INSTALL -d -v "${SHAREDIR}/icons/32x32" || exit $?
$INSTALL -m 0644 icons/32x32/* "${SHAREDIR}/icons/32x32" || exit $?
$INSTALL -d -v "${SHAREDIR}/icons/22x22" || exit $?
$INSTALL -m 0644 icons/22x22/* "${SHAREDIR}/icons/22x22" || exit $?
$INSTALL -d -v "${SHAREDIR}/icons/16x16" || exit $?
$INSTALL -m 0644 icons/16x16/* "${SHAREDIR}/icons/16x16" || exit $?
$INSTALL -d -v "${SHAREDIR}/lang" || exit $?
$INSTALL -m 0644 lang/* "${SHAREDIR}/lang" || exit $?
if [ "${GUI}" == "KDE4" ]; then
    $INSTALL -m 0644 mupen64plusui.rc "${SHAREDIR}" || exit $?
fi

$INSTALL -D -m 0755 mupen64plus "${BINDIR}/mupen64plus" || exit $?
$INSTALL -d -v "${MANDIR}" || exit $?
$INSTALL -m 0644 doc/mupen64plus.1.gz "${MANDIR}" || exit $?
$INSTALL -d -v "${LIBDIR}" || exit $?
$INSTALL -m 0755 plugins/* "${LIBDIR}" || exit $?

echo "Done."

