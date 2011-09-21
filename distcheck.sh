for pack in . a4atlas a4io a4process; do
    pushd $pack;
    rm _dcbuild -rf;
    autoreconf -f -i
    mkdir _dcbuild
    pushd _dcbuild
    ../configure
    make distcheck
    popd
    rm _dcbuild -rf
    popd;
done;
echo "distcheck done on all packages"
