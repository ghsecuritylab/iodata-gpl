if [ -e /apps/apps_lzma_package.tar.lzma ]; then
    echo "Extracting /apps/apps_lzma_package.tar.lzma"
    cd /
    tar xavf /apps/apps_lzma_package.tar.lzma
fi
ln -s /apps/sbin/* /sbin
ln -s /apps/bin/* /bin
ln -s /apps/lib/* /lib
ln -s /apps/usr/sbin/* /usr/sbin
ln -s /apps/usr/bin/* /usr/bin
ln -s /apps/usr/lib/* /usr/lib
ln -s /apps/usr/local/bin/* /usr/local/bin
mkdir -p lib/modules/2.6.34/kernel
ln -s /apps/lib/modules/2.6.34/kernel/* /lib/modules/2.6.34/kernel
cat /apps/lib/modules/2.6.34/modules.dep >> /lib/modules/2.6.34/modules.dep
if [ -e /apps/lib/modules/2.6.34/modulesApp.dep ]; then
cat /apps/lib/modules/2.6.34/modulesApp.dep >>/lib/modules/2.6.34/modules.dep
fi



