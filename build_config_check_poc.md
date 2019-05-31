# How to build the ModSec Config Check prototype

1. Clone ModSecurity and check out this branch:
    ```
    git clone --recursive https://github.com/microsoft/ModSecurity.git
    
    git checkout modsec_config_check_poc
    ```
2. Build ModSecurity as follows:
    ```
    ./autogen.sh
    
    LDFLAGS="-g" CXXFLAGS="-std=c++11" CFLAGS="-fPIC" ./configure
    --enable-standalone-module --disable-mlogc --enable-waf_json_logging
    
    cd standalone
    
    make
    
    cd ..
    
    g++ -std=c++11 modsec_config_check.cpp -Iapache2 -Istandalone -I/usr/include/apache2/ -I/usr/include/apr-1.0/ -I/usr/include/libxml2/ -Iapache2/waf_lock -pthread -L/home/argenet/code/Networking-AppGW/src/datapath/ifxwrap -Wl,-rpath,'3423ORIGIN' standalone/.libs/standalone.a -Wl,-E -ldl -lpthread -lcrypt -L/usr/lib -lm -llua5.1 -L/usr/lib/x86_64-linux-gnu -lapr-1 -L/usr/lib/x86_64-linux-gnu -laprutil-1 -I/usr/include/apache2 -L/usr/lib/x86_64-linux-gnu -lcurl -lxml2 -llua5.1 -lpcre -L/usr/lib -lyajl -lstdc++ -Wl,-Bstatic -lprotobuf -Wl,-Bdynamic -lstdc++ -Wl,-Bstatic -lprotobuf -lgrpc++ -lgrpc -Wl,-Bdynamic -lpcre -lssl -lcrypto -ldl -lz -o modsec_config_check
    ```
3. Check the utility by running it against ModSecurity config:
    ```
    ./modsec_config_check /etc/nginx/modsec/modsecurity_appgateway.conf
    ```
This code is still prototype that should be tidied up.
