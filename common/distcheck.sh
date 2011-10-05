export A4INSTALL=$PWD/_install
export NTHREADS=4
chmod -R +w $A4INSTALL 2> /dev/null;
rm $A4INSTALL -rf || exit 1;
mkdir -p $A4INSTALL
for pack in a4io a4process a4atlas; do
    pushd $pack || exit 1
    chmod -R +w _dcbuild 2> /dev/null;
    rm _dcbuild -rf || exit 1
    ../autogen.sh || exit 1
    mkdir _dcbuild || exit 1
    pushd _dcbuild || exit 1
    ../configure --prefix=$A4INSTALL --with-a4=$A4INSTALL $1 || exit 1
    make -j$NTHREADS distcheck || exit 1
    make -j$NTHREADS install || exit 1
    make -j$NTHREADS installcheck || exit 1
    popd || exit 1
    chmod -R +w _dcbuild 2> /dev/null;
    rm _dcbuild -rf || exit 1
    popd || exit 1
done;
chmod -R +w $A4INSTALL 2> /dev/null;
rm $A4INSTALL -rf || exit 1;
mkdir -p $A4INSTALL
for pack in .; do
    pushd $pack || exit 1
    chmod -R +w _dcbuild 2> /dev/null;
    rm _dcbuild -rf || exit 1
    ./autogen.sh || exit 1
    mkdir _dcbuild || exit 1
    pushd _dcbuild || exit 1
    ../configure --prefix=$A4INSTALL $1 || exit 1
    make -j$NTHREADS distcheck || exit 1
    make -j$NTHREADS install || exit 1
    make -j$NTHREADS installcheck || exit 1
    popd || exit 1
    chmod -R +w _dcbuild 2> /dev/null;
    rm _dcbuild -rf || exit 1
    popd || exit 1
done;
echo "=============================="
echo "Tests successful on all packs!"
echo "=============================="
