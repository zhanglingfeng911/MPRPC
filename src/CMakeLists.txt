#aux_source_directory(. SRC_LIST)
set(SRC_LIST mprpcapplication.cc mprpcconfig.cc rpcheader.pb.cc rpcprovider.cc mprpcchannel.cc logger.cc zookeeperutil.cc)
add_library(mprpc ${SRC_LIST})#因爲Muduo庫是靜態庫 而此處又要鏈接muduo庫 所以我們把這個也編譯成靜態庫
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)#zookeeper_mt是多线程版本
